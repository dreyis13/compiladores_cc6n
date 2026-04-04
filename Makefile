CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I./src
TARGET = salc
SRCDIR = src
OBJDIR = obj
BINDIR = .

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))

$(BINDIR)/$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(BINDIR)/$(TARGET)

.PHONY: clean

mepa_simulator.c
/*
Andrey Bezerra Virginio dos Santos - 10420696
Igor Silva Araujo - 10428505
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_CODE 10000      // tamanho máximo do código
#define MAX_STACK 10000     // tamanho máximo da pilha
#define MAX_LABELS 1000     // máximo de rótulos

typedef struct {
    char label[32];
    int address;
} Label;

typedef struct {
    char op[16];
    int arg;
    int address;   // endereço da instrução (para referência)
} Instruction;

Instruction code[MAX_CODE];
int code_count = 0;
Label labels[MAX_LABELS];
int label_count = 0;

int stack[MAX_STACK];
int sp = 0;           // stack pointer (topo)
int fp = 0;           // frame pointer
int pc = 0;           // program counter

// Função para adicionar um rótulo
void add_label(const char *label, int addr) {
    for (int i = 0; i < label_count; i++) {
        if (strcmp(labels[i].label, label) == 0) return;
    }
    strcpy(labels[label_count].label, label);
    labels[label_count].address = addr;
    label_count++;
}

// Busca endereço de um rótulo
int get_label_addr(const char *label) {
    for (int i = 0; i < label_count; i++) {
        if (strcmp(labels[i].label, label) == 0) return labels[i].address;
    }
    fprintf(stderr, "Erro: rótulo não encontrado: %s\n", label);
    exit(1);
}

// Carrega o arquivo .mepa para a memória
void load_program(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Erro ao abrir arquivo %s\n", filename);
        exit(1);
    }

    char line[256];
    int addr = 0;
    while (fgets(line, sizeof(line), f)) {
        // Remove newline
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) == 0) continue;

        // Verifica se é um rótulo (termina com ':')
        if (line[strlen(line)-1] == ':') {
            line[strlen(line)-1] = '\0';
            add_label(line, addr);
            continue;
        }

        // Parse instrução: op arg (opcional)
        char op[16];
        int arg = 0;
        if (sscanf(line, "%s %d", op, &arg) == 2) {
            strcpy(code[addr].op, op);
            code[addr].arg = arg;
        } else if (sscanf(line, "%s", op) == 1) {
            strcpy(code[addr].op, op);
            code[addr].arg = 0;
        } else {
            fprintf(stderr, "Linha inválida: %s\n", line);
            exit(1);
        }
        code[addr].address = addr;
        addr++;
    }
    code_count = addr;
    fclose(f);
}

// Executa a instrução atual e avança o PC
void execute_instruction(void) {
    Instruction *ins = &code[pc];
    // printf("DEBUG: PC=%d OP=%s ARG=%d\n", pc, ins->op, ins->arg); // descomente para trace

    if (strcmp(ins->op, "AMEM") == 0) {
        // Aloca memória na pilha (apenas ajusta SP)
        sp += ins->arg;
    }
    else if (strcmp(ins->op, "DMEM") == 0) {
        // Desaloca memória da pilha
        sp -= ins->arg;
    }
    else if (strcmp(ins->op, "CRCT") == 0) {
        stack[sp++] = ins->arg;
    }
    else if (strcmp(ins->op, "CRVL") == 0) {
        // Carrega variável local/global (endereço relativo ao FP)
        // No modelo MEPA, variáveis locais têm offset negativo? Vamos simplificar:
        // CRVL endereço -> empilha stack[fp + endereço]
        stack[sp++] = stack[fp + ins->arg];
    }
    else if (strcmp(ins->op, "ARMZ") == 0) {
        // Armazena topo da pilha em variável
        int valor = stack[--sp];
        stack[fp + ins->arg] = valor;
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
        if (b == 0) {
            fprintf(stderr, "Erro: divisão por zero\n");
            exit(1);
        }
        stack[sp++] = a / b;
    }
    else if (strcmp(ins->op, "INVR") == 0) {
        int a = stack[--sp];
        stack[sp++] = !a;
    }
    else if (strcmp(ins->op, "CONJ") == 0) {
        int b = stack[--sp];
        int a = stack[--sp];
        stack[sp++] = a && b;
    }
    else if (strcmp(ins->op, "DISJ") == 0) {
        int b = stack[--sp];
        int a = stack[--sp];
        stack[sp++] = a || b;
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
        pc = get_label_addr(ins->op);
        return;
    }
    else if (strcmp(ins->op, "DSVF") == 0) {
        int cond = stack[--sp];
        if (!cond) {
            pc = get_label_addr(ins->op);
            return;
        }
    }
    else if (strcmp(ins->op, "CHPR") == 0) {
        // Imprime o valor no topo da pilha (sem consumir? Vamos consumir)
        int val = stack[--sp];
        printf("%d", val);
        fflush(stdout);
    }
    else if (strcmp(ins->op, "CHLE") == 0) {
        // Lê um inteiro do teclado e empilha
        int val;
        scanf("%d", &val);
        stack[sp++] = val;
    }
    else if (strcmp(ins->op, "PARA") == 0) {
        // Passagem de parâmetros: apenas indica quantos foram empilhados
        // Na MEPA real, isso ajustaria algo. Aqui ignoramos.
        // Os parâmetros já estão na pilha.
    }
    else if (strcmp(ins->op, "CHAM") == 0) {
        // Chama sub-rotina: salva PC e FP, ajusta novo FP
        int addr = get_label_addr(ins->op);
        // Salva frame atual na pilha (para retorno)
        stack[sp++] = fp;
        stack[sp++] = pc + 1;  // endereço de retorno
        // Novo frame pointer aponta para o topo atual (antes dos parâmetros)
        // Mas precisamos ajustar: parâmetros já estão na pilha, então FP = SP - (n_param)
        // Vamos usar uma simplificação: FP = SP - (número de parâmetros)
        // O número de parâmetros foi passado por PARA antes.
        // Vamos guardar o número de parâmetros em um lugar? Por simplicidade,
        // assumimos que o código gerado coloca os parâmetros na pilha e o CHAM
        // deve calcular FP = SP - param_count. Como não temos esse número,
        // vamos usar FP = SP (ou seja, parâmetros estão acessíveis via offset positivo)
        fp = sp;
        pc = addr;
        return;
    }
    else if (strcmp(ins->op, "RETOR") == 0) {
        // Retorna de sub-rotina: restaura FP e PC
        // Assume que o valor de retorno (se houver) está no topo da pilha
        int ret_val = 0;
        if (sp > 0) ret_val = stack[sp-1]; // valor de retorno, se existir
        // Restaura frame anterior
        sp = fp;
        pc = stack[--sp];
        fp = stack[--sp];
        // Coloca valor de retorno (se houver) de volta na pilha
        if (ret_val != 0 || (sp > 0 && stack[sp-1] == ret_val)) {
            // Não fazemos nada, pois o valor já está na pilha? Complexo.
            // Vamos simplesmente empilhar o valor de retorno se ele não estiver lá.
            // Melhor: na chamada, o valor retornado deve ficar no topo.
            // Implementaremos: após retorno, o valor retornado (último valor na pilha) permanece.
        }
    }
    else {
        fprintf(stderr, "Instrução desconhecida: %s\n", ins->op);
        exit(1);
    }
    pc++;
}

// Executa todo o programa
void run_program(void) {
    pc = 0;
    sp = 0;
    fp = 0;
    // Inicializa a pilha com zeros (opcional)
    memset(stack, 0, sizeof(stack));

    while (pc < code_count) {
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

teste.sal
module Teste;

proc main() locals x: int; start
    print("Digite um numero:");
    scan(x);
    print("Voce digitou:", x);
end
