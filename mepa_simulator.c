/*
Andrey Bezerra Virginio dos Santos - 10420696
Igor Silva Araujo - 10428505
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_CODE 10000
#define MAX_STACK 10000
#define MAX_LABELS 1000

typedef struct {
    char label[32];
    int address;
} Label;

typedef struct {
    char op[16];
    char arg_str[32];   // argumento como string (para rótulos)
    int arg_int;        // argumento como inteiro (para números)
    int is_label_arg;   // 1 se o argumento é um rótulo (não numérico)
    int address;
} Instruction;

Instruction code[MAX_CODE];
int code_count = 0;
Label labels[MAX_LABELS];
int label_count = 0;

int stack[MAX_STACK];
int sp = 0;
int fp = 0;
int pc = 0;

void add_label(const char *label, int addr) {
    for (int i = 0; i < label_count; i++) {
        if (strcmp(labels[i].label, label) == 0) return;
    }
    strcpy(labels[label_count].label, label);
    labels[label_count].address = addr;
    label_count++;
}

int get_label_addr(const char *label) {
    for (int i = 0; i < label_count; i++) {
        if (strcmp(labels[i].label, label) == 0) return labels[i].address;
    }
    fprintf(stderr, "Rotulo desconhecido: %s\n", label);
    exit(1);
}

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

        // Remove espaços iniciais
        char *p = line;
        while (isspace(*p)) p++;

        // Verifica se é um rótulo (termina com ':')
        char *colon = strchr(p, ':');
        if (colon && colon[1] == '\0') {
            *colon = '\0';
            add_label(p, addr);
            continue;
        }

        // Divide a linha em op e argumento
        char op[32], arg[32];
        int n = sscanf(p, "%s %s", op, arg);
        strcpy(code[addr].op, op);
        if (n == 2) {
            // Tenta interpretar argumento como número
            char *endptr;
            long num = strtol(arg, &endptr, 10);
            if (*endptr == '\0') {
                // É número
                code[addr].arg_int = (int)num;
                code[addr].is_label_arg = 0;
                strcpy(code[addr].arg_str, arg);
            } else {
                // É rótulo (string)
                code[addr].is_label_arg = 1;
                strcpy(code[addr].arg_str, arg);
                code[addr].arg_int = 0;
            }
        } else {
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

void execute_instruction(void) {
    Instruction *ins = &code[pc];
    
    if (strcmp(ins->op, "AMEM") == 0) {
        sp += ins->arg_int;
    }
    else if (strcmp(ins->op, "CHPD") == 0) {
        int val = stack[--sp];
        printf("%d", val);
        fflush(stdout);
    }
    else if (strcmp(ins->op, "DMEM") == 0) {
        sp -= ins->arg_int;
    }
    else if (strcmp(ins->op, "CRCT") == 0) {
        stack[sp++] = ins->arg_int;
    }
    else if (strcmp(ins->op, "CRVL") == 0) {
        stack[sp++] = stack[fp + ins->arg_int];
    }
    else if (strcmp(ins->op, "ARMZ") == 0) {
        int valor = stack[--sp];
        stack[fp + ins->arg_int] = valor;
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
        stack[sp++] = (a == 0) ? 1 : 0;
    }
    else if (strcmp(ins->op, "CONJ") == 0) {
        int b = stack[--sp];
        int a = stack[--sp];
        stack[sp++] = (a && b) ? 1 : 0;
    }
    else if (strcmp(ins->op, "DISJ") == 0) {
        int b = stack[--sp];
        int a = stack[--sp];
        stack[sp++] = (a || b) ? 1 : 0;
    }
    else if (strcmp(ins->op, "CMME") == 0) {
        int b = stack[--sp];
        int a = stack[--sp];
        stack[sp++] = (a < b) ? 1 : 0;
    }
    else if (strcmp(ins->op, "CMMA") == 0) {
        int b = stack[--sp];
        int a = stack[--sp];
        stack[sp++] = (a > b) ? 1 : 0;
    }
    else if (strcmp(ins->op, "CMEQ") == 0) {
        int b = stack[--sp];
        int a = stack[--sp];
        stack[sp++] = (a == b) ? 1 : 0;
    }
    else if (strcmp(ins->op, "CDIF") == 0) {
        int b = stack[--sp];
        int a = stack[--sp];
        stack[sp++] = (a != b) ? 1 : 0;
    }
    else if (strcmp(ins->op, "CMMEQ") == 0) {
        int b = stack[--sp];
        int a = stack[--sp];
        stack[sp++] = (a <= b) ? 1 : 0;
    }
    else if (strcmp(ins->op, "CMMAQ") == 0) {
        int b = stack[--sp];
        int a = stack[--sp];
        stack[sp++] = (a >= b) ? 1 : 0;
    }
    else if (strcmp(ins->op, "DSVS") == 0) {
        pc = get_label_addr(ins->arg_str);
        return;
    }
    else if (strcmp(ins->op, "DSVF") == 0) {
        int cond = stack[--sp];
        if (cond == 0) {
            pc = get_label_addr(ins->arg_str);
            return;
        }
    }
    else if (strcmp(ins->op, "CHPR") == 0) {
        int val = stack[--sp];
        if (val >= 32 && val <= 126)
            printf("%c", val);
        else
            printf("%d", val);
        fflush(stdout);
    }
    else if (strcmp(ins->op, "CHLE") == 0) {
        int val;
        scanf("%d", &val);
        stack[sp++] = val;
    }
    else if (strcmp(ins->op, "PARA") == 0) {
        // nada
    }
    else if (strcmp(ins->op, "CHAM") == 0) {
        int target = get_label_addr(ins->arg_str);
        stack[sp++] = fp;
        stack[sp++] = pc + 1;
        fp = sp;
        pc = target;
        return;
    }
    else if (strcmp(ins->op, "RETOR") == 0) {
        if (sp == 0) {
            pc = code_count;
            return;
        }
        sp = fp;
        pc = stack[--sp];
        fp = stack[--sp];
        return;
    }
    else if (strcmp(ins->op, "CRVI") == 0) {
        int addr = stack[--sp];
        stack[sp++] = stack[addr];
    }
    else if (strcmp(ins->op, "ARMI") == 0) {
        int addr = stack[--sp];
        int valor = stack[--sp];
        stack[addr] = valor;
    }
    else if (strcmp(ins->op, "TROCA") == 0) {
        int a = stack[sp-2];
        int b = stack[sp-1];
        stack[sp-2] = b;
        stack[sp-1] = a;
    }
    else {
        fprintf(stderr, "Instrucao desconhecida: %s\n", ins->op);
        exit(1);
    }
    pc++;
}

void run_program(void) {
    pc = 0;
    sp = 0;
    fp = 0;
    memset(stack, 0, sizeof(stack));
    while (pc < code_count && pc >= 0) {
        execute_instruction();
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo.mepa>\n", argv[0]);
        return 1;
    }
    load_program(argv[1]);
    run_program();
    return 0;
}
