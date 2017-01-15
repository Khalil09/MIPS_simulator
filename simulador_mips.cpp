#include <bits/stdc++.h>

using namespace std;

#define MEM_SIZE 4096
#define WHITE     "\x1b[47m"
#define CYAN     "\x1b[46m"
#define RESET    "\x1b[0m"


uint32_t opcode,rs,rt,rd,shamt,funct,RI; /* Segmentos das instrucoes */
int32_t  K16, K26; 

uint32_t mem[MEM_SIZE];
uint32_t breg[32];
uint32_t PC;
uint32_t HI, LO;

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
int32_t two_complements(int32_t numb, int bits){
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
    if(numb != 65535 && numb < 0) return result*2;
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

void syscall(){
    
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

void fetch(){
    RI = mem[PC];
    PC += 1; /* PC + 4 */
}

void execute(){
    int32_t end;
    int64_t aux_mult;
    int64_t mask_HI_LO = 0x00000000FFFFFFFF;
    
    
    switch (opcode) {
        case 0x08: /* addi*/
        printf("Entrei aqui\n");
            breg[rt] = breg[rd] + K16;
            break;
            
        case 0x0C: /* andi */
            breg[rt] = breg[rs] & K16;    
            break;
        
        case 0x04: /* beq */
            if(rs == rt){
                end = K16 >> 2;
                PC = end;
            }
            break;
            
        case 0x05: /* bne */
            if(rs != rt){
                end = K16 >> 2;
                PC = end;
            }
            break;
            
        case 0x02: /* J */
            end = K26 >> 2;
            PC = end;
            break;
            
        case 0x03: /* jal */
            breg[31] = PC + 1;
            end = K26 >> 2;
            PC = end;
            break;
            
        case 0x0D: /* ori */
            breg[rt] = breg[rs] | K16; 
            break;
            
        case 0x23: /* lw */
            end = breg[rs] + K16;
            breg[rt] = mem[end >> 2];
            break;
            
        default: /* ext */
            break;
    }
    if(opcode == 0){
        switch (funct) {
            case 0x20: /* add */
                breg[rd] = breg[rs] + breg[rt];
                break;
                
            case 0x22: /* sub */
                breg[rd] = breg[rs] - breg[rt];
                break;
                
            case 0x18: /* mult */
                aux_mult = (int64_t)breg[rs] * breg[rt];            
                LO = aux_mult & mask_HI_LO;
                aux_mult = aux_mult >> 32;
                HI = aux_mult & mask_HI_LO;
                break;
            
            case 0x24: /* and */
                breg[rd] = breg[rs] & breg[rt];
                break;
                
            case 0x25: /* or */
                breg[rd] = breg[rs] | breg[rt];
                break;
                
            case 0x26: /* xor */
                breg[rd] = (~breg[rs] & ~breg[rt]);
                break;
            
            case 0x27: /* nor */
                breg[rd] = ~(breg[rs] | breg[rt]);
                break;
                
            case 0x2A: /* slt */
                if(breg[rs] < breg[rt]) breg[rd] = 1;
                else 0;
                break;
            
            case 0x08: /* jr */
                PC = breg[rs] >> 2;
                break;
            
            case 0x00: /* sll */
                breg[rd] = breg[rt] << K16;
                break;
                
            case 0x02: /* srl */
                breg[rd] = breg[rt] >> K16;
                break;
                
            case 0x03: /* sra */
                break;
                
            case 0x0C:
                syscall();
                break;
                
            case 0x10:
                breg[rd] = HI;
                break;
            
            case 0x12:
                breg[rd] = LO;
                break;
        }
    }
}

void dump_reg(char format){
    int cont_A = 0;
    int cont_T = 0;
    int cont_S = 0;  
    
    if(format == 'h'){
        printf("  REGISTRADORES\n");
        printf("==================\n");
        printf(WHITE "|$zero|0x%08x|\n" RESET, breg[0]);
        printf("|$at  |0x%08x|\n", breg[1]);
        printf(WHITE "|$v0  |0x%08x|\n" RESET, breg[2]);
        printf("|$v1  |0x%08x|\n", breg[3]);
        for(int i = 4; i < 24; i++){
            if (i < 8){
                if(cont_A % 2 == 0)
                    printf(WHITE "|$a%d  |0x%08x|\n" RESET, cont_A, breg[i]);
                else 
                    printf("|$a%d  |0x%08x|\n", cont_A, breg[i]);
                cont_A++;
            } else if(i < 16) {
                if(cont_T % 2 ==0)
                    printf(WHITE "|$t%d  |0x%08x|\n" RESET, cont_T, breg[i]);
                else
                    printf("|$t%d  |0x%08x|\n", cont_T, breg[i]);
                cont_T++;
            }  else {
                if(cont_S % 2 == 0)
                    printf(WHITE "|$s%d  |0x%08x|\n" RESET, cont_S, breg[i]);
                else
                    printf("|$s%d  |0x%08x|\n", cont_S, breg[i]);
                cont_S++;
            }
        }
        printf(WHITE "|$t8  |0x%08x|\n" RESET, breg[24]);
        printf("|$t9  |0x%08x|\n", breg[25]);
        printf(WHITE "|$k0  |0x%08x|\n" RESET, breg[26]);
        printf("|$k1  |0x%08x|\n", breg[27]);
        printf(WHITE "|$gp  |0x%08x|\n" RESET, breg[28]);
        printf("|$sp  |0x%08x|\n", breg[29]);
        printf(WHITE "|$fp  |0x%08x|\n" RESET, breg[30]);
        printf("|$ra  |0x%08x|\n", breg[31]);
        printf(WHITE "|pc   |0x%08x|\n" RESET, PC);
        printf("|hi   |0x%08x|\n", HI);
        printf(WHITE "|lo   |0x%08x|\n" RESET, LO);        
        printf("==================\n");
    }
    
    if(format == 'd'){
        printf("==================\n");
        printf(WHITE "|$zero|%10i|\n" RESET, breg[0]);
        printf("|$at  |%10i|\n", breg[1]);
        printf(WHITE "|$v0  |%10i|\n" RESET, breg[2]);
        printf("|$v1  |%10i|\n", breg[3]);
        for(int i = 4; i < 24; i++){
            if (i < 8){
                if(cont_A % 2 == 0)
                    printf(WHITE "|$a%d  |%10i|\n" RESET, cont_A, breg[i]);
                else 
                    printf("|$a%d  |%10i|\n", cont_A, breg[i]);
                cont_A++;
            } else if(i < 16) {
                if(cont_T % 2 ==0)
                    printf(WHITE "|$t%d  |%10i|\n" RESET, cont_T, breg[i]);
                else
                    printf("|$t%d  |%10i|\n", cont_T, breg[i]);
                cont_T++;
            }  else {
                if(cont_S % 2 == 0)
                    printf(WHITE "|$s%d  |%10i|\n" RESET, cont_S, breg[i]);
                else
                    printf("|$s%d  |%10i|\n", cont_S, breg[i]);
                
                cont_S++;
            }
        }
        printf(WHITE "|$t8  |%10i|\n" RESET, breg[24]);
        printf("|$t9  |%10i|\n", breg[25]);
        printf(WHITE "|$k0  |%10i|\n" RESET, breg[26]);
        printf("|$k1  |%10i|\n", breg[27]);
        printf(WHITE "|$gp  |%10i|\n" RESET, breg[28]);
        printf("|$sp  |%10i|\n", breg[29]);
        printf(WHITE "|$fp  |%10i|\n" RESET, breg[30]);
        printf("|$ra  |%10i|\n", breg[31]);
        printf(WHITE "|pc   |%10i|\n" RESET, PC);
        printf("|hi   |%10i|\n", HI);
        printf(WHITE "|lo   |%10i|\n" RESET, LO);        
        printf("==================\n");
    }
}

void dump_mem(){
    
}

void run(){
    fetch();
    decode();
    print_intruc();
    getchar();
    execute();
}

int main(){
    mem[0] = 0x01090018;
    
    PC = 0;
    breg[0] = 0;
    
    breg[8] = 200000;
    breg[9] = 5000000;
    run();
    
    dump_reg('d');
}