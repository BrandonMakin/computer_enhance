#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int8_t i8;
typedef int16_t i16;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint64_t u64;

char *filename = "listing_0039_more_movs";

// table to decode w and r/m fields into their corresponding registers
// usage:
// registers[w][rm] == the correct register
char *registers[2][8] = {
    {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"},
    {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"}
};

char *rm_effective_address_calculation[8] = {"bx + si", "bx + di", "bp + si", "bp + di", "si", "di", "bp", "bx"};

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

FILE *file;

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
    char *eac = rm_effective_address_calculation[rm];
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
                sprintf(text, "[%s - %i]", eac, offset);
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

        // second bit
        u8 byte_2 = next_8();
        
        // mod, reg, rm = get_mod_reg_rm(byte_2)
        u8 mod = mod_from(byte_2);
        u8 reg = reg_from(byte_2);
        u8 rm = rm_from(byte_2);

        char *reg_text = registers[w][reg];
        char rm_text[32];
        rm_to_text(rm_text, mod, rm, w);

        if (d)
            printf("mov %s, %s\n", reg_text, rm_text);
        else
            printf("mov %s, %s\n", rm_text, reg_text);
    }

    // MOV (immediate to register)
    else if (0b1011 == byte >> 4)
    {
        u8 w = byte >> 3 & 1;
        u8 reg = byte & 0b111;
        u16 immediate_value = next_8(); // least significant byte
        if (w)
            immediate_value += next_8() * 0x100; // most significant byte (only used for 2-byte registers)
        printf("mov %s, %i\n", registers[w][reg], immediate_value);
    }

    // MOV (immediate to register/memory)
    else if (0b1100011 == byte >> 1)
    {
        puts("MOV (immediate to register/memory) is not implemented yet.");
        // u8 w = byte & 1
        // u8 byte_2 = next_8()
        // u8 mod = byte_2 >> 6
        // u8 rm = byte_2 & 0b111

        // address = rm_effective_address_calculation[rm]
        // offset = 0 // won't work if there is a defined offset equal to 0, but can that ever happen?

        // match mod:
        //     case Mode.MEMORY_NO_DISPLACEMENT_OR_DIRECT_ADDRESS:
        //         if rm == 0b110: # direct address. 16-bit displacement to follow.
        //             address = str(next_16())
        //     case Mode.MEMORY_8_BIT_DISPLACEMENT:
        //         offset = next()
        //     case Mode.MEMORY_16_BIT_DISPLACEMET:
        //         offset = next_16()
        //     case Mode.REGISTER:
        //         print("If this text appears, IDK why MOV (immediate to register/memory) is being called in register mode, but apparently it is.")
        //         print("Or I'm really misunderstanding something.")
        //         print("Because it seems like the assembly should just assemble to MOV (immediate to register) instead.")
        //         print("Let's bail before things break any more.")
        //         exit(1)
        
        // immediate_value = ""
        // if w:
        //     immediate_value = f"word {next_16()}"
        // else:
        //     immediate_value = f"byte {next()}"

        // if (offset != 0):
        //     address = address + f" + {offset}"
        
        // print(f"mov [{address}], {immediate_value}")
    }

    else
    {
        puts("unexpected data (unknown instruction, or an instruction was longer than expected); exiting.");
        exit(1);
    }
}

int main(void)
{
    puts("bits 16");

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