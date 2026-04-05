/*
Andrey Bezerra Virginio dos Santos - 10420696
Igor Silva Araujo - 10428505
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Limites máximos para código, pilha e rótulos */
#define MAX_CODE 10000
#define MAX_STACK 10000
#define MAX_LABELS 1000

/* Estrutura para armazenar um rótulo e seu endereço */
typedef struct {
    char label[32];
    int address;
} Label;

/* Estrutura para armazenar uma instrução do código MEPA */
typedef struct {
    char op[16];
    char arg_str[32];   // argumento como string (para rótulos)
    int arg_int;        // argumento como inteiro (para números)
    int is_label_arg;   // 1 se o argumento é um rótulo (não numérico)
    int address;
} Instruction;

/* Memória do programa e estruturas auxiliares */
Instruction code[MAX_CODE];
int code_count = 0;
Label labels[MAX_LABELS];
int label_count = 0;

/* Pilha de execução, ponteiros e contador de programa */
int stack[MAX_STACK];
int sp = 0;   // stack pointer (topo)
int fp = 0;   // frame pointer (base do quadro atual)
int pc = 0;   // program counter

/* Adiciona um rótulo à tabela, evitando duplicatas */
void add_label(const char *label, int addr) {
    for (int i = 0; i < label_count; i++) {
        if (strcmp(labels[i].label, label) == 0) return;
    }
    strcpy(labels[label_count].label, label);
    labels[label_count].address = addr;
    label_count++;
}

/* Retorna o endereço associado a um rótulo */
int get_label_addr(const char *label) {
    for (int i = 0; i < label_count; i++) {
        if (strcmp(labels[i].label, label) == 0) return labels[i].address;
    }
    fprintf(stderr, "Rotulo desconhecido: %s\n", label);
    exit(1);
}

/* Carrega o arquivo .mepa para a memória, interpretando rótulos e instruções */
void load_program(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Erro ao abrir arquivo %s\n", filename);
        exit(1);
    }

    char line[256];
    int addr = 0;
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) == 0) continue;

        char *p = line;
        while (isspace(*p)) p++;

        /* Verifica se a linha define um rótulo (termina com ':') */
        char *colon = strchr(p, ':');
        if (colon && colon[1] == '\0') {
            *colon = '\0';
            add_label(p, addr);
            continue;
        }

        /* Divide a linha em opcode e argumento (se houver) */
        char op[32], arg[32];
        int n = sscanf(p, "%s %s", op, arg);
        strcpy(code[addr].op, op);
        if (n == 2) {
            char *endptr;
            long num = strtol(arg, &endptr, 10);
            if (*endptr == '\0') {
                /* Argumento numérico */
                code[addr].arg_int = (int)num;
                code[addr].is_label_arg = 0;
                strcpy(code[addr].arg_str, arg);
            } else {
                /* Argumento é um rótulo (string) */
                code[addr].is_label_arg = 1;
                strcpy(code[addr].arg_str, arg);
                code[addr].arg_int = 0;
            }
        } else {
            /* Instrução sem argumento */
            code[addr].is_label_arg = 0;
            code[addr].arg_int = 0;
            code[addr].arg_str[0] = '\0';
        }
        code[addr].address = addr;
        addr++;
    }
    code_count = addr;
    fclose(f);
}

/* Executa a instrução apontada pelo PC e avança (ou desvia) */
void execute_instruction(void) {
    Instruction *ins = &code[pc];
    
    if (strcmp(ins->op, "AMEM") == 0) {
        sp += ins->arg_int;                     // aloca n células na pilha
    }
    else if (strcmp(ins->op, "DMEM") == 0) {
        sp -= ins->arg_int;                     // desaloca n células
    }
    else if (strcmp(ins->op, "CRCT") == 0) {
        stack[sp++] = ins->arg_int;             // empilha constante
    }
    else if (strcmp(ins->op, "CRVL") == 0) {
        stack[sp++] = stack[fp + ins->arg_int]; // empilha valor de variável (offset relativo a fp)
    }
    else if (strcmp(ins->op, "ARMZ") == 0) {
        int valor = stack[--sp];
        stack[fp + ins->arg_int] = valor;       // armazena topo na variável
    }
    else if (strcmp(ins->op, "SOMA") == 0) {
        int b = stack[--sp];
        int a = stack[--sp];
        stack[sp++] = a + b;
    }
    else if (strcmp(ins->op, "SUBT") == 0) {
        int b = stack[--sp];
        int a = stack[--sp];
        stack[sp++] = a - b;
    }
    else if (strcmp(ins->op, "MULT") == 0) {
        int b = stack[--sp];
        int a = stack[--sp];
        stack[sp++] = a * b;
    }
    else if (strcmp(ins->op, "DIVI") == 0) {
        int b = stack[--sp];
        int a = stack[--sp];
        if (b == 0) { fprintf(stderr, "Erro: divisao por zero\n"); exit(1); }
        stack[sp++] = a / b;
    }
    else if (strcmp(ins->op, "INVR") == 0) {
        int a = stack[--sp];
        stack[sp++] = (a == 0) ? 1 : 0;        // negação lógica (~)
    }
    else if (strcmp(ins->op, "CONJ") == 0) {
        int b = stack[--sp];
        int a = stack[--sp];
        stack[sp++] = (a && b) ? 1 : 0;        // conjunção (^)
    }
    else if (strcmp(ins->op, "DISJ") == 0) {
        int b = stack[--sp];
        int a = stack[--sp];
        stack[sp++] = (a || b) ? 1 : 0;        // disjunção (v)
    }
    else if (strcmp(ins->op, "CMME") == 0) {
        int b = stack[--sp];
        int a = stack[--sp];
        stack[sp++] = (a < b) ? 1 : 0;         // menor
    }
    else if (strcmp(ins->op, "CMMA") == 0) {
        int b = stack[--sp];
        int a = stack[--sp];
        stack[sp++] = (a > b) ? 1 : 0;         // maior
    }
    else if (strcmp(ins->op, "CMEQ") == 0) {
        int b = stack[--sp];
        int a = stack[--sp];
        stack[sp++] = (a == b) ? 1 : 0;        // igual
    }
    else if (strcmp(ins->op, "CDIF") == 0) {
        int b = stack[--sp];
        int a = stack[--sp];
        stack[sp++] = (a != b) ? 1 : 0;        // diferente
    }
    else if (strcmp(ins->op, "CMMEQ") == 0) {
        int b = stack[--sp];
        int a = stack[--sp];
        stack[sp++] = (a <= b) ? 1 : 0;        // menor ou igual
    }
    else if (strcmp(ins->op, "CMMAQ") == 0) {
        int b = stack[--sp];
        int a = stack[--sp];
        stack[sp++] = (a >= b) ? 1 : 0;        // maior ou igual
    }
    else if (strcmp(ins->op, "DSVS") == 0) {
        pc = get_label_addr(ins->arg_str);     // desvio incondicional
        return;
    }
    else if (strcmp(ins->op, "DSVF") == 0) {
        int cond = stack[--sp];
        if (cond == 0) {                       // desvia se falso (condição 0)
            pc = get_label_addr(ins->arg_str);
            return;
        }
    }
    else if (strcmp(ins->op, "CHPR") == 0) {
        int val = stack[--sp];
        /* Se o valor for um caractere imprimível, exibe como caractere; senão como número */
        if (val >= 32 && val <= 126)
            printf("%c", val);
        else
            printf("%d", val);
        fflush(stdout);
    }
    else if (strcmp(ins->op, "CHPD") == 0) {
        int val = stack[--sp];
        printf("%d", val);                    // imprime número decimal
        fflush(stdout);
    }
    else if (strcmp(ins->op, "CHLE") == 0) {
        int val;
        scanf("%d", &val);                    // lê um inteiro e empilha
        stack[sp++] = val;
    }
    else if (strcmp(ins->op, "PARA") == 0) {
        /* Indica passagem de parâmetros – não requer ação na pilha */
    }
    else if (strcmp(ins->op, "CHAM") == 0) {
        int target = get_label_addr(ins->arg_str);
        stack[sp++] = fp;                    // salva frame pointer atual
        stack[sp++] = pc + 1;                // salva endereço de retorno
        fp = sp;                             // novo frame pointer
        pc = target;                         // salta para a sub-rotina
        return;
    }
    else if (strcmp(ins->op, "RETOR") == 0) {
        if (sp == 0) {
            pc = code_count;                 // fim do programa
            return;
        }
        sp = fp;                             // restaura stack pointer
        pc = stack[--sp];                    // recupera endereço de retorno
        fp = stack[--sp];                    // restaura frame pointer anterior
        return;
    }
    else if (strcmp(ins->op, "CRVI") == 0) {
        int addr = stack[--sp];
        stack[sp++] = stack[addr];           // carrega valor do endereço no topo
    }
    else if (strcmp(ins->op, "ARMI") == 0) {
        int addr = stack[--sp];
        int valor = stack[--sp];
        stack[addr] = valor;                 // armazena valor no endereço no topo
    }
    else if (strcmp(ins->op, "TROCA") == 0) {
        int a = stack[sp-2];
        int b = stack[sp-1];
        stack[sp-2] = b;                     // troca os dois valores do topo
        stack[sp-1] = a;
    }
    else {
        fprintf(stderr, "Instrucao desconhecida: %s\n", ins->op);
        exit(1);
    }
    pc++;
}

/* Executa o programa carregado, iniciando o PC e a pilha */
void run_program(void) {
    pc = 0;
    sp = 0;
    fp = 0;
    memset(stack, 0, sizeof(stack));
    while (pc < code_count && pc >= 0) {
        execute_instruction();
    }
}

/* Ponto de entrada: carrega o arquivo .mepa passado como argumento e executa */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo.mepa>\n", argv[0]);
        return 1;
    }
    load_program(argv[1]);
    run_program();
    return 0;
}
