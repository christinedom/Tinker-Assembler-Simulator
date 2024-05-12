#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>

struct data
{
    // make unsigned
    // i dont think this is needed!
    // char type[5];
    int opcode;
    // make rd, rs, and rt unsigned
    uint32_t rd;
    uint32_t rs;
    uint32_t rt;
    // L has to be unsigned or signed, so make two different variables?!
    // should L be a short (question after discussison)
    // short L;
    short L;
    // track line number
    // hmmmm is this needed ... no!
    // int address;
};

struct label
{
    char *name;
    int lineNumber;
    int address;
};

bool isLabelDefined(char *labelName, struct label *labelArray, int numberOfColons)
{
    for (int i = 0; i < numberOfColons; i++)
    {
        if (strcmp(labelName, labelArray[i].name) == 0)
        {
            return true;
        }
    }
    return false;
}

int regexCheck(char *line, regex_t *regexArray, int regexSize)
{

    regmatch_t match;
    for (int i = 0; i < regexSize; i++)
    {
        if (regexec(&regexArray[i], line, 1, &match, 0) == 0)
        {
            return i; // return the opcode if matched regex
        }
    }
    return -1;
}

bool removeFile(const char *filename)
{
    if (remove(filename) == 0)
    {
        printf("Removed file: %s\n", filename);
        return true;
    }
    else
    {
        printf("Failed to remove file: %s\n", filename);
        return false;
    }
}

int handleMissingCodeSection(FILE *output, const char *outputFile)
{
    fprintf(stderr, "Error: No code section found in the input file.\n");
    fclose(output);
    if (!removeFile(outputFile))
    {
        return -1;
    }
    return -1;
}

void instructionOutput(FILE *output, uint32_t opcode, uint32_t rd, uint32_t rs, uint32_t rt, short L)
{
    printf("opcode: %d, rd: %d, rs: %d, rt: %d, L: %d\n", opcode, rd, rs, rt, L);
    uint32_t instr = 0;
    instr |= (opcode & 0x1f) << 27;
    instr |= (rd & 0x1f) << 22;
    instr |= (rs & 0x1f) << 17;
    instr |= (rt & 0x1f) << 12;
    instr |= L & 0xFFF;
    fwrite((const void *)&instr, sizeof(instr), 1, output);
}

int main(int argc, char **argv)
{

    // checks to see if there are arguments passed, must be 2 LOL
    if (argc != 2)
    {
        fprintf(stderr, "Invalid tinker filepath\n");
        // first did exit? but don't think that is right, so change to return (any non-zero number)
        return 1;
    }

    // checks to see if any argument is passed (doesn't check if valid or whateverr)
    if (argv[1] == NULL)
    {
        fprintf(stderr, "Invalid tinker filepath\n");
        return 1;
    }

    char *path = argv[1];
    FILE *fptr = fopen(path, "r");

    // tries to open file to see if it exists!
    if (fptr == NULL)
    {
        fprintf(stderr, "Invalid tinker filepath\n");
        return 1;
    }

    // checks to see if the file has a .tk extension (has to equal .tk and must not have nothing!)
    char *extension = strrchr(argv[1], '.');
    if (extension == NULL || strcmp(extension, ".tk") != 0)
    {
        fprintf(stderr, "Invalid tinker filepath\n");
        return 1;
    }

    struct data *instructions = malloc(sizeof(struct data) * 1000);
    char outputPath[256];     // Adjust the size according to your needs
    strcpy(outputPath, path); // Make a copy of the input file path
    char *outputFile = strtok(outputPath, ".");
    strcat(outputFile, ".tko");
    FILE *output = fopen(outputFile, "w");

    // reading file and putting it in a character array!
    fseek(fptr, 0L, SEEK_END);
    long fileSize = ftell(fptr);
    char *buffer = (char *)malloc(fileSize * sizeof(char) + 1);
    buffer[fileSize] = '\0';
    rewind(fptr);

    size_t bytesRead = fread(buffer, 1, fileSize, fptr);
    char split[] = "\n";
    int numberOfLines = 0;

    regex_t regexArray[37];
    // regex patterns to compare the instructions
    const char *opcodePatterns[] = {
        // arithmetic
        "^\tadd r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0])$", // add rd, rs, rt - 0
        "^\taddi r([0-9]|1[0-9]|2[0-9]|3[0]), ([0-9]+)$",                                                // addi rd, L - 1
        "^\tsub r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0])$", // sub rd, rs, rt - 2
        "^\tsubi r([0-9]|1[0-9]|2[0-9]|3[0]), ([0-9_-]+)$",                                              // subi rd, L - 3
        "^\tmul r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0])$", // mul rd, rs, rt - 4
        "^\tdiv r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0])$", // div rd, rs, rt - 5

        // logic
        "^\tand r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0])$",   // and rd, rs, rt - 6
        "^\tor r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0])$",    // or rd, rs, rt - 7
        "^\txor r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0])$",   // xor rd, rs, rt // - 8
        "^\tnot r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0])$",                                // not rd, rs - 9
        "^\tshftr r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0])$", // shftr rd, rs, rt - 10
        "^\tshftri r([0-9]|1[0-9]|2[0-9]|3[0]), ([0-9_-]+)$",                                              // shftri rd, L - 11
        "^\tshftl r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0])$", // shftl rd, rs, rt - 12
        "^\tshftli r([0-9]|1[0-9]|2[0-9]|3[0]), ([0-9_-]+)$",                                              // shftli rd, L - 13

        // control
        "^\tbr r([0-9]|1[0-9]|2[0-9]|3[0])$",                                                             // br rd - 14
        "^\tbrr r([0-9]|1[0-9]|2[0-9]|3[0])$",                                                            // brr rd - 15
        "^\tbrr ([0-9_-]+)$",                                                                             // brr L - 16
        "^\tbrnz r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0])$",                              // brnz rd, rs - 17
        "^\tcall r([0-9]|1[0-9]|2[0-9]|3[0])$",                                                           // call rd,  - 18
        "^\treturn$",                                                                                     // return - 19
        "^\tbrgt r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0])$", // brgt rd, rs, rt - 20                                                                                     // halt - 21

        // data movement
        // hmmm check if these regex expressions are correct!
        // for 1st and 4th expression could it be like this?
        //"\tmov \\(r([0-9]|1[0-9]|2[0-9]|3[0])\\)\\(([0-9]|1[0-9]|2[0-9]|3[0])\\), r([0-9]|1[0-9]|2[0-9]|3[0])$"
        "^\tmov r([0-9]|1[0-9]|2[0-9]|3[0]), \\(r([0-9]|1[0-9]|2[0-9]|3[0])\\)\\(([0-9_-]+)\\)$", // mov rd, rs L - 22
        "^\tmov r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0])$",                       // mov rd, rs - 23
        "^\tmov r([0-9]|1[0-9]|2[0-9]|3[0]), ([0-9_-]+)$",                                        // mov rd, L - 24
        "^\tmov \\(r([0-9]|1[0-9]|2[0-9]|3[0])\\)\\(([0-9_-]+)\\), r([0-9]|1[0-9]|2[0-9]|3[0])$", // mov rd L, rs - 25

        // floating point
        "^\taddf r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0])$", // addf rd, rs, rt - 26
        "^\tsubf r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0])$", // subf rd, rs, rt - 27
        "^\tmulf r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0])$", // mulf rd, rs, rt - 28
        "^\tdivf r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0])$", // divf rd, rs, rt - 29

        // input & output
        "^\tin r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0])$",  // in rd, rs - 30
        "^\tout r([0-9]|1[0-9]|2[0-9]|3[0]), r([0-9]|1[0-9]|2[0-9]|3[0])$", // out rd, rs - 31
        "^\thalt$",

        // useful macros!
        "^\tclr r([0-9]|1[0-9]|2[0-9]|3[0])$", // clr rd == ... - 32
        "^\tld r([0-9]|1[0-9]|2[0-9]|3[0]), ([0-9]+)$",
        "^\tpush r([0-9]|1[0-9]|2[0-9]|3[0])$",                  // push rd == ... - 34
        "^\tpop r([0-9]|1[0-9]|2[0-9]|3[0])$",                   // pop rd == ... - 35
        "^\tld r([0-9]|1[0-9]|2[0-9]|3[0]), :([a-zA-Z0-9_-]+)$", // ld rd, :label // ld rd, L == ... - 33
    };

    for (int i = 0; i < 37; i++)
    {
        if (regcomp(&regexArray[i], opcodePatterns[i], REG_EXTENDED) != 0)
        {
            fprintf(stderr, "Error on this line\n");
            fclose(output);
            removeFile(outputFile);
            return 1;
        }
    }

    for (int i = 0; buffer[i] != '\0'; i++)
    {
        if (buffer[i] == '\n')
        {
            numberOfLines++;
        }
    }
    printf("lines: %d\n", numberOfLines);

    char *splitter[numberOfLines];
    splitter[0] = strtok(buffer, "\n");
    for (int i = 1; i < numberOfLines; i++)
    {
        splitter[i] = strtok(NULL, "\n");
        if (splitter[i] == NULL)
        {
            fprintf(stderr, "Error: Failed to tokenize line %d\n", i);
        }
        else
        {
            printf("%s\n", splitter[i]);
        }
    }

    int j = 0;
    int numberOfColons = 0;
    while (j < numberOfLines)
    {
        if (splitter[j][0] == ':')
        {
            numberOfColons++;
        }
        j++;
    }
    printf("Number of colons: %d\n", numberOfColons);

    int numberOfBytes = 0;
    struct label labelArray[numberOfColons];
    int index = 0;
    bool codeToggle;
    bool dataToggle;
    bool codeFollow = false;
    char *line;
    for (int i = 0; i < numberOfLines; i++)
    {
        if (splitter[i][0] == ':')
        {
            struct label labels;
            char *nameOfLabel = (char *)malloc(strlen(splitter[i]));
            strcpy(nameOfLabel, splitter[i] + 1);
            labels.name = nameOfLabel;
            labels.address = numberOfBytes;
            labels.lineNumber = i + 1;
            for (int j = 0; j < index; j++)
            {
                if (strcmp(labelArray[j].name, nameOfLabel) == 0)
                {
                    fprintf(stderr, "Same label '%s' defined twice\n", nameOfLabel);
                    return 1;
                }
            }
            labelArray[index] = labels;
            index++;
            printf("label name: %s\n", nameOfLabel);
            printf("byte count: %d\n", numberOfBytes);
            continue;
        }

        for (int i = 0; i < numberOfLines; i++)
        {
            if (strstr(splitter[i], "ld") != NULL)
            {
                char *labelName = strchr(splitter[i], ':');
                if (labelName != NULL)
                {
                    labelName++;
                    if (!isLabelDefined(labelName, labelArray, numberOfColons))
                    {
                        fprintf(stderr, "Error: Label '%s' used in instruction but not defined.\n", labelName);
                        fclose(output);
                        removeFile(outputFile);
                        return 1;
                    }
                }
            }
        }

        if (splitter[i][0] == ';')
        {
            continue;
        }
        else if (strncmp(splitter[i], ".code", 5) == 0)
        {
            codeToggle = true;
            dataToggle = false;
            codeFollow = true;
            continue;
        }
        else if (strncmp(splitter[i], ".data", 5) == 0)
        {
            codeToggle = false;
            dataToggle = true;
            continue;
        }
        else if (dataToggle)
        {
            numberOfBytes += 8;
        }
        else if (codeToggle)
        {
            if (strstr(splitter[i], "ld") != NULL)
            {
                numberOfBytes += 48;
            }
            else if (strstr(splitter[i], "push") != NULL)
            {
                numberOfBytes += 8;
            }
            else if (strstr(splitter[i], "pop") != NULL)
            {
                numberOfBytes += 8;
            }
            if (strstr(splitter[i], "ld"))
            {
                numberOfBytes += 48;
            }
            else if (strstr(splitter[i], "push"))
            {
                numberOfBytes += 8;
            }
            else if (strstr(splitter[i], "pop"))
            {
                numberOfBytes += 8;
            }
            else if (strstr(splitter[i], "pop"))
            {
                numberOfBytes += 4;
            }
            else if (strstr(splitter[0], "addi"))
            {
                numberOfBytes += 4;
            }
            else if (strstr(splitter[0], "sub"))
            {
                numberOfBytes += 4;
            }
            else if (strstr(splitter[0], "subi"))
            {
                numberOfBytes += 4;
            }
            else if (strstr(splitter[0], "mul"))
            {
                numberOfBytes += 4;
            }
            else if (strstr(splitter[0], "div"))
            {
                numberOfBytes += 4;
            }
            else if (strstr(splitter[0], "and"))
            {
                numberOfBytes += 4;
            }
            else if (strstr(splitter[0], "or"))
            {
                numberOfBytes += 4;
            }
            else if (strstr(splitter[0], "xor"))
            {
                numberOfBytes += 4;
            }
            else if (strstr(splitter[0], "not"))
            {
                numberOfBytes += 4;
            }
            else if (strstr(splitter[0], "shftr"))
            {
                numberOfBytes += 4;
            }
            else if (strstr(splitter[0], "shftri"))
            {
                numberOfBytes += 4;
            }
            else if (strstr(splitter[0], "shftl"))
            {
                numberOfBytes += 4;
            }
            else if (strstr(splitter[0], "shftli"))
            {
                numberOfBytes += 4;
            }
            else if (strstr(splitter[0], "br"))
            {
                numberOfBytes += 4;
            }
            else if (strstr(splitter[0], "brr"))
            {
                numberOfBytes += 4;
            }
            else if (strstr(splitter[0], "brnz"))
            {
                numberOfBytes += 4;
            }
            else if (strstr(splitter[0], "call"))
            {
                numberOfBytes += 4;
            }
            else if (strstr(splitter[0], "return"))
            {
                numberOfBytes += 4;
            }
            else if (strstr(splitter[0], "brgt"))
            {
                numberOfBytes += 4;
            }
            else if (strstr(splitter[0], "halt"))
            {
                numberOfBytes += 4;
            }
            else if (strstr(splitter[0], "mov"))
            {
                numberOfBytes += 4;
            }
            else if (strstr(splitter[0], "addf"))
            {
                numberOfBytes += 4;
            }
            else if (strstr(splitter[0], "subf"))
            {
                numberOfBytes += 4;
            }
            else if (strstr(splitter[0], "mulf"))
            {
                numberOfBytes += 4;
            }
            else if (strstr(splitter[0], "divf"))
            {
                numberOfBytes += 4;
            }
            else if (strstr(splitter[0], "in"))
            {
                numberOfBytes += 4;
            }
            else if (strstr(splitter[0], "out"))
            {
                numberOfBytes += 4;
            }
            else
            {
                numberOfBytes += 4;
            }
        }
    }

    if (!codeFollow)
    {
        return handleMissingCodeSection(output, outputFile);
    }

    for (int i = 0; i < numberOfColons; i++)
    {
        for (int j = 0; j < strlen(labelArray[i].name); j++)
        {
            char *labelArrayName = labelArray[i].name;
            if (labelArrayName[j] == ' ')
            {
                fprintf(stderr, "Error on line %d\n", labelArray[i].lineNumber);
                fclose(output);
                removeFile(outputFile);
                return 1;
            }
        }
    }

    for (int i = 0; i < numberOfLines; i++)
    {
        if (splitter[i][0] == ':')
        {
            continue;
        }
        if (splitter[i][0] == ';')
        {
            continue;
        }
        else if (strncmp(splitter[i], ".code", 5) == 0)
        {
            codeToggle = true;
            dataToggle = false;
            continue;
        }
        else if (strncmp(splitter[i], ".data", 5) == 0)
        {
            codeToggle = false;
            dataToggle = true;
            continue;
        }
        else if (dataToggle)
        {
            uint64_t labelValue;
            if (sscanf(splitter[i], "\t%lu", &labelValue) == 1)
            {
                printf("the value is: %lu\n", labelValue);
                // fwrite((const void *)&labelValue, sizeof(labelValue), 1, output);
            }
        }
        else if (codeToggle)
        {
            int labelValue2 = regexCheck(splitter[i], regexArray, 37);
            printf("label value: %d \n", labelValue2);

            if (labelValue2 != -1)
            {
                struct data instruction;
                if (labelValue2 == 0)
                {
                    if (sscanf(splitter[i], "\tadd r%d, r%d, r%d", &instruction.rd, &instruction.rs, &instruction.rt) == 3)
                    {
                        printf("we have an add!\n");
                        instruction.opcode = 0x0;
                        instruction.L = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }
                else if (labelValue2 == 1)
                {
                    if (sscanf(splitter[i], "\taddi r%d, %hd", &instruction.rd, &instruction.L) == 2)
                    {
                        if (instruction.L > 4095 || instruction.L < 0)
                        {
                            fprintf(stderr, "Error on line %d\n", i + 1);
                            fclose(output);
                            removeFile(outputFile);
                            return 1;
                        }
                        printf("we have an addi!\n");
                        instruction.opcode = 0x1;
                        instruction.rs = 0;
                        instruction.rt = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }

                else if (labelValue2 == 2)
                {
                    if (sscanf(splitter[i], "\tsub r%d, r%d, r%d", &instruction.rd, &instruction.rs, &instruction.rt) == 3)
                    {
                        printf("we have a sub!\n");
                        instruction.opcode = 0x2;
                        instruction.L = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }
                else if (labelValue2 == 3)
                {
                    if (sscanf(splitter[i], "\tsubi r%d, %hd", &instruction.rd, &instruction.L) == 2)
                    {
                        if (instruction.L > 4095 || instruction.L < 0)
                        {
                            fprintf(stderr, "Error on line %d\n", i + 1);
                            fclose(output);
                            removeFile(outputFile);
                            return 1;
                        }
                        printf("we have a subi!\n");
                        instruction.opcode = 0x3;
                        instruction.rs = 0;
                        instruction.rt = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }

                else if (labelValue2 == 4)
                {
                    if (sscanf(splitter[i], "\tmul r%d, r%d, r%d", &instruction.rd, &instruction.rs, &instruction.rt) == 3)
                    {
                        printf("we have a mul!\n");
                        instruction.opcode = 0x4;
                        instruction.L = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }
                else if (labelValue2 == 5)
                {
                    if (sscanf(splitter[i], "\tdiv r%d, r%d, r%d", &instruction.rd, &instruction.rs, &instruction.rt) == 3)
                    {
                        printf("we have a div!\n");
                        instruction.opcode = 0x5;
                        instruction.L = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }
                else if (labelValue2 == 6)
                {
                    if (sscanf(splitter[i], "\tand r%d, r%d, r%d", &instruction.rd, &instruction.rs, &instruction.rt) == 3)
                    {
                        printf("we have an and!\n");
                        instruction.opcode = 0x6;
                        instruction.L = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }
                else if (labelValue2 == 7)
                {
                    if (sscanf(splitter[i], "\tor r%d, r%d, r%d", &instruction.rd, &instruction.rs, &instruction.rt) == 3)
                    {
                        printf("we have an or!\n");
                        instruction.opcode = 0x7;
                        instruction.L = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }
                else if (labelValue2 == 8)
                {
                    if (sscanf(splitter[i], "\txor r%d, r%d, r%d", &instruction.rd, &instruction.rs, &instruction.rt) == 3)
                    {
                        printf("we have an xor!\n");
                        instruction.opcode = 0x8;
                        instruction.L = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }
                else if (labelValue2 == 9)
                {
                    if (sscanf(splitter[i], "\tnot r%d, r%d", &instruction.rd, &instruction.rs) == 2)
                    {
                        printf("we have a not!\n");
                        instruction.opcode = 0x9;
                        instruction.rt = 0;
                        instruction.L = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }
                else if (labelValue2 == 10)
                {
                    if (sscanf(splitter[i], "\tshftr r%d, r%d, r%d", &instruction.rd, &instruction.rs, &instruction.rt) == 3)
                    {
                        printf("we have a shftr!\n");
                        instruction.opcode = 0xa;
                        instruction.L = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }
                else if (labelValue2 == 11)
                {
                    if (sscanf(splitter[i], "\tshftri r%d, %hd", &instruction.rd, &instruction.L) == 2)
                    {
                        if (instruction.L > 4095 || instruction.L < 0)
                        {
                            fprintf(stderr, "Error on line %d\n", i + 1);
                            fclose(output);
                            removeFile(outputFile);
                            return 1;
                        }
                        printf("we have a shftri!\n");
                        instruction.opcode = 0xb;
                        instruction.rs = 0;
                        instruction.rt = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }
                else if (labelValue2 == 12)
                {

                    if (sscanf(splitter[i], "\tshftl r%d, r%d, r%d", &instruction.rd, &instruction.rs, &instruction.rt) == 3)
                    {
                        printf("we have a shftl!\n");
                        instruction.opcode = 0xc;
                        instruction.L = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }
                else if (labelValue2 == 13)
                {
                    if (sscanf(splitter[i], "\tshftli r%d, %hd", &instruction.rd, &instruction.L) == 2)
                    {
                        if (instruction.L > 4095 || instruction.L < 0)
                        {
                            fprintf(stderr, "Error on line %d\n", i + 1);
                            fclose(output);
                            removeFile(outputFile);
                            return 1;
                        }
                        printf("we have a shftli!\n");
                        instruction.opcode = 0xd;
                        instruction.rs = 0;
                        instruction.rt = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }
                else if (labelValue2 == 14)
                {
                    if (sscanf(splitter[i], "\tbr r%d", &instruction.rd) == 1)
                    {
                        printf("we have a br!\n");
                        instruction.opcode = 0x0e;
                        instruction.rt = 0;
                        instruction.rs = 0;
                        instruction.L = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }
                else if (labelValue2 == 15)
                {
                    if (sscanf(splitter[i], "\tbrr r%d", &instruction.rd) == 1)
                    {
                        printf("we have a brr for rd!\n");
                        instruction.opcode = 0x0f;
                        instruction.rt = 0;
                        instruction.rs = 0;
                        instruction.L = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }
                else if (labelValue2 == 16)
                {
                    if (sscanf(splitter[i], "\tbrr %hd", &instruction.L) == 1)
                    {
                        if (instruction.L > 2047 || instruction.L < -2048)
                        {
                            fprintf(stderr, "Error on line %d\n", i + 1);
                            fclose(output);
                            removeFile(outputFile);
                            return 1;
                        }
                        if (instruction.L < 0)
                        {
                            short mask = 0XFFF;
                            instruction.L = instruction.L & mask;
                        }
                        printf("we have a brr for L!\n");
                        instruction.opcode = 0x10;
                        instruction.rt = 0;
                        instruction.rs = 0;
                        instruction.rd = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }
                else if (labelValue2 == 17)
                {
                    if (sscanf(splitter[i], "\tbrnz r%d, r%d", &instruction.rd, &instruction.rs) == 2)
                    {
                        printf("we have a brnz!\n");
                        instruction.opcode = 0x11;
                        instruction.L = 0;
                        instruction.rt = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }
                else if (labelValue2 == 18)
                {
                    if (sscanf(splitter[i], "\tcall r%d", &instruction.rd) == 1)
                    {
                        printf("we have a call!\n");
                        instruction.opcode = 0x12;
                        instruction.L = 0;
                        instruction.rt = 0;
                        instruction.rs = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }
                else if (labelValue2 == 19)
                {
                    if (strcmp(splitter[i], "\treturn") == 0)
                    {
                        printf("we have a found!\n");
                        instruction.opcode = 0x13;
                        instruction.rd = 0;
                        instruction.rs = 0;
                        instruction.rt = 0;
                        instruction.L = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }
                else if (labelValue2 == 20)
                {
                    if (sscanf(splitter[i], "\tbrgt r%d, r%d, r%d", &instruction.rd, &instruction.rs, &instruction.rt) == 3)
                    {
                        printf("we found a brgt!\n");
                        instruction.opcode = 0x14;
                        instruction.L = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }

                else if (labelValue2 == 31)
                {
                    if (strcmp(splitter[i], "\thalt") == 0)
                    {
                        printf("we found a halt!\n");
                        instruction.opcode = 0x1f;
                        instruction.rd = 0;
                        instruction.rs = 0;
                        instruction.rt = 0;
                        instruction.L = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }

                else if (labelValue2 == 21)
                {
                    if (sscanf(splitter[i], "\tmov r%d, (r%d)(%hd)", &instruction.rd, &instruction.rs, &instruction.L) == 3)
                    {
                        if (instruction.L > 2047 || instruction.L < -2048)
                        {
                            fprintf(stderr, "Error on line %d\n", i + 1);
                            fclose(output);
                            removeFile(outputFile);
                            return 1;
                        }
                        if (instruction.L < 0)
                        {
                            short mask = 0XFFF;
                            instruction.L = instruction.L & mask;
                        }
                        printf("we found a move1! \n");
                        instruction.opcode = 0x15;
                        instruction.rt = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }
                else if (labelValue2 == 22)
                {
                    if (sscanf(splitter[i], "\tmov r%d, r%d", &instruction.rd, &instruction.rs) == 2)
                    {
                        printf("we found a mov2! \n");
                        instruction.opcode = 0x16;
                        instruction.rt = 0;
                        instruction.L = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }
                else if (labelValue2 == 23)
                {
                    if (sscanf(splitter[i], "\tmov r%d, %hd", &instruction.rd, &instruction.L) == 2)
                    {
                        if (instruction.L > 4095 || instruction.L < 0)
                        {
                            fprintf(stderr, "Error on line %d\n", i + 1);
                            fclose(output);
                            removeFile(outputFile);
                            return 1;
                        }
                        printf("mov3 is found!\n");
                        instruction.opcode = 0x17;
                        instruction.rt = 0;
                        instruction.rs = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }
                else if (labelValue2 == 24)
                {
                    if (sscanf(splitter[i], "\tmov (r%d)(%hd), r%d", &instruction.rd, &instruction.L, &instruction.rs) == 3)
                    {
                        if (instruction.L > 2047 || instruction.L < -2048)
                        {
                            fprintf(stderr, "Error on line %d\n", i + 1);
                            fclose(output);
                            removeFile(outputFile);
                            return 1;
                        }
                        if (instruction.L < 0)
                        {
                            short mask = 0XFFF;
                            instruction.L = instruction.L & mask;
                        }
                        printf("we found a mov4!\n");
                        instruction.opcode = 0x18;
                        instruction.rt = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }
                else if (labelValue2 == 25)
                {
                    if (sscanf(splitter[i], "\taddf r%d, r%d, r%d", &instruction.rd, &instruction.rs, &instruction.rt) == 3)
                    {
                        printf("we found an addf! \n");
                        instruction.opcode = 0x19;
                        instruction.L = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }
                else if (labelValue2 == 26)
                {
                    if (sscanf(splitter[i], "\tsubf r%d, r%d, r%d", &instruction.rd, &instruction.rs, &instruction.rt) == 3)
                    {
                        printf("we found a subf! \n");
                        instruction.opcode = 0x1a;
                        instruction.L = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }
                else if (labelValue2 == 27)
                {
                    if (sscanf(splitter[i], "\tmulf r%d, r%d, r%d", &instruction.rd, &instruction.rs, &instruction.rt) == 3)
                    {
                        printf("we found a mulf! \n");
                        instruction.opcode = 0x1b;
                        instruction.L = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }
                else if (labelValue2 == 28)
                {
                    if (sscanf(splitter[i], "\tdivf r%d, r%d, r%d", &instruction.rd, &instruction.rs, &instruction.rt) == 3)
                    {
                        printf("we found a divf! \n");
                        instruction.opcode = 0x1c;
                        instruction.L = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }
                else if (labelValue2 == 29)
                {
                    if (sscanf(splitter[i], "\tin r%d, r%d", &instruction.rd, &instruction.rs) == 2)
                    {
                        printf("we found an in!\n");
                        instruction.opcode = 0x1d;
                        instruction.rt = 0;
                        instruction.L = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }
                else if (labelValue2 == 30)
                {
                    if (sscanf(splitter[i], "\tout r%d, r%d", &instruction.rd, &instruction.rs) == 2)
                    {
                        printf("we found an out!\n");
                        instruction.opcode = 0x1e;
                        instruction.rt = 0;
                        instruction.L = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }

                else if (labelValue2 == 32)
                {
                    if (sscanf(splitter[i], "\tclr r%d", &instruction.rd) == 1)
                    {
                        printf("we found a clr!\n");
                        instruction.opcode = 0x8;
                        instruction.rs = instruction.rd;
                        instruction.rt = instruction.rd;
                        instruction.L = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }
                else if (labelValue2 == 33)
                {
                    printf("we found a load!\n");
                    char max[100];
                    if (sscanf(splitter[i], "\tld r%d, %s", &instruction.rd, max) == 2)
                    {

                        printf("the max value is %s\n", max);
                        if (strlen(max) == 20)
                        {
                            int lastDigit = max[strlen(max) - 1] - '0';
                            printf("last digit: %d\n", lastDigit);

                            if (lastDigit > 5)
                            {
                                fprintf(stderr, "Error on line %d\n", i + 1);
                                fclose(output);
                                removeFile(outputFile);
                                return 1;
                            }
                        }
                    }
                    uint64_t L;
                    if (sscanf(splitter[i], "\tld r%d, %lu", &instruction.rd, &L) == 2)
                    {
                        if (L <= 4294967295)
                        {
                            printf("found a clear! \n");
                            int shiftAmounts[] = {52, 40, 28, 16, 4};

                            for (int i = 0; i < 5; ++i)
                            {
                                instruction.opcode = 0x1;
                                instruction.rs = 0;
                                instruction.rt = 0;

                                uint64_t bits12 = (L >> shiftAmounts[i]) & 0XFFF;
                                instruction.L = bits12;

                                instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);

                                instruction.opcode = 0xd;
                                instruction.L = (i == 4) * 4 + (i != 4) * 12;

                                instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                            }
                            instruction.opcode = 0x1;
                            instruction.L = L & 0XF;
                            instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                        }
                    }
                    else
                    {
                        fprintf(stderr, "Error on line %d\n", i + 1);
                        fclose(output);
                        removeFile(outputFile);
                        return 1;
                    }
                }
                else if (labelValue2 == 34)
                {
                    printf("we found a push\n");
                    if (sscanf(splitter[i], "\tpush r%d", &instruction.rs) == 1)
                    {
                        instruction.opcode = 0x18;
                        instruction.rd = 31;
                        instruction.rt = 0;
                        instruction.L = -8;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                        instruction.opcode = 0x3;
                        instruction.rd = 31;
                        instruction.rs = 0;
                        instruction.rt = 0;
                        instruction.L = 8;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }
                else if (labelValue2 == 35)
                {
                    printf("found a pop!\n");
                    if (sscanf(splitter[i], "\tpop r%d", &instruction.rd) == 1)
                    {
                        instruction.opcode = 0x15;
                        instruction.rs = 31;
                        instruction.rt = 0;
                        instruction.L = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                        instruction.opcode = 0x1;
                        instruction.rd = 31;
                        instruction.rs = 0;
                        instruction.rt = 0;
                        instruction.L = 8;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);
                    }
                }
                else if (labelValue2 == 36)
                {
                    char lab[256];
                    uint64_t L2;
                    if (sscanf(splitter[i], "\tld r%d, :%s", &instruction.rd, lab) == 2)
                    {
                        for (int i = 0; i < numberOfColons; i++)
                        {
                            if (strcmp(labelArray[i].name, lab) == 0)
                            {
                                printf("there is a match found!\n");
                                L2 = labelArray[i].address;
                            }
                        }

                        instruction.opcode = 0x8;
                        instruction.L = 0;
                        instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, instruction.L);

                        for (int i = 52; i >= 4; i -= 12)
                        {
                            uint64_t value = (L2 >> i) & 0XFFF;

                            if (i == 52 || i == 16)
                            {
                                instruction.opcode = 0x1;
                            }
                            else
                            {
                                instruction.opcode = 0xd;
                            }

                            if (i == 4)
                            {
                                instruction.L = 4;
                            }
                            else
                            {
                                instruction.L = 12;
                            }
                            instructionOutput(output, instruction.opcode, instruction.rd, instruction.rs, instruction.rt, value);
                        }
                    }
                }
            }
            else if (labelValue2 == -1)
            {
                fprintf(stderr, "Error on line %d\n", i + 1);
                fclose(output);
                removeFile(outputFile);
                return 1;
            }
        }
    }
    for (int i = 0; i < 37; i++)
    {
        regfree(&regexArray[i]);
    }
}