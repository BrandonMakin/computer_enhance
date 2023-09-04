# Emulate MOV

"""
table to decode W and R/M fields into their corresponding registers
usage:
registers[w][rm] == the correct register
"""
registers = [
    ["AL", "CL", "DL", "BL", "SP", "BP", "SI", "DI"],
    ["AX", "CX", "DX", "BX", "AH", "CH", "DH", "BH"]
    ]


with open("listing_0037_single_register_mov", "rb") as file:
    while (bytes := list(file.read(2))):
        
        # first bit
        opcode = bytes[0] >> 2

        """
        if d == 0: reg field specifies instruction source (direction is from register)
        if d == 1: reg field specifies instruction destination (direction is to register)
        """
        d = bytes[0] >> 1 & 1
        
        w = bytes[0] >> 0 & 1

        # second bit

        ## mod tells us whether this is a memory operation or a register operation
        mod = bytes[1] >> 6 # top 2 bytes

        ##
        reg = bytes[1] >> 3 & 0b111 # next 3 bytes
        
        ##
        rm  = bytes[1] >> 0 & 0b111 # last 3 bytes

        print()
        match opcode:
            case 0b100010: # MOV
                match mod:
                    case 0b11: # Register Mode
                        source = registers[w][reg]  # set source to register specified by reg
                        destination = registers[w][rm] # set destination to register specified by rm
                        if (d): # if d: swap source and destination
                            source, destination = destination, source
                            print("hi")
                        
                        print(f"mov {destination}, {source}")
                
                    case _:
                        print ("unsupported memory mode")
            case _:
                print("unknown instruction")