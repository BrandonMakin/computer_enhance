#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int8_t i8;
typedef int16_t i16;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint64_t u64;

const char *filename = "listing_0041_add_sub_cmp_jnz";

// table to decode w and r/m fields into their corresponding registers
// usage:
// registers[w][rm] == the correct register
const char *registers[2][8] = {
    {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"},
    {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"}
};

const char *rm_effective_address_calculation[8] = {"bx + si", "bx + di", "bp + si", "bp + di", "si", "di", "bp", "bx"};

FILE *file;

typedef enum Mode
{
    MEMORY_NO_DISPLACEMENT_OR_DIRECT_ADDRESS    = 0b00,
    MEMORY_8_BIT_DISPLACEMENT                   = 0b01,
    MEMORY_16_BIT_DISPLACEMET                   = 0b10,
    REGISTER                                    = 0b11,
} Mode;

u8 mod_from(u8 byte)
{
    return byte >> 6;
}

u8 reg_from(u8 byte)
{
    return byte >> 3 & 0b111;
}

u8 rm_from(u8 byte)
{
    return byte >> 0 & 0b111;
}

const char* operation_from(u8 byte)
{
        switch (reg_from(byte))
        {
            case 0b000:
            {
                return "add";
                break;
            }
            case 0b101:
            {
                return "sub";
                break;
            }
            case 0b111:
            {
                return "cmp";
                break;
            }
            default:
            {
                return "UNKNOWN_OP";
                break;
            }
        }
}

u8 next_8()
{
    u8 result;
    if (fread(&result, 1, 1, file) == 0)
    {
        puts("(next_8) Uh oh, there are less bytes than expected.");
        exit(1);
    }
    
    return result;
}

i8 next_8_signed()
{
    i8 result;
    if (fread(&result, 1, 1, file) == 0)
    {
        puts("(next_8) Uh oh, there are less bytes than expected.");
        exit(1);
    }
    
    return result;
}

u16 next_16()
{
    u16 result;
    if (fread(&result, 2, 1, file) == 0)
    {
        puts("(next_16) Uh oh, there are less bytes than expected.");
        exit(1);
    }
    
    return result;
}

void rm_to_text(char *text, u8 mod, u8 rm, u8 w)
{
    // memset(arr, 0, sizeof arr);
    const char *eac = rm_effective_address_calculation[rm];
    switch (mod)
    {
        case MEMORY_NO_DISPLACEMENT_OR_DIRECT_ADDRESS:
        {
            if (rm == 0b110) // direct address. 16-bit displacement to follow.
                sprintf(text, "[%i]", next_16());
            else
                sprintf(text, "[%s]", eac);
            break;
        }
        case MEMORY_8_BIT_DISPLACEMENT:
        {
            i8 offset = next_8_signed();
            if (offset == 0)
                sprintf(text, "[%s]", eac);
            else if (offset < 0)
                sprintf(text, "[%s - %i]", eac, -offset);
            else
                sprintf(text, "[%s + %i]", eac, offset);
            break;
        }
        case MEMORY_16_BIT_DISPLACEMET:
        {
            u16 offset = next_16();
            if (offset != 0)
                sprintf(text, "[%s + %i]", eac, offset);
            break;
        }
        case REGISTER:
        {
            strcpy(text, registers[w][rm]);  
            break;
        }
    }
}

void decompile(u8 byte)
{
    // MOV (register/memory to/from register)
    if (0b100010 == byte >> 2)
    {
        // if d == 0: reg field specifies instruction source (direction is from register)
        // if d == 1: reg field specifies instruction destination (direction is to register)
        u8 d = byte >> 1 & 1;

        // if w == 0: move a byte
        // if w == 1: to move a word (2 bytes)
        u8 w = byte & 1;

        u8 byte_2 = next_8();
        
        u8 mod = mod_from(byte_2);
        u8 reg = reg_from(byte_2);
        u8 rm = rm_from(byte_2);

        const char *reg_text = registers[w][reg];
        char rm_text[32];
        rm_to_text(rm_text, mod, rm, w);

        if (d)
            printf("mov %s, %s\n", reg_text, rm_text);
        else
            printf("mov %s, %s\n", rm_text, reg_text);
    }

    // MOV: immediate to register
    else if (0b1011 == byte >> 4)
    {
        u8 w = byte >> 3 & 1;
        u8 reg = byte & 0b111;
        u16 immediate_value = next_8(); // least significant byte
        if (w)
            immediate_value += next_8() * 0x100; // most significant byte (only used for 2-byte registers)
        printf("mov %s, %i\n", registers[w][reg], immediate_value);
    }

    // MOV: immediate to register/memory
    else if (0b1100011 == byte >> 1)
    {
        u8 w = byte & 1;
        u8 byte_2 = next_8();
        u8 mod = byte_2 >> 6;
        u8 rm = byte_2 & 0b111;

        char destination[32];
        rm_to_text(destination, mod, rm, w);
        
        u16 offset = 0;

        if (w)
            printf("mov %s, word %i\n", destination, next_16());
        else
            printf("mov %s, byte %i\n", destination, next_8());
    }

    // MOV: memory to accumulator / accumulator to memory
    else if (0b101000 == byte >> 2)
    {
        u8 to_memory = byte >> 1 & 1;
        u8 w = byte & 1;

        u16 memory_address = w ? next_16() : next_8();
        if (to_memory) // accumulator to memory
            printf("mov [%i], ax\n", memory_address);
        else // memory to accumulator
            printf("mov ax, [%i]\n", memory_address);
    }

    // ADD/SUB/CMP: reg/memory and register to either
    else if ((byte >> 2 | 0b001110) == 0b001110)
    {
        const char *operation = operation_from(byte);
        u8 d = byte >> 1 & 1;
        u8 w = byte & 1;
        u8 byte_2 = next_8();
        u8 mod = mod_from(byte_2);
        u8 reg = reg_from(byte_2);
        u8 rm = rm_from(byte_2);

        const char *reg_text = registers[w][reg];
        char rm_text[32];
        rm_to_text(rm_text, mod, rm, w);

        if (d)
            printf("%s %s, %s\n", operation, reg_text, rm_text);
        else
            printf("%s %s, %s\n", operation, rm_text, reg_text);
    }

    // ADD/SUB/CMP: immediate to register/memory
    else if (0b100000 == (byte >> 2)) 
    {
        // if s == 0: No sign extension.
        // if s == 1: Sign extend 8-bit immediate data to 16 bits if W == 1
        u8 s = byte >> 1 & 1;

        u8 w = byte & 1;
        u8 byte_2 = next_8();
        u8 mod = mod_from(byte_2);
        u8 rm = rm_from(byte_2);
        const char *operation = operation_from(byte_2);

        char rm_text[32];
        rm_to_text(rm_text, mod, rm, w);
        u16 immediate_value = (s || !w) ? next_8() : next_16();

        if (mod == REGISTER)
            printf("%s %s, %i\n", operation, rm_text, immediate_value);
        else
        {
            char* size = w ? "word" : "byte";
            printf("%s %s %s, %i\n", operation, size, rm_text, immediate_value);
        }
    }

    // ADD/SUB/CMP: immediate to accumulator
    else if (byte >> 6 == 0 && (byte >> 1 & 0b11) == 0b10)
    {
        const char* operation = operation_from(byte);
        u8 w = byte & 1;
        u16 immediate_value = w ? next_16() : next_8();
        const char *register_name = w ? "ax" : "al";
        printf("%s %s, %i\n", operation, register_name, immediate_value);
    }
    
    // Jumps and loops
    else if (byte == 0b01110100) //JE/JZ = Jump on equal/zero
        printf("JE/JZ ; %i\n", next_8_signed());
    else if (byte == 0b01111100) //JL/JNGE = Jump on less/not greater or equal
        printf("JL/JNGE ; %i\n", next_8_signed());
    else if (byte == 0b01111110) //JLE/JNG = Jump on less or equal/not greater
        printf("JLE/JNG ; %i\n", next_8_signed());
    else if (byte == 0b01110010) //JB/JNAE = Jump on below/not above or equal
        printf("JB/JNAE ; %i\n", next_8_signed());
    else if (byte == 0b01110110) //JBE/JNA = Jump on below or equal/not above
        printf("JBE/JNA ; %i\n", next_8_signed());
    else if (byte == 0b01111010) //JP/JPE = Jump on parity/parity even
        printf("JP/JPE ; %i\n", next_8_signed());
    else if (byte == 0b01110000) //JO = Jump on overflow
        printf("JO ; %i\n", next_8_signed());
    else if (byte == 0b01111000) //JS = Jump on sign
        printf("JS ; %i\n", next_8_signed());
    else if (byte == 0b01110101) //JNE/JNZ = Jump on not equal/not zero
        printf("JNE/JNZ ; %i\n", next_8_signed());
    else if (byte == 0b01111101) //JNL/JGE = Jump on not less/greater or equal
        printf("JNL/JGE ; %i\n", next_8_signed());
    else if (byte == 0b01111111) //JNLE/JG = Jump on not less or equal/greater
        printf("JNLE/JG ; %i\n", next_8_signed());
    else if (byte == 0b01110011) //JNB/JAE = Jump on not below/above or equal
        printf("JNB/JAE ; %i\n", next_8_signed());
    else if (byte == 0b01110111) //JNBE/JA = Jump on not below or equal/above
        printf("JNBE/JA ; %i\n", next_8_signed());
    else if (byte == 0b01111011) //JNP/JPO = Jump on not parity/parity odd
        printf("JNP/JPO ; %i\n", next_8_signed());
    else if (byte == 0b01110001) //JNO = Jump on not overflow
        printf("JNO ; %i\n", next_8_signed());
    else if (byte == 0b01111001) //JNS = Jump on not sign
        printf("JNS ; %i\n", next_8_signed());
    else if (byte == 0b11100010) //LOOP = Loop CX times
        printf("LOOP ; %i\n", next_8_signed());
    else if (byte == 0b11100001) //LOOPZ/LOOPE = Loop while zero/equal
        printf("LOOPZ/LOOPE ; %i\n", next_8_signed());
    else if (byte == 0b11100000) //LOOPNZ/LOOPNE = Loop while not zero/equal
        printf("LOOPNZ/LOOPNE ; %i\n", next_8_signed());
    else if (byte == 0b11100011) //JCXZ = Jump on CX zero
        printf("JCXZ ; %i\n", next_8_signed());

    else
    {
        puts("unexpected data (unknown instruction, or an instruction was longer than expected); exiting.");
        exit(1);
    }
}

int main(void)
{
    puts("bits 16\n");

    file = fopen(filename, "rb");
    if (file == NULL)
    {
        puts("bye");
        perror("Couldn't open the file.");
        return 1;
    }

    u8 byte;
    while (fread(&byte, 1, 1, file) != 0)
    {
        // printf("%x ",byte);
        decompile(byte);
    }
    printf("\n");

    fclose(file);
    

}