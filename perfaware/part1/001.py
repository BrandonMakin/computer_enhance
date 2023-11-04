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

with open("listing_0039_more_movs", "rb") as file:
    def next(): return int.from_bytes(file.read(1))

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
                    print ("mov with direct address, but I don't remember how to handle this!!")
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
                            offset = next() + (next() * 0x100)
                        
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

        else:
            print("unexpected data (unknown instruction, or an instruction was longer than expected); exiting")
            exit(1)