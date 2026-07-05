// https://www.jmeiners.com/lc3-vm/

#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <ostream>

enum {
  R_R0 = 0,
  R_R1,
  R_R2,
  R_R3,
  R_R4,
  R_R5,
  R_R6,
  R_R7,
  R_PC, /* program counter */
  R_COND,
  R_COUNT
};

class Memory {
 public:
  Memory() {
    std::cout << "Initializing Memory.\n";
    Memory::PrintConfig();
  }

  static void PrintConfig() {
    std::cout << "LC3 inspired Arch" << std::endl;
    std::cout << "Word Size       : 16 bits" << std::endl;
    std::cout << "Total Locations : " << Memory::MAX << std::endl;
    std::cout << "Memory Size     : "
              << (Memory::MAX * 8 * 2 /* Word Size */) / std::pow(2, 13)
              << " KB" << std::endl;
    std::cout << "No of registers : " << R_COUNT << " each 16 bits"
              << std::endl;
  }

  uint16_t read(uint32_t loc) {
    // TODO: For now return garbage; throw an exception later
    if (loc > Memory::MAX) {
      return 67;
    }
    return memory[loc];
  }
  uint16_t write(uint32_t loc, uint16_t data) {
    // TODO: For now garbage; throw an exception later
    if (loc > Memory::MAX) {
      return 67;
    }
    return memory[loc] = data;
  }

  uint16_t reg_read(uint8_t r) { return registers[r]; }
  uint16_t reg_write(uint8_t r, uint16_t data) { return registers[r] = data; }

 private:
  static const uint32_t MAX =
      static_cast<uint32_t>(1 << 16);  // TODO: Maybe > 65536 mem locs
  std::array<uint16_t, Memory::MAX> memory{};
  std::array<uint16_t, R_COUNT> registers{};
};

class VirtualMachine {
 private:
  Memory mem{};

 public:
  VirtualMachine() { mem.reg_write(R_PC, 0x1000); }
};

int main() {
  VirtualMachine vm;

  // Instruction: 32 bits
  //  opcode    :  4  bits
  //  operands  : 28 bits
  //
  // Total No of GPRegs = 8   : no of bits = 3
  //    + 1 Program Counter   : no of bits = 1
  //
  // Addressing Modes:
  //  - Register Direct Mode:
  //  (a)- mov R1, R2
  //          (4) (4) -> 8 bits for operands
  //
  //  - Direct Mode:
  //  (b)- mov R1, [#addr]
  //          (4)    (16) -> 20 bits for operands
  //  (c)- mov [#addr], R1
  //          (16)   (4) -> 20 bits for operands
  //
  //  - Index Absolute:
  //  (d)- mov R1, R2(#offset)
  //          (4)  (4) (16)   -> 24 bits for operands
  //  (e)- mov R1(#offset), R2
  //          (4)  (16)    (4) -> 24 bits for operands
  //
  //  - Immediate Mode:
  //  (f)- mov R1, #imm
  //          (4)  (16) -> 20 bits for operands
  //  (g)- mov [R1], #imm
  //          (4)  (16) -> 20 bits for operands
  //
  //  - Base + Index:
  //  (g)- mov R1, R2(R3 + #offset)
  //          (4) (4) (4)   (16)   -> 28 bits for operands
  //  (h)- mov R2(R3 + #offset), R1
  //          (4) (4)   (16)    (4)  -> 28 bits for operands
  //
  // No. of Instructions = 1 (mov)      : no of bits = 0
  // No. of Opcodes      = 9 (2x4 + 1)  : no of bits = 3
  //
  // Opcode
  //   0000 : (a)
  //   0010 : (b)
  //   0011 : (c)
  //   0100 : (d)
  //   0101 : (e)
  //   0110 : (f)
  //   0111 : (g)
  //   1000 : (h)
  //   1001 : (i)
}
