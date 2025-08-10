#include <stdio.h>
#include <stdlib.h> 
#include <stdint.h>
#include <assert.h>
#include "lib.h" 
#include <iostream>

constexpr uint16_t mem_size = 1024 * 32;


struct Mem {
    uint16_t memory[mem_size] = {0};
};

struct CPU {
    uint16_t registrador[8] = {0};
    uint16_t PC = 1;
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



void executarInstrucao(uint16_t instrucao, CPU *cpu, Mem *mem) {

    uint16_t tipo = extract_bits(instrucao, 15, 1);
    bool pc_modificado = false; //pra controlar caso seja um jump, pq o jump modifica o pc p/ valor imediato
    switch (tipo) {
        case 0: { // Tipo R
            uint16_t opcode  = extract_bits(instrucao, 9, 6);
            uint16_t destino = extract_bits(instrucao, 6, 3);
            uint16_t op1     = extract_bits(instrucao, 3, 3);
            uint16_t op2     = extract_bits(instrucao, 0, 3);

            switch (opcode) {
                case 0b000000:  // ADD
                    cpu->registrador[destino] = cpu->registrador[op1] + cpu->registrador[op2];
                    break;
                case 0b000001:  // SUB
                    cpu->registrador[destino] = cpu->registrador[op1] - cpu->registrador[op2];
                    break;
                case 0b000010:  // MUL
                    cpu->registrador[destino] = cpu->registrador[op1] * cpu->registrador[op2];
                    break;
                case 0b000011:  // DIV
                    if (cpu->registrador[op2] != 0) {
                        cpu->registrador[destino] = cpu->registrador[op1] / cpu->registrador[op2];
                    }
                    break;
                case 0b000100:  // CMP_EQ
                    cpu->registrador[destino] = (cpu->registrador[op1] == cpu->registrador[op2]);
                    break;
                case 0b000101:  // CMP_NEQ
                    cpu->registrador[destino] = (cpu->registrador[op1] != cpu->registrador[op2]);
                    break;
                case 0b001111: // LOAD
                    cpu->registrador[destino] = mem->memory[cpu->registrador[op1]];
                    break;
                case 0b010000: // STORE
                    mem->memory[cpu->registrador[op1]] = cpu->registrador[op2];
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
            uint16_t opcode   = extract_bits(instrucao, 13, 2);
            uint16_t reg      = extract_bits(instrucao, 10, 3);
            uint16_t imediato = extract_bits(instrucao, 0, 10);

            switch (opcode) {
                case 0b00: // JUMP
                    cpu->PC = imediato;
                pc_modificado = true;//como o jump altera o pc pra valor imediato, iso aqui avisa lá em baixo que não é pra incrementar (bqaseado na condicao!!)
                    break;
                case 0b01: // JUMP_COND
                    if (cpu->registrador[reg] != 0) {
                        cpu->PC = imediato;
                        pc_modificado = true;
                    }
                    break;
                case 0b11: // MOV
                    cpu->registrador[reg] = imediato;
                    break;
                default:
                    printf("Opcode Tipo I desconhecido: %u\n", opcode);
                    break;
            }
            break;
        }
    }
    //tudo q for rodar de instrução vai aumentar o pc, mas se for jump, o pc recebe um valor imediato
    //entao eh bom diferenciar (se vc tiver alguma idea melhor me avisa)
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

    load_binary_to_memory(argv[1], mem.memory, mem_size);

    while(1) {
        //fetch
        uint16_t instrucao_atual = mem.memory[cpu.PC];
        //decode, execute
        executarInstrucao(instrucao_atual, &cpu, &mem);
    }

    return 0;
}
