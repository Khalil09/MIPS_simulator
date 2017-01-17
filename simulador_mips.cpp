#include <bits/stdc++.h>

using namespace std;

#define MEM_SIZE 4096
#define MEM_INTR 0
#define MEM_DADO 2048

#define WHITE_BACK     "\x1b[47m"
#define BLACK_BACK     "\x1b[40m"
#define WHITE          "\x1b[37m"
#define BLACK          "\x1b[30m"
#define RESET          "\x1b[0m"

FILE *fp;

bool flag_run = true;
bool can_advance = true;
int cont = 0;

uint32_t opcode,rs,rt,rd,shamt,funct,RI; /* Segmentos das instrucoes */
int32_t  K16, K26; 

uint32_t mem[MEM_SIZE];
uint32_t breg[32];
uint32_t PC;
uint32_t HI, LO;

enum OPCODES {
    ADDI=0x08, ANDI=0x0C, BEQ=0x04, BNE=0x05, EXT=0x00, J=0x02, JAL=0x03,
    LW=0x23, ORI=0x0D, SW=0x2B
};

enum FUNCT {
    ADD=0x20, SUB=0x22, MULT=0x18, AND=0x24, OR=0x25, XOR=0x26, NOR=0x27, SLT=0x2A,
    JR=0x08, SLL=0x00, SRL=0x02, SRA=0x03, SYSCALL=0x0c, MFHI=0x10, MFLO=0x12
};

int32_t two_complements(uint32_t numb){
    uint32_t mask_16 = 0x8000;
    uint32_t mask_26 = 0x2000000;
    uint32_t result = numb;
    uint32_t aux;
    
    aux = numb >> 15;
    if(aux & 1)
        result = numb ^ 0xffff0000;
    else
        result = numb;
    
    return result;
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
    if(breg[2] == 10) flag_run = false;
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
    K16 = two_complements(aux);          /* Filtra imediato 16 bits */
    
    aux = RI & mask_imediate_26;
    K26 = two_complements(aux);          /* Filtra imediato 26 bits */
}

void fetch(){
    RI = mem[PC/4];
}

void execute(){
    int32_t end = 0;
    int32_t aux;
    int64_t aux_mult;
    int64_t mask_HI_LO = 0x00000000FFFFFFFF;
    
    
    switch (opcode) {
        case ADDI: /* addi */
            breg[rt] = breg[rs] + K16;
            PC += 4;
            break;
            
        case ANDI: /* andi */
            breg[rt] = breg[rs] & K16;
            PC += 4;
            break;
        
        case BEQ: /* beq */
            if(breg[rs] == breg[rt]){
                end = K16 << 2;
                PC += end+4;
            } else PC += 4;
            break;
            
        case BNE: /* bne */
            if(breg[rs] != breg[rt]){
                end = K16 << 2;
                printf("K16 = %d\n", K16);
                PC += end+4;
            } else PC += 4;
            break;
            
        case J: /* J */
            end = (PC & 0xF0000000) | (K26 << 2);
            PC = end;
            break;
            
        case JAL: /* jal */
            breg[31] = PC + 4;
            end = (PC & 0xF0000000) | (K26 << 2);
            PC = end;
            break;
            
        case ORI: /* ori */
            breg[rt] = breg[rs] | K16;
            PC += 4; 
            break;
            
        case LW: /* lw */
            end = breg[rs] + K16;
            breg[rt] = mem[end/4];
            PC += 4;
            break;
            
        case SW: /* sw */
            end = breg[rs] + K16;
            mem[end/4] = breg[rt];
            PC += 4;
            break;
            
        default: /* ext */
            break;
    }
    if(opcode == 0){
        switch (funct) {
            case ADD: /* add */
                breg[rd] = breg[rs] + breg[rt];
                PC += 4;
                break;
                
            case SUB: /* sub */
                breg[rd] = breg[rs] - breg[rt];
                PC += 4;
                break;
                
            case MULT: /* mult */
                aux_mult = (int64_t)breg[rs] * breg[rt];
                LO = (int32_t)aux_mult & mask_HI_LO;
                aux_mult = aux_mult >> 32;
                HI = (int32_t)aux_mult & mask_HI_LO;
                PC += 4;
                break;
            
            case AND: /* and */
                breg[rd] = breg[rs] & breg[rt];
                PC += 4;
                break;
                
            case OR: /* or */
                breg[rd] = breg[rs] | breg[rt];
                PC += 4;
                break;
                
            case XOR: /* xor */
                breg[rd] = (~breg[rs] & ~breg[rt]);
                PC += 4;
                break;
            
            case NOR: /* nor */
                breg[rd] = ~(breg[rs] | breg[rt]);
                PC += 4;
                break;
                
            case SLT: /* slt */
                if(breg[rs] < breg[rt]) breg[rd] = 1;
                else breg[rd] = 0;
                PC += 4;
                break;
            
            case JR: /* jr */
                PC = breg[rs];
                break;
            
            case SLL: /* sll */
                breg[rd] = breg[rt] << shamt;
                PC += 4;
                break;
                
            case SRL: /* srl */
                breg[rd] = breg[rt] >> shamt;
                PC += 4;

                break;
                
            case SRA: /* sra */
                breg[rd] = (int32_t)breg[rt] >> shamt;
                PC += 4;
                break;
                
            case SYSCALL:
                syscall();
                PC += 4;
                break;
                
            case MFHI: /* MFHI */
                breg[rd] = HI;
                PC += 4;
                break;
            
            case MFLO: /* MFLO */
                breg[rd] = LO;
                PC += 4;
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
        printf(WHITE_BACK "|$zero|0x%08x|\n" RESET, breg[0]);
        printf("|$at  |0x%08x|\n", breg[1]);
        printf(WHITE_BACK "|$v0  |0x%08x|\n" RESET, breg[2]);
        printf("|$v1  |0x%08x|\n", breg[3]);
        for(int i = 4; i < 24; i++){
            if (i < 8){
                if(cont_A % 2 == 0)
                    printf(WHITE_BACK "|$a%d  |0x%08x|\n" RESET, cont_A, breg[i]);
                else 
                    printf("|$a%d  |0x%08x|\n", cont_A, breg[i]);
                cont_A++;
            } else if(i < 16) {
                if(cont_T % 2 ==0)
                    printf(WHITE_BACK "|$t%d  |0x%08x|\n" RESET, cont_T, breg[i]);
                else
                    printf("|$t%d  |0x%08x|\n", cont_T, breg[i]);
                cont_T++;
            }  else {
                if(cont_S % 2 == 0)
                    printf(WHITE_BACK "|$s%d  |0x%08x|\n" RESET, cont_S, breg[i]);
                else
                    printf("|$s%d  |0x%08x|\n", cont_S, breg[i]);
                cont_S++;
            }
        }
        printf(WHITE_BACK "|$t8  |0x%08x|\n" RESET, breg[24]);
        printf("|$t9  |0x%08x|\n", breg[25]);
        printf(WHITE_BACK "|$k0  |0x%08x|\n" RESET, breg[26]);
        printf("|$k1  |0x%08x|\n", breg[27]);
        printf(WHITE_BACK "|$gp  |0x%08x|\n" RESET, breg[28]);
        printf("|$sp  |0x%08x|\n", breg[29]);
        printf(WHITE_BACK "|$fp  |0x%08x|\n" RESET, breg[30]);
        printf("|$ra  |0x%08x|\n", breg[31]);
        printf(WHITE_BACK "|pc   |0x%08x|\n" RESET, PC);
        printf("|hi   |0x%08x|\n", HI);
        printf(WHITE_BACK "|lo   |0x%08x|\n" RESET, LO);        
        printf("==================\n");
    }
    
    if(format == 'd'){
        printf("==================\n");
        printf(WHITE_BACK "|$zero|%10i|\n" RESET, breg[0]);
        printf("|$at  |%10i|\n", breg[1]);
        printf(WHITE_BACK "|$v0  |%10i|\n" RESET, breg[2]);
        printf("|$v1  |%10i|\n", breg[3]);
        for(int i = 4; i < 24; i++){
            if (i < 8){
                if(cont_A % 2 == 0)
                    printf(WHITE_BACK "|$a%d  |%10i|\n" RESET, cont_A, breg[i]);
                else 
                    printf("|$a%d  |%10i|\n", cont_A, breg[i]);
                cont_A++;
            } else if(i < 16) {
                if(cont_T % 2 ==0)
                    printf(WHITE_BACK "|$t%d  |%10i|\n" RESET, cont_T, breg[i]);
                else
                    printf("|$t%d  |%10i|\n", cont_T, breg[i]);
                cont_T++;
            }  else {
                if(cont_S % 2 == 0)
                    printf(WHITE_BACK "|$s%d  |%10i|\n" RESET, cont_S, breg[i]);
                else
                    printf("|$s%d  |%10i|\n", cont_S, breg[i]);
                
                cont_S++;
            }
        }
        printf(WHITE_BACK "|$t8  |%10i|\n" RESET, breg[24]);
        printf("|$t9  |%10i|\n", breg[25]);
        printf(WHITE_BACK "|$k0  |%10i|\n" RESET, breg[26]);
        printf("|$k1  |%10i|\n", breg[27]);
        printf(WHITE_BACK "|$gp  |%10i|\n" RESET, breg[28]);
        printf("|$sp  |%10i|\n", breg[29]);
        printf(WHITE_BACK "|$fp  |%10i|\n" RESET, breg[30]);
        printf("|$ra  |%10i|\n", breg[31]);
        printf(WHITE_BACK "|pc   |%10i|\n" RESET, PC);
        printf("|hi   |%10i|\n", HI);
        printf(WHITE_BACK "|lo   |%10i|\n" RESET, LO);        
        printf("==================\n");
    }
}

void dump_mem(int start, int end, char format){
    printf("\n==================================================\n");
    if(format == 'h'){
        for (int i = start; i <= end; i++){
            printf("|MEM[%4i] | 0x%08x|+", i, mem[i]);
            if(i % 2 == 1) printf("\n");
        }
    } else {
        for (int i = start; i <= end; i++){
            printf("|MEM[%4i] | %10i|+", i, mem[i]);
            if(i % 2 == 1) printf("\n");
        }
    }
    printf("\n==================================================\n");
}

void step(){
    fetch();
    decode();
    print_intruc();
    execute();
}

void run(){
    while(flag_run){
        step();
    }
}

void read_bin(){
    uint32_t result;
    int i = 0;
    
    fp = fopen("data.bin", "rb");
    
    if(fp == NULL){ 
        printf("Nao existe tal arquivo\n");
        getchar();
    } else {
        while(fread(&result, sizeof(uint32_t), 1, fp) == 1){
            mem[MEM_DADO + i] = result;
            i++;
        }
    }
    
    fclose(fp);
    
    fp = fopen("text.bin", "rb");
    
    if(fp == NULL){ 
        printf("Nao existe tal arquivo\n");
        getchar();
    } else {    
        i = 0;
        while(fread(&result, sizeof(uint32_t), 1, fp) == 1){
            mem[MEM_INTR + i] = result;
            i++;
            cont++;
        }    
    }
    
    fclose(fp);
}

int main(){
    PC = 0;
    breg[0] = 0;
    
    read_bin();
    
    run();
    dump_reg('d');
    dump_mem(0, 40, 'h');
    dump_mem(2048, 2055, 'd');
}