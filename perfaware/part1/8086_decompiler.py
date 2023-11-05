# emulate mov

'''
table to decode w and r/m fields into their corresponding registers
usage:
registers[w][rm] == the correct register
'''
registers = [
    ["al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"],
    ["ax", "cx", "dx", "bx", "sp", "bp", "si", "di"]
    ]
rm_effective_address_calculation = ["bx + si", "bx + di", "bp + si", "bp + di", "si", "di", "bp", "bx"]

def get_mod_reg_rm(byte):
    mod = byte >> 6 # MOD (mode) tells us whether this is a memory operation or a register operation
    reg = byte >> 3 & 0b111 # REG (register)
    rm  = byte >> 0 & 0b111 # R/M (register/memory)
    return mod, reg, rm

class Mode:
    MEMORY_NO_DISPLACEMENT_OR_DIRECT_ADDRESS    = 0b00
    MEMORY_8_BIT_DISPLACEMENT                   = 0b01
    MEMORY_16_BIT_DISPLACEMET                   = 0b10
    REGISTER                                    = 0b11

global file_data
global idx
idx = 0

with open("listing_0041_add_sub_cmp_jnz", "rb") as file:
    file_data = bytearray(file.read())

def next(signed = False):
    global idx
    if signed and file_data[idx] > 128:
        result = file_data[idx] - 256
    else:
        result = file_data[idx]
    idx += 1
    return result

def next_16(): 
    global idx
    result = int.from_bytes(file_data[idx:idx+2], 'little')
    idx += 2
    return result

def rm_to_text(mod, rm):
    text = rm_effective_address_calculation[rm]
    match mod:
        case Mode.MEMORY_NO_DISPLACEMENT_OR_DIRECT_ADDRESS:
            if rm == 0b110: # direct address. 16-bit displacement to follow.
                text = str(next_16())
        case Mode.MEMORY_8_BIT_DISPLACEMENT:
            offset = next(signed=True) # get next byte as a signed int8
            if offset < 0:
                text = f"{text} - {offset}"
            elif offset > 0:
                text = f"{text} + {offset}"
        case Mode.MEMORY_16_BIT_DISPLACEMET:
            offset = next_16()
            if offset != 0:
                text = f"{text} + {offset}"
        case Mode.REGISTER:
            text = registers[w][rm] # set destination to register specified by rm

    # add brackets to rm for any memory mode
    if (mod != Mode.REGISTER):
        text = "[" + text + "]"
    
    return text

print("bits 16\n")

while (idx < len(file_data)):
    byte = next()
    # MOV (register/memory to/from register)
    if 0b100010 == byte >> 2:
        '''
        if d == 0: reg field specifies instruction source (direction is from register)
        if d == 1: reg field specifies instruction destination (direction is to register)
        '''
        d = byte >> 1 & 1

        '''
        if w == 0: move a byte
        if w == 1: to move a word (2 bytes)
        '''
        w = byte & 1

        # second bit
        byte_2 = next()
        
        mod, reg, rm = get_mod_reg_rm(byte_2)

        source = registers[w][reg]  # set source to register specified by reg

        if mod == 0b11: # register mode
            destination = registers[w][rm] # set destination to register specified by rm
        else: # memory mode
            address = rm_effective_address_calculation[rm] # effective address calculation
            match mod:
                case Mode.MEMORY_NO_DISPLACEMENT_OR_DIRECT_ADDRESS:
                    if rm == 0b110: # direct address
                        address = str(next_16())
                case Mode.MEMORY_8_BIT_DISPLACEMENT:
                    sign = "+"
                    offset = next()
                    if offset > 128:
                        offset = 256 - offset
                        sign = "-"
                    if offset != 0:
                        address += f" {sign} {offset}"
                case Mode.MEMORY_16_BIT_DISPLACEMET:
                    offset = next_16()
                    if offset != 0:
                        address += f" + {offset}"
            
            destination = f"[{address}]"

        if (d): # if d: swap source and destination
            source, destination = destination, source

        print(f"mov {destination}, {source}")

    # MOV (immediate to register)
    elif 0b1011 == byte >> 4:
        w = byte >> 3 & 1
        reg = byte >> 0 & 0b111
        immediate_value = next() # least significant byte
        if w:
            immediate_value += next() * 0x100 # most significant byte (only used for 2-byte registers)
        print(f"mov {registers[w][reg]}, {immediate_value}")

    elif 0b1100011 == byte >> 1: # MOV (immediate to register/memory)
        w = byte & 1
        byte_2 = next()
        mod = byte_2 >> 6
        rm = byte_2 & 0b111

        address = rm_effective_address_calculation[rm]
        offset = 0 # won't work if there is a defined offset equal to 0, but can that ever happen?

        match mod:
            case Mode.MEMORY_NO_DISPLACEMENT_OR_DIRECT_ADDRESS:
                if rm == 0b110: # direct address. 16-bit displacement to follow.
                    address = str(next_16())
            case Mode.MEMORY_8_BIT_DISPLACEMENT:
                offset = next()
            case Mode.MEMORY_16_BIT_DISPLACEMET:
                offset = next_16()
            case Mode.REGISTER:
                print("If this text appears, IDK why MOV (immediate to register/memory) is being called in register mode, but apparently it is.")
                print("Or I'm really misunderstanding something.")
                print("Because it seems like the assembly should just assemble to MOV (immediate to register) instead.")
                print("Let's bail before things break any more.")
                exit(1)
        
        immediate_value = ""
        if w:
            immediate_value = f"word {next_16()}"
        else:
            immediate_value = f"byte {next()}"

        if (offset != 0):
            address = address + f" + {offset}"
        
        print(f"mov [{address}], {immediate_value}")

    elif 0b101000 == byte >> 2: # memory to accumulator / accumulator to memory
        to_memory = byte >> 1 & 1
        w = byte & 1

        memory_address = next_16() if w else next()
        if to_memory: # accumulator to memory
            print(f"mov [{memory_address}], ax")
        else: # memory to accumulator
            print(f"mov ax, [{memory_address}]")

    elif byte >> 2 | 0b001110 == 0b001110: # add/sub/cmp: reg/memory and register to either
        operation = ""
        match (byte & 0b00111000) >> 3:
            case 0b000:
                operation = "add"
            case 0b101:
                operation = "sub"
            case 0b111:
                operation = "cmp"
        
        d = byte >> 1 & 1 # 1 if `reg` is the destination. 0 if `reg` is the source.
        w = byte & 1

        mod, reg, rm = get_mod_reg_rm(next())

        reg_as_text = registers[w][reg]
        rm_as_text = rm_to_text(mod, rm)

        if d:
            print(f"{operation} {reg_as_text}, {rm_as_text}")
        else:
            print(f"{operation} {rm_as_text}, {reg_as_text}")
    elif 0b100000 == byte >> 2: # add/sub/cmp: immediate to register/memory
        w = byte & 1

        '''
        if s == 0: No sign extension.
        if s == 1: Sign extend 8-bit immediate data to 16 bits if W == 1
        '''
        s = byte >> 1 & 1
        
        mod, opcode, rm = get_mod_reg_rm(next())

        operation = ""
        match (opcode):
            case 0b000:
                operation = "add"
            case 0b101:
                operation = "sub"
            case 0b111:
                operation = "cmp"

        rm_as_text = rm_to_text(mod, rm)
        immediate_value = next() if (s or not w) else next_16()

        immediate_size = ""
        if mod != Mode.REGISTER:
            immediate_size = "word " if w else "byte "

        print(f"{operation} {immediate_size}{rm_as_text}, {immediate_value}")
    
    elif byte >> 6 == 0 and byte >> 1 & 0b11 == 0b10: # add/sub/cmp: immediate to accumulator
        operation = ""
        match (byte & 0b00111000) >> 3:
            case 0b000:
                operation = "add"
            case 0b101:
                operation = "sub"
            case 0b111:
                operation = "cmp"
        
        w = byte & 1
        immediate_value = next_16() if w else next()
        register = "ax" if w else "al"

        print(f"{operation} {register}, {immediate_value}")

    elif byte == 0b01110100: #JE/JZ = Jump on equal/zero
        print (f"JE/JZ ; {next(signed=True)}")
    elif byte == 0b01111100: #JL/JNGE = Jump on less/not greater or equal
        print(f"JL/JNGE ; {next(signed=True)}")
    elif byte == 0b01111110: #JLE/JNG = Jump on less or equal/not greater
        print(f"JLE/JNG ; {next(signed=True)}")
    elif byte == 0b01110010: #JB/JNAE = Jump on below/not above or equal
        print(f"JB/JNAE ; {next(signed=True)}")
    elif byte == 0b01110110: #JBE/JNA = Jump on below or equal/not above
        print(f"JBE/JNA ; {next(signed=True)}")
    elif byte == 0b01111010: #JP/JPE = Jump on parity/parity even
        print(f"JP/JPE ; {next(signed=True)}")
    elif byte == 0b01110000: #JO = Jump on overflow
        print(f"JO ; {next(signed=True)}")
    elif byte == 0b01111000: #JS = Jump on sign
        print(f"JS ; {next(signed=True)}")
    elif byte == 0b01110101: #JNE/JNZ = Jump on not equal/not zero
        print(f"JNE/JNZ ; {next(signed=True)}")
    elif byte == 0b01111101: #JNL/JGE = Jump on not less/greater or equal
        print(f"JNL/JGE ; {next(signed=True)}")
    elif byte == 0b01111111: #JNLE/JG = Jump on not less or equal/greater
        print(f"JNLE/JG ; {next(signed=True)}")
    elif byte == 0b01110011: #JNB/JAE = Jump on not below/above or equal
        print(f"JNB/JAE ; {next(signed=True)}")
    elif byte == 0b01110111: #JNBE/JA = Jump on not below or equal/above
        print(f"JNBE/JA ; {next(signed=True)}")
    elif byte == 0b01111011: #JNP/JPO = Jump on not parity/parity odd
        print(f"JNP/JPO ; {next(signed=True)}")
    elif byte == 0b01110001: #JNO = Jump on not overflow
        print(f"JNO ; {next(signed=True)}")
    else:
        print("unexpected data (unknown instruction, or an instruction was longer than expected); exiting")
        exit(1)