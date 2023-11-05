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

with open("listing_0040_challenge_movs", "rb") as file:
    def next(): return int.from_bytes(file.read(1))
    def next_16(): return int.from_bytes(file.read(2), 'little')

    print("bits 16\n")

    while (byte := next()):

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
            
            mod = byte_2 >> 6 # MOD (mode) tells us whether this is a memory operation or a register operation
            reg = byte_2 >> 3 & 0b111 # REG (register)
            rm  = byte_2 >> 0 & 0b111 # R/M (register/memory)

            source = registers[w][reg]  # set source to register specified by reg

            if mod == 0b11: # register mode
                destination = registers[w][rm] # set destination to register specified by rm
            else: # memory mode
                address = ""
                if mod == 0b00 and rm == 0b110: # direct address
                    address = str(next_16())
                else:
                    # effective address calculation
                    match rm:
                        case 0:
                            address = "bx + si"
                        case 1:
                            address = "bx + di"
                        case 2:
                            address = "bp + si"
                        case 3:
                            address = "bp + di"
                        case 4:
                            address = "si"
                        case 5:
                            address = "di"
                        case 6:
                            address = "bp"
                        case 7:
                            address = "bx"
                    
                    if mod != 0b00:
                        if mod == 0b01:
                            offset = next()
                        if mod == 0b10:
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
                case 0b00:
                    if rm == 0b110: # direct address. 16-bit displacement to follow.
                        address = str(next_16())
                case 0b01:
                    offset = next()
                case 0b10:
                    offset = next_16()
                case 0b11:
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

        else:
            print("unexpected data (unknown instruction, or an instruction was longer than expected); exiting")
            exit(1)