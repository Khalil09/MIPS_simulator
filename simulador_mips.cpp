#include <bits/stdc++.h>

using namespace std;

uint32_t opcode,rs,rt,rd,shamt,funct,RI, K16, K26; 


/*
www.opencores.com

Banco de Registradores:
    int breg[32]
    add $t0, $t1, $t2                               |-----------|
    breg[t0] = breg[t1] + breg[t2]              PC->|    add    |
                            RI = mem[PC]            |           |
void fetch(); ->                                    |           |
voide decode();                                     |           |
voide execute();                                    |           |
                                                    |           |
                                                    |-----------|
RI: Registrador de Instrucoes > variavel global
PC: Program Counter           > Variavel global

Formatos de Intrucoes:
                6    5  5   5   5    6
    tipo R-> |opcode|rs|rt|rd|shamt|funct|
    tipo I-> |opcode|rs|rt|      k16     |
    tipo J-> |opcode|         k26        |
    
    32767
    
    0 1 1
    1 0 1
    1 1 0
    0 0 0
    
*/

int two_complements(uint32_t numb, int bits){
    uint32_t mask_16 = 0x8000;
    uint32_t mask_26 = 0x2000000;
    uint32_t result = numb;
    uint32_t aux;
    
    if(bits == 16){
        if(numb & mask_16){
            aux = numb >> 1;
            aux -= 32768;
            result = aux;
            printf("%d\n", result);
        }
    } else {
        if(numb & mask_26){
            aux = numb >> 1;
            aux -= -33554432;
            result = aux;
        }
    }
    if(numb != 65535) return result*2;
    else return result;
}

void to_binario(uint32_t numb){
    while(numb) {
        if (numb & 1) {
            printf("1");
        } else {
            printf("0");
        }
        numb >>= 1;
    }
}

void print_intruc(){
    printf("==================\n");
    printf("|RI   |0x%08x|\n", RI);
    printf("==================\n");
    printf("|opcode |%08x|\n", opcode);
    printf("|rs     |%08x|\n", rs);
    printf("|rt     |%08x|\n", rt);
    printf("|rd     |%08x|\n", rd);
    printf("|shamt  |%08x|\n", shamt);
    printf("|funct  |%08x|\n", funct);
    printf("|K16    |%08x|\n", K16);
    printf("|K26    |%08x|\n", K26);
    printf("==================\n");
    
}

void decode(){
    
    uint32_t mask_opcode_funct = 0x3F;
    uint32_t mask_reg_shamt = 0x1F;
    uint32_t mask_imediate_16 = 0xFFFF;
    uint32_t mask_imediate_26 = 0x3FFFFFF;
    uint32_t aux;
    
    opcode = (RI >> 26) & mask_opcode_funct; /* Filtra opcode */    
    rs = (RI >> 21) & mask_reg_shamt;        /* Filtra rs */
    rt = (RI >> 16) & mask_reg_shamt;        /* Filtra rt */
    rd = (RI >> 11) & mask_reg_shamt;        /* Filtra rd */
    shamt = (RI >> 6) & mask_reg_shamt;      /* Filtra shamt */
    funct = RI & mask_opcode_funct;          /* Filtra funct */
    
    aux = RI & mask_imediate_16;
    K16 = two_complements(aux, 16);          /* Filtra imediato 16 bits */
    
    aux = RI & mask_imediate_26;
    K26 = two_complements(aux, 26);          /* Filtra imediato 26 bits */
}

int main(){
    uint32_t x = 64496;
    RI = 0x01304020;                         /* add $t0, $t1, $s0 */
    decode();
    print_intruc();
    getchar();
    
    RI = 0x2128fffe;                         /* addi $t0, $t1, -2 */
    decode();
    print_intruc();
    getchar();
    
    RI = 0x08000001;                         /* j FIM */
    decode();
    print_intruc();
    getchar();
}