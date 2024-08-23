### Addressing Modes: 
1. Immediate Addressing Mode (IAM)
2. Register Direct Addressing Mode / Register Addressing Mode (RDAM / RAM)
3. Direct Addressing Mode (DAM)
4. Register Indirect Addressing Mode (RIAM)
5. Implicit / Inherent Addressing Mode (IAM)

#### IAM
The 8/16 bit data itself is provided in the operand of the instruction.
`MVI A, C3H` -> C3H is the 8 bit data provided in the instruction.

#### RDAM / RAM
The 8/16 bit data is stored in a register and the name of register is provided in the instruction.
`MOV A, B` -> data stored in register B is copied in A

#### DAM
The 8/16 bit data is stored in some memory location and the 16 bit address of the memory location is provided in the instruction.
> Data is fetched directly from the provided memory location.

`LDA FF99H`
`MOV A, FF99H` -> 8 bit data @M(FF99) is now stored in Accumulator

#### RIAM
The 8/16 bit data is stored in some memory location and the 16 bit address of the memory location is stored in a Register Pair and the name of the Register is provided in the instruction
> Data is fetched from the memory location whose address itself is stored in a register pair

`LDAX H` -> 16 bit address of memory location is fetched from HL pair and then the data from the memory location.

#### IAM
The source of data and the destination of data, both are Accumulator.
`CMA` -> Complements the data in Accumulator

## Data Transfer Instructions
1. MOV Rd, Rs
       Rd, M        {M(HL)}
       M, Rs        {M(HL)}

2. MVI Rd, data8
       M, data8     {M(HL)}

3. LXI RP, data16   {RP = B, D, H}

4. LDAX RP          {RP = B, D}

5. LDA address16    {M(address16)}

6. LHLD address16   {L = M(address16); H = M(address16 + 1)}

7. STAX RP          {RP = B, D}

8. STA address16    {M(address16)}

9. SHLD address16   {M(address16) = L; M(address16 + 1) = H}

10. XCHG            {HL <=> DE}

## Arithmetic Instructions
1. ADD R
       M            {M(HL)}

2. ADI data8

3. ADC R
       M            {M(HL)}

4. ACI data8

5. SUB R 
       M            {M(HL)}

6. SUI data8

7. SBB R 
       M            {M(HL)}

8. SBI data8

9. DAA              `[invalid BCD to valid BCD result]`

10. DAD RP          {RP = B, D; HL + RP = HL}

11. INR R
        M           {M(HL)}

12. DCR R
        M           {M(HL)}

13. INX RP          {RP = B, D, H}

14. DCX RP          {RP = B, D, H}
