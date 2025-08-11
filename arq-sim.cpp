#include <stdio.h>
#include <stdlib.h> 
#include <stdint.h>
#include <assert.h>
#include "lib.h" 
#include <iostream>

constexpr uint16_t MEM_SIZE = 1024 * 32;
constexpr uint16_t REG_SIZE = 8;

struct Mem {
    uint16_t memory[MEM_SIZE];
};

struct CPU {
    uint16_t registrador[REG_SIZE];
    uint16_t PC = 1;
};

struct Instr {
    uint16_t tipo = 0;
    uint16_t opcode = 0;
    uint16_t destino = 0;
    uint16_t op1 = 0;
    uint16_t op2 = 0;
    uint16_t reg = 0;
    uint16_t imediato = 0;
};

void executarSyscall(CPU *cpu, Mem *mem) {
    switch (cpu->registrador[0]) {
        case 0: 
            printf("\nPrograma finalizado\n");
            for(int i = 0; i < 8; i++) {
                printf("R%d: %u\n", i, cpu->registrador[i]);
            }
            exit(0);
        case 1: { 
            int i = cpu->registrador[1];
            while(mem->memory[i] != 0){
                printf("%c", (char)mem->memory[i]);
                i++;
            }
            break;
        }
        case 2:
            printf("\n");
            break;
        case 3:
            printf("%d",cpu->registrador[1]);
            break;
        default:
            exit(1);
    }
}

void decodificarInstrucao(uint16_t instrucao, Instr *instr){
    instr->tipo = extract_bits(instrucao, 15, 1);
    
    if(instr->tipo == 0){
        instr->opcode  = extract_bits(instrucao, 9, 6);
        instr->destino = extract_bits(instrucao, 6, 3);
        instr->op1     = extract_bits(instrucao, 3, 3);
        instr->op2     = extract_bits(instrucao, 0, 3);
    } else {
        instr->opcode   = extract_bits(instrucao, 13, 2);
        instr->reg      = extract_bits(instrucao, 10, 3);
        instr->imediato = extract_bits(instrucao, 0, 10);
    }

}

void executarInstrucao(CPU *cpu, Mem *mem, Instr *instr) {
    bool pc_modificado = false; //pra controlar caso seja um jump, pq o jump modifica o pc p/ valor imediato
    
    switch (instr->tipo) {
        case 0: { // Tipo R

            switch (instr->opcode) {
                case 0b000000:  // ADD
                    cpu->registrador[instr->destino] = cpu->registrador[instr->op1] + cpu->registrador[instr->op2];
                    break;
                case 0b000001:  // SUB
                    cpu->registrador[instr->destino] = cpu->registrador[instr->op1] - cpu->registrador[instr->op2];
                    break;
                case 0b000010:  // MUL
                    cpu->registrador[instr->destino] = cpu->registrador[instr->op1] * cpu->registrador[instr->op2];
                    break;
                case 0b000011:  // DIV
                    if (cpu->registrador[instr->op2] != 0) {
                        cpu->registrador[instr->destino] = cpu->registrador[instr->op1] / cpu->registrador[instr->op2];
                    }
                    break;
                case 0b000100:  // CMP_EQ
                    cpu->registrador[instr->destino] = (cpu->registrador[instr->op1] == cpu->registrador[instr->op2]);
                    break;
                case 0b000101:  // CMP_NEQ
                    cpu->registrador[instr->destino] = (cpu->registrador[instr->op1] != cpu->registrador[instr->op2]);
                    break;
                case 0b001111: // LOAD
                    cpu->registrador[instr->destino] = mem->memory[cpu->registrador[instr->op1]];
                    break;
                case 0b010000: // STORE
                    mem->memory[cpu->registrador[instr->op1]] = cpu->registrador[instr->op2];
                    break;
                case 0b111111: // SYSCALL
                    executarSyscall(cpu, mem);
                    break;
                default:
                    break;
            }
            break;
        }
        case 1: { // Tipo I

            switch (instr->opcode) {
                case 0b00: // JUMP
                    cpu->PC = instr->imediato;
                    pc_modificado = true;//como o jump altera o pc pra valor imediato, iso aqui avisa lá em baixo que não é pra incrementar (baseado na condição!)
                    break;
                case 0b01: // JUMP_COND
                    if (cpu->registrador[instr->reg] != 0) {
                        cpu->PC = instr->imediato;
                        pc_modificado = true;
                    }
                    break;
                case 0b11: // MOV
                    cpu->registrador[instr->reg] = instr->imediato;
                    break;
                default:
                    printf("Opcode Tipo I desconhecido: %u\n", instr->opcode);
                    break;
            }
            break;
        }
    }
    //tudo q for rodar de instrução vai aumentar o pc, mas se for jump, o pc recebe um valor imediato
    //entao eh bom diferenciar
    if (!pc_modificado) {
        cpu->PC++;
    }
}

int main (int argc, char **argv) {
    if (argc != 2) {
        printf("usage: %s [bin_name]\n", argv[0]);
        exit(1);
    }

    Mem mem;
    CPU cpu;
    Instr instr;

    load_binary_to_memory(argv[1], mem.memory, MEM_SIZE);

    while(1) {
        //fetch
        uint16_t instrucao_atual = mem.memory[cpu.PC];
        //decode
        decodificarInstrucao(instrucao_atual, &instr);
        //execute
        executarInstrucao(&cpu, &mem, &instr);
    }

    return 0;
}
