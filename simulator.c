#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <limits.h>
#include <inttypes.h>
#include <stdint.h>

#define MEM_SIZE 512 * 1024

bool err = false;
bool inc = true;

uint64_t registers[32];
uint64_t pc = 0;
uint64_t sp = MEM_SIZE;
int64_t readBytes;
uint64_t memC = 0;
uint8_t memory[MEM_SIZE];

void writeMemory(uint64_t address, uint64_t v)
{
    if (address >= MEM_SIZE || (address + 8) > MEM_SIZE)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
    }

    for (int x = 0; x < 8; x++)
    {
        memory[address + x] = (v >> ((7 - x) * 8)) & 0xFF;
    }
}

uint64_t readMemory(uint64_t address)
{
    if (address >= MEM_SIZE || (address + 8) > MEM_SIZE)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
    }

    uint64_t out = 0b0ull;

    for (int x = 0; x < 8; x++)
    {
        out |= (memory[address + x] << (8 * x));
    }

    return out;
}

void addFunction(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (L != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }

    registers[rd] = ((int64_t)registers[rs]) + ((int64_t)registers[rt]);
    pc += 4;
}

void addiFunction(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (rs != 0 || rt != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }

    registers[rd] = (registers[rd]) + (L);
    pc += 4;
}

void subFunction(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (L != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }

    registers[rd] = ((int64_t)registers[rs]) - ((int64_t)registers[rt]);
    pc += 4;
}

void subiFunction(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (rs != 0 || rt != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }

    registers[rd] = (registers[rd]) - (L);
    pc += 4;
}

void mulFunction(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (L != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }

    registers[rd] = ((int64_t)registers[rs]) * ((int64_t)registers[rt]);
    pc += 4;
}

void divFunction(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (L != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }
    if (registers[rt] != 0)
    {
        registers[rd] = ((int64_t)registers[rs]) / ((int64_t)registers[rt]);
        pc += 4;
    }
    else
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
    }
}

void andFunction(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (L != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }

    registers[rd] = registers[rs] & registers[rt];
    pc += 4;
}

void orFunction(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (L != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }

    registers[rd] = registers[rs] | registers[rt];
    pc += 4;
}

void xorFunction(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (L != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }

    registers[rd] = registers[rs] ^ registers[rt];
    pc += 4;
}

void notFunction(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (L != 0 || rt != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }

    registers[rd] = ~registers[rs];
    pc += 4;
}

void shiftrFunction(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (L != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }

    registers[rd] = registers[rs] >> registers[rt];
    pc += 4;
}

void shiftriFunction(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (rt != 0 || rs != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }

    registers[rd] = registers[rd] >> L;
    pc += 4;
}

void shiftlFunction(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (L != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }

    registers[rd] = registers[rs] << registers[rt];
    pc += 4;
}

void shiftliFunction(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (rs != 0 || rt != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }

    registers[rd] = registers[rd] << L;
    pc += 4;
}

void brrFunction(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (L != 0 || rt != 0 || rs != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }
    pc = registers[rd];
}

void brr1Function(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (L != 0 || rt != 0 || rs != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }

    pc = pc + registers[rd];
}

void brr2Function(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (rs != 0 || rt != 0 || rd != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }

    int32_t t = L << 20;
    t = t >> 20;
    pc = pc + t;
}

void brnzFunction(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (L != 0 || rt != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }

    if (registers[rs] == 0)
    {
        pc += 4;
    }
    else
    {
        pc = registers[rd];
    }
}

void callFunction(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (L != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }
    
    writeMemory((registers[31] - 8), (pc + 4));
    pc = registers[rd];
}

void returnFunction(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (L != 0 || rt != 0 || rs != 0 || rd != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }
    pc = readMemory((registers[31] - 8));
}

void brgtFunction(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (L != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }

    if (((int64_t)registers[rs]) <= ((int64_t)registers[rt]))
    {
        pc += 4;
    }
    else
    {
        pc = registers[rd];
    }
    inc = false;
}

void haltFunction(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (L != 0 || rd != 0 || rs != 0 || rt != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }

    exit(EXIT_SUCCESS);
}

void mov1Function(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (rt != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }

    int32_t t = L << 20;
    t = t >> 20;

    if (MEM_SIZE - t < 0 || registers[rs] + t > MEM_SIZE || registers[rs] + t < 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
    }

    registers[rd] = readMemory((registers[rs] + t));
    pc += 4;
}

void mov2Function(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (L != 0 || rt != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }

    registers[rd] = registers[rs];
    pc += 4;
}

void mov3Function(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (rs != 0 || rt != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }
    registers[rd] = (registers[rd] & 0xFFFFFFFFFFFFF000) | (L & 0xFFF);
    pc += 4;
}

void mov4Function(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (rt != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }

    int32_t t = ((int32_t)L) << 20;
    t = t >> 20;

    // if (MEM_SIZE - t < 0 || registers[rs] + t > MEM_SIZE || registers[rs] + t < 0)
    // {
    //     fprintf(stderr, "Simulation error\n");
    //     exit(EXIT_FAILURE);
    // }

    writeMemory((registers[rd] + t), registers[rs]);
    pc += 4;
}

void addFFunction(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (L != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }

    double rsDouble = *(double *)&registers[rs];
    double rtDouble = *(double *)&registers[rt];
    double sum = (rsDouble) + (rtDouble);
    registers[rd] = *((uint64_t *)&sum);
    pc += 4;
}

void subFFunction(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (L != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }

    double rsDouble = *(double *)&registers[rs];
    double rtDouble = *(double *)&registers[rt];
    double sum = (rsDouble) - (rtDouble);
    registers[rd] = *((uint64_t *)&sum);
    pc += 4;
}

void mulFFunction(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (L != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }

    double rsDouble = *(double *)&registers[rs];
    double rtDouble = *(double *)&registers[rt];
    double sum = (rsDouble) * (rtDouble);
    registers[rd] = *((uint64_t *)&sum);
    pc += 4;
}

void divFFunction(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (L != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }
    if (registers[rt] != 0)
    {
        double rsDouble = *(double *)&registers[rs];
        double rtDouble = *(double *)&registers[rt];
        double sum = (rsDouble) / (rtDouble);
        registers[rd] = *((uint64_t *)&sum);
        pc += 4;
    }
    else
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }
}

void inFunction(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (L != 0 || rt != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }

    if (registers[rs] == 0)
    {
        uint64_t input = 0;
        scanf("%llu\n", &input);
        registers[rd] = input;
        pc += 4;
    }
}

void outFunction(uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L)
{
    if (L != 0 || rt != 0)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
        return;
    }
    else
    {
        if (registers[rd] == 1)
        {
            printf("%llu\n", registers[rs]);
            pc += 4;
        }
        else
        {
            pc += 4;
        }
    }
}

int32_t twoCompFunction(int32_t n, int num)
{
    if (n >> (num - 1))
    {
        n = (~n) + 1;
        int32_t new = n & 0b00000000000000000000111111111111;
        return new;
    }
    else
    {
        return n;
    }
}

int main(int argc, char **argv)
{

    if (argv[1] == NULL || argv[2] != NULL)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
    }


    FILE *file = fopen(argv[1], "r");
    if (file == NULL)
    {
        fprintf(stderr, "Simulation error\n");
        return 1;
    }

    readBytes = fread(&memory, sizeof(char), 512 * 1024, file);
    registers[31] = (int64_t)512 * 1024;

    char buffer[33];
    size_t readChar;
    bool haltC = false;

    while (pc < MEM_SIZE)
    {
        if (pc < 0 || pc >= MEM_SIZE)
        {
            fprintf(stderr, "Simulation error\n");
            exit(EXIT_FAILURE);
        }

        uint32_t cur = (uint32_t)((memory[pc + 3] << 24) | (memory[pc + 2] << 16) | (memory[pc + 1] << 8) | memory[pc]);
        int start = pc;

        uint32_t opcode = cur >> 27;
        uint32_t rd = (cur >> 22) & 0b11111;
        uint32_t rs = (cur >> 17) & 0b11111;
        uint32_t rt = (cur >> 12) & 0b11111;

        int32_t lSig = 0;
        if (opcode == 16 || opcode == 21 || opcode == 24)
        {
            lSig = (int32_t)((cur) & 0xFFF);
            lSig = twoCompFunction(lSig, 12);
        }

        uint32_t L = (uint32_t)((cur) & 0xFFF);

        switch (opcode)
        {
        case 0:
            addFunction(rd, rs, rt, L);
            break;
        case 1:
            addiFunction(rd, rs, rt, L);
            break;
        case 2:
            subFunction(rd, rs, rt, L);
            break;
        case 3:
            subiFunction(rd, rs, rt, L);
            break;
        case 4:
            mulFunction(rd, rs, rt, L);
            break;
        case 5:
            divFunction(rd, rs, rt, L);
            break;
        case 6:
            andFunction(rd, rs, rt, L);
            break;
        case 7:
            orFunction(rd, rs, rt, L);
            break;
        case 8:
            xorFunction(rd, rs, rt, L);
            break;
        case 9:
            notFunction(rd, rs, rt, L);
            break;
        case 10:
            shiftrFunction(rd, rs, rt, L);
            break;
        case 11:
            shiftriFunction(rd, rs, rt, L);
            break;
        case 12:
            shiftlFunction(rd, rs, rt, L);
            break;
        case 13:
            shiftliFunction(rd, rs, rt, L);
            break;
        case 14:
            brrFunction(rd, rs, rt, L);
            break;
        case 15:
            brr1Function(rd, rs, rt, L);
            break;
        case 16:
            brr2Function(rd, rs, rt, L);
            break;
        case 17:
            brnzFunction(rd, rs, rt, L);
            break;
        case 18:
            callFunction(rd, rs, rt, L);
            break;
        case 19:
            returnFunction(rd, rs, rt, L);
            break;
        case 20:
            brgtFunction(rd, rs, rt, L);
            break;
        case 31:
            haltFunction(rd, rs, rt, L);
            haltC = true;
            break;
        case 21:
            mov1Function(rd, rs, rt, L);
            break;
        case 22:
            mov2Function(rd, rs, rt, L);
            break;
        case 23:
            mov3Function(rd, rs, rt, L);
            break;
        case 24:
            mov4Function(rd, rs, rt, L);
            break;
        case 25:
            addFFunction(rd, rs, rt, L);
            break;
        case 26:
            subFFunction(rd, rs, rt, L);
            break;
        case 27:
            mulFFunction(rd, rs, rt, L);
            break;
        case 28:
            divFFunction(rd, rs, rt, L);
            break;
        case 29:
            inFunction(rd, rs, rt, L);
            break;
        case 30:
            outFunction(rd, rs, rt, L);
            break;
        default:
            fprintf(stderr, "Simulation error\n");
            exit(EXIT_FAILURE);
        }

        if (start == pc)
        {
            fprintf(stderr, "Simulation error\n");
            exit(EXIT_FAILURE);
        }
    }

    if (!haltC)
    {
        fprintf(stderr, "Simulation error\n");
        exit(EXIT_FAILURE);
    }

    fclose(file);
    return 0;
}

//     fread(memory, sizeof(uint32_t), MEM_SIZE / sizeof(uint32_t), file);
//     fclose(file);

//     while (*(uint32_t *)&memory[pc] != 0xFFFFFFFF)
//     {
//         if (pc < 0 || pc >= MEM_SIZE)
//         {
//             fprintf(stderr, "Simulation error\n");
//              exit(EXIT_FAILURE);
// return;
// //         }

// //         uint32_t instruction = *(uint32_t *)&memory[pc];
// //         uint8_t opcode = (instruction >> 27) & 0x1F;

// //         if (opcode < 0x0 || opcode > 0x1f)
// //         {
// //             fprintf(stderr, "Simulation error\n");
// //              exit(EXIT_FAILURE);
// return;
// //         }

// //         if (opcode == 0x0)
// //         { // add rd, rs, rt

// //             uint8_t rd = (instruction >> 22) & 0x1F;
// //             uint8_t rs = (instruction >> 17) & 0x1F;
// //             uint8_t rt = (instruction >> 12) & 0x1F;
// //             registers[rd] = registers[rs] + registers[rt];
// //             pc += 4;
// //         }
// //         else if (opcode == 0x1)
// //         { // addi rd, L

// //             uint8_t rd = (instruction >> 22) & 0x1F;
// //             uint16_t L = instruction & 0xFFF;
// //             registers[rd] = registers[rd] + L;
// //             pc += 4;
// //         }
// //         else if (opcode == 0x2)
// //         { // sub rd, rs, rt

// //             uint8_t rd = (instruction >> 22) & 0x1F;
// //             uint8_t rs = (instruction >> 17) & 0x1F;
// //             uint8_t rt = (instruction >> 12) & 0x1F;
// //             registers[rd] = registers[rs] - registers[rt];
// //             pc += 4;
// //         }
// //         else if (opcode == 0x3)
// //         { // subi rd, L

// //             uint8_t rd = (instruction >> 22) & 0x1F;
// //             uint16_t L = instruction & 0xFFF;
// //             registers[rd] = registers[rd] - L;
// //             pc += 4;
// //         }
// //         else if (opcode == 0x4)
// //         {

// //             uint8_t rd = (instruction >> 22) & 0x1F;
// //             uint8_t rs = (instruction >> 17) & 0x1F;
// //             uint8_t rt = (instruction >> 12) & 0x1F;
// //             registers[rd] = registers[rs] * registers[rt];
// //             pc += 4;
// //         }
// //         else if (opcode == 0x5)
// //         {

// //             int8_t rd = (instruction >> 22) & 0x1F;
// //             uint8_t rs = (instruction >> 17) & 0x1F;
// //             uint8_t rt = (instruction >> 12) & 0x1F;

// //             // ERROR_INT_DIV_ZERO
// //             if (registers[rt] == 0)
// //             {
// //                 printf("Simulation error\n");
// //                  exit(EXIT_FAILURE);
// return;
// //             }

// //             registers[rd] = registers[rs] / registers[rt];
// //             pc += 4;
// //         }
// //         else if (opcode == 0x6)
// //         {

// //             uint8_t rd = (instruction >> 22) & 0x1F;
// //             uint8_t rs = (instruction >> 17) & 0x1F;
// //             uint8_t rt = (instruction >> 12) & 0x1F;
// //             registers[rd] = registers[rs] & registers[rt];
// //             pc += 4;
// //         }

// //         else if (opcode == 0x7)
// //         {

// //             uint8_t rd = (instruction >> 22) & 0x1F;
// //             uint8_t rs = (instruction >> 17) & 0x1F;
// //             uint8_t rt = (instruction >> 12) & 0x1F;
// //             registers[rd] = registers[rs] | registers[rt];
// //             pc += 4;
// //         }
// //         else if (opcode == 0x8)
// //         {

// //             uint8_t rd = (instruction >> 22) & 0x1F;
// //             uint8_t rs = (instruction >> 17) & 0x1F;
// //             uint8_t rt = (instruction >> 12) & 0x1F;
// //             registers[rd] = registers[rs] ^ registers[rt];
// //             pc += 4;
// //         }
// //         else if (opcode == 0x9)
// //         {

// //             uint8_t rd = (instruction >> 22) & 0x1F;
// //             uint8_t rs = (instruction >> 17) & 0x1F;
// //             registers[rd] = ~registers[rs];
// //             pc += 4;
// //         }
// //         else if (opcode == 0xa)
// //         {

// //             uint8_t rd = (instruction >> 22) & 0x1F;
// //             uint8_t rs = (instruction >> 17) & 0x1F;
// //             uint8_t rt = (instruction >> 12) & 0x1F;
// //             registers[rd] = registers[rs] >> registers[rt];
// //             pc += 4;
// //         }
// //         else if (opcode == 0xb)
// //         {

// //             uint8_t rd = (instruction >> 22) & 0x1F;
// //             uint16_t L = instruction & 0xFFF;
// //             registers[rd] = registers[rd] >> L;
// //             pc += 4;
// //         }
// //         else if (opcode == 0xc)
// //         {

// //             uint8_t rd = (instruction >> 22) & 0x1F;
// //             uint8_t rs = (instruction >> 17) & 0x1F;
// //             uint8_t rt = (instruction >> 12) & 0x1F;
// //             registers[rd] = registers[rs] << registers[rt];
// //             pc += 4;
// //         }
// //         else if (opcode == 0xd)
// //         {

// //             uint8_t rd = (instruction >> 22) & 0x1F;
// //             uint16_t L = instruction & 0xFFF;
// //             registers[rd] = registers[rd] << L;
// //             pc += 4;
// //         }
// //         else if (opcode == 0x0e)
// //         {

// //             uint8_t rd = (instruction >> 22) & 0x1F;
// //             pc = registers[rd];
// //             pc += 4;
// //         }
// //         else if (opcode == 0x0f)
// //         {

// //             uint8_t rd = (instruction >> 22) & 0x1F;
// //             pc = pc + registers[rd];
// //             pc += 4;
// //         }
// //         else if (opcode == 0x10)
// //         {

// //             uint16_t L = instruction & 0xFFF;
// //             pc = pc + L;
// //             pc += 4;
// //         }
// //         else if (opcode == 0x11)
// //         {

// //             uint8_t rd = (instruction >> 22) & 0x1F;
// //             uint8_t rs = (instruction >> 17) & 0x1F;
// //             if (registers[rs] != 0)
// //             {
// //                 pc = registers[rd];
// //             }
// //             else
// //             {
// //                 pc = pc + 4;
// //             }
// //             pc += 4;
// //         }

// //         else if (opcode == 0x12)
// //         {

// //             uint8_t rd = (instruction >> 22) & 0x1F;
// //             uint8_t rs = (instruction >> 17) & 0x1F;
// //             uint8_t rt = (instruction >> 12) & 0x1F;

// //             if ((registers[31] - 8) < 0 || (registers[31] - 8) >= MEM_SIZE)
// //             {
// //                 printf("Simulation error\n");
// //                  exit(EXIT_FAILURE);
// return;
// //             }

// //             if ((registers[31] - 8) % sizeof(uint64_t) != 0)
// //             {
// //                 printf("Simulation error\n");
// //                  exit(EXIT_FAILURE);
// return;
// //             }

// //             memory[registers[31] - 8] = pc + 4;
// //             pc = registers[rd];

// //             pc += 4;
// //         }

// //         else if (opcode == 0x13)
// //         {

// //             if ((registers[31] - 8) < 0 || (registers[31] - 8) >= MEM_SIZE)
// //             {
// //                 printf("Simulation error\n");
// //                  exit(EXIT_FAILURE);
// return;
// //             }

// //             if ((registers[31] - 8) % sizeof(uint64_t) != 0)
// //             {
// //                 printf("Simulation error\n");
// //                  exit(EXIT_FAILURE);
// return;
// //             }

// //             pc = memory[registers[31] - 8];
// //             pc += 4;
// //         }
// //         else if (opcode == 0x14)
// //         {

// //             uint8_t rd = (instruction >> 22) & 0x1F;
// //             uint8_t rs = (instruction >> 17) & 0x1F;
// //             uint8_t rt = (instruction >> 12) & 0x1F;
// //             if ((int64_t)registers[rs] <= (int64_t)registers[rt])
// //             {
// //                 pc = pc + 4;
// //             }
// //             else
// //             {
// //                 pc = registers[rd];
// //             }

// //             pc += 4;
// //         }
// //         else if (opcode == 0x1f)
// //         {

// //             return 0;
// //             pc += 4;
// //         }
// //         else if (opcode == 0x15)
// //         {

// //             uint8_t rd = (instruction >> 22) & 0x1F;
// //             uint8_t rs = (instruction >> 17) & 0x1F;
// //             uint16_t L = instruction & 0xFFF;

// //             int64_t mem_address = (int64_t)registers[rs] - L;

// //             // if (mem_address < 0 || mem_address >= MEM_SIZE)
// //             // {
// //             //     fprintf(stderr, "Simulation error: Memory access out of bounds\n");
// //             //      exit(EXIT_FAILURE);
// return;
// //             // }

// //             int32_t t = L << 20;
// //             t = t >> 20;
// //             pc = pc + t;
// //             if (MEM_SIZE - t < 0 || registers[rs] + t < 0 || registers[rs] + t > MEM_SIZE)
// //             {
// //                 fprintf(stderr, "Simulation error\n");
// //                  exit(EXIT_FAILURE);
// return;
// //             }

// //             // if ((registers[rs] + L) < 0 || (registers[rs] + L) >= MEM_SIZE)
// //             // {
// //             //     printf("Simulation error\n");
// //             //      exit(EXIT_FAILURE);
// return;
// //             // }

// //             // if (MEM_SIZE - L < 0 || (int64_t)(registers[rs] + L) < 0 || (int64_t)(registers[rs] + L) >= MEM_SIZE)
// //             // {
// //             //     fprintf(stderr, "Simulation error: Memory access out of bounds\n");
// //             //      exit(EXIT_FAILURE);
// return;
// //             // }

// //             // if ((registers[rs] + L) % sizeof(uint64_t) != 0)
// //             // {
// //             //     printf("Simulation error\n");
// //             //      exit(EXIT_FAILURE);
// return;
// //             // }

// //             registers[rd] = memory[(uint64_t)(registers[rs] + L)];
// //             pc += 4;
// //         }
// //         else if (opcode == 0x16)
// //         {

// //             uint8_t rd = (instruction >> 22) & 0x1F;
// //             uint8_t rs = (instruction >> 17) & 0x1F;
// //             registers[rd] = registers[rs];
// //             pc += 4;
// //         }
// //         else if (opcode == 0x17)
// //         {

// //             uint8_t rd = (instruction >> 22) & 0x1F;
// //             uint64_t L = instruction & 0xFFFFFFFFFFF; // extract bits 52:63
// //             registers[rd] = L;
// //             pc += 4;
// //         }
// //         else if (opcode == 0x18)
// //         {

// //             uint8_t rd = (instruction >> 22) & 0x1F;
// //             uint8_t rs = (instruction >> 17) & 0x1F;
// //             uint64_t L = instruction & 0xFFF;

// //             // if ((registers[rs] + L) < 0 || (registers[rs] + L) >= MEM_SIZE)
// //             // {
// //             //     printf("Simulation error\n");
// //             //      exit(EXIT_FAILURE);
// return;
// //             // }

// //             // if ((registers[rs] + L) % sizeof(uint64_t) != 0)
// //             // {
// //             //     printf("Simulation error\n");
// //             //      exit(EXIT_FAILURE);
// return;
// //             // }

// //             int32_t t = L << 20;
// //             t = t >> 20;
// //             pc = pc + t;
// //             if (MEM_SIZE - t < 0 || registers[rs] + t < 0 || registers[rs] + t > MEM_SIZE)
// //             {
// //                 fprintf(stderr, "Simulation error\n");
// //                  exit(EXIT_FAILURE);
// return;
// //             }

// //             memory[registers[rd] + L] = registers[rs];
// //             pc += 4;
// //         }
// //         else if (opcode == 0x19)
// //         {

// //             uint8_t rd = (instruction >> 22) & 0x1F;
// //             uint8_t rs = (instruction >> 17) & 0x1F;
// //             uint8_t rt = (instruction >> 12) & 0x1F;
// //             double *doublePrecisionRegisters = (double *)registers;
// //             doublePrecisionRegisters[rd] = doublePrecisionRegisters[rs] + doublePrecisionRegisters[rt];
// //             pc += 4;
// //         }
// //         else if (opcode == 0x1a)
// //         {

// //             uint8_t rd = (instruction >> 22) & 0x1F;
// //             uint8_t rs = (instruction >> 17) & 0x1F;
// //             uint8_t rt = (instruction >> 12) & 0x1F;
// //             double *doublePrecisionRegisters = (double *)registers;
// //             doublePrecisionRegisters[rd] = doublePrecisionRegisters[rs] - doublePrecisionRegisters[rt];
// //             pc += 4;
// //         }
// //         else if (opcode == 0x1b)
// //         {

// //             uint8_t rd = (instruction >> 22) & 0x1F;
// //             uint8_t rs = (instruction >> 17) & 0x1F;
// //             uint8_t rt = (instruction >> 12) & 0x1F;
// //             double *doublePrecisionRegisters = (double *)registers;
// //             doublePrecisionRegisters[rd] = doublePrecisionRegisters[rs] * doublePrecisionRegisters[rt];
// //             pc += 4;
// //         }
// //         else if (opcode == 0x1c)
// //         {

// //             uint8_t rd = (instruction >> 22) & 0x1F;
// //             uint8_t rs = (instruction >> 17) & 0x1F;
// //             uint8_t rt = (instruction >> 12) & 0x1F;
// //             double *doublePrecisionRegisters = (double *)registers;

// //             if (doublePrecisionRegisters[rt] == 0.0)
// //             {
// //                 printf("Simulation error\n");
// //                  exit(EXIT_FAILURE);
// return;
// //             }

// //             doublePrecisionRegisters[rd] = doublePrecisionRegisters[rs] / doublePrecisionRegisters[rt];
// //             pc += 4;
// //         }
// //         else if (opcode == 0x1d)
// //         {

// //             uint8_t rd = (instruction >> 22) & 0x1F;
// //             uint8_t rs = (instruction >> 17) & 0x1F;

// //             printf("%d", rs);
// //             scanf("%lu", &registers[rd]);
// //             pc += 4;
// //         }
// //         else if (opcode == 0x1e)
// //         {

// //             uint8_t rd = (instruction >> 22) & 0x1F;
// //             uint8_t rs = (instruction >> 17) & 0x1F;

// //             printf("%lu\n", registers[rs]);
// //             pc += 4;
// //         }
// //         else
// //         {
// //             fprintf(stderr, "Simulation Error\n");
// //              exit(EXIT_FAILURE);
// return;
// //         }
//     }
// }