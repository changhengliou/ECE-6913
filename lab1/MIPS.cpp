#include <bitset>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

#define ADDU 1
#define SUBU 3
#define AND 4
#define OR 5
#define NOR 7

// author Changheng Liou <cl5533@nyu.edu>
//#define __DEBUG__
//#define __LOCAL_ENV__

#ifdef __LOCAL_ENV__
const string cwd = "/Users/line/Desktop/nyu/Computer architecture ECE-GY 6913/lab1/";
#endif

// Memory size.
// In reality, the memory size should be 2^32, but for this lab and space
// reasons, we keep it as this large number, but the memory is still 32-bit
// addressable.
#define MemSize 65536

// Register file
class RF {
public:
  bitset<32> ReadData1, ReadData2;

  RF() {
    Registers.resize(32);
    Registers[0] = bitset<32>(0);
  }

  void ReadWrite(bitset<5> RdReg1, bitset<5> RdReg2, bitset<5> WrtReg,
                 bitset<32> WrtData, bitset<1> WrtEnable) {
    // TODO: implement!
    ReadData1 = Registers[RdReg1.to_ulong()];
    ReadData2 = Registers[RdReg2.to_ulong()];
    if (WrtEnable[0]) {
      Registers[WrtReg.to_ulong()] = WrtData;
    }
  }

  void OutputRF() {
    ofstream rfout;
#ifdef __LOCAL_ENV__
    const string file = cwd + "RFresult.txt";
#else
    const string file = "RFresult.txt";
#endif
    rfout.open(file, std::ios_base::app);
    if (rfout.is_open()) {
      rfout << "A state of RF:" << endl;
      for (int j = 0; j < 32; j++) {
        rfout << Registers[j] << endl;
      }

    } else
      cout << "Unable to open file: " << file << endl;
    rfout.close();
  }

private:
  vector<bitset<32>> Registers;
};

class ALU {
public:
  bitset<32> ALUresult;

  bitset<32> ALUOperation(bitset<3> ALUOP, bitset<32> oprand1,
                          bitset<32> oprand2) {
    // TODO: implement!
    const auto op = ALUOP.to_ulong();
    switch (op) {
      case ADDU:
        ALUresult = oprand1.to_ulong() + oprand2.to_ulong();
        break;
      case SUBU:
        ALUresult = oprand1.to_ulong() - oprand2.to_ulong();
        break;
      case AND:
        ALUresult = oprand1 & oprand2;
        break;
      case OR:
        ALUresult = oprand1 | oprand2;
        break;
      case NOR:
        ALUresult = (oprand1 | oprand2).flip();
        break;
      default:
        ALUresult = oprand2;
        break;
    }
    return ALUresult;
  }
};

// Instruction memory
class INSMem {
public:
  bitset<32> Instruction;

  INSMem() {
    IMem.resize(MemSize);
    ifstream imem;
    string line;
    int i = 0;
#ifdef __LOCAL_ENV__
    const string file = cwd + "imem.txt";
#else
    const string file = "imem.txt";
#endif
    imem.open(file);
    if (imem.is_open()) {
      while (getline(imem, line)) {
        IMem[i] = bitset<8>(line);
        i++;
      }

    } else
      cout << "Unable to open file: " << file << endl;
    imem.close();
  }

  bitset<32> ReadMemory(bitset<32> ReadAddress) {
    // TODO: implement!
    // (Read the byte at the ReadAddress and the following three byte).
    auto pc = ReadAddress.to_ulong();
    Instruction = IMem[pc].to_ulong() << 24 | IMem[pc + 1].to_ulong() << 16 |
                  IMem[pc + 2].to_ulong() << 8 | IMem[pc + 3].to_ulong();
    return Instruction;
  }

private:
  vector<bitset<8>> IMem;
};

// Data Memory is initialized using the “dmem.txt” file
// the data memory addresses also begin at 0 and increment by one in each line.
class DataMem {
public:
  bitset<32> readdata;

  DataMem() {
    DMem.resize(MemSize);
    ifstream dmem;
    string line;
#ifdef __LOCAL_ENV__
    const string file = cwd + "dmem.txt";
#else
    const string file = "dmem.txt";
#endif
    int i = 0;
    dmem.open(file);
    if (dmem.is_open()) {
      while (getline(dmem, line)) {
        DMem[i] = bitset<8>(line);
        i++;
      }
    } else
      cout << "Unable to open file: " << file << endl;
    dmem.close();
  }

  bitset<32> MemoryAccess(bitset<32> Address, bitset<32> WriteData,
                          bitset<1> readmem, bitset<1> writemem) {
    // TODO: implement!
    const auto addr = Address.to_ulong();
    if (readmem.count()) { // loadMode
      readdata = (DMem[addr].to_ulong() << 24) |
                 (DMem[addr + 1].to_ulong() << 16) |
                 (DMem[addr + 2].to_ulong() << 8) |
                 DMem[addr + 3].to_ulong();
    } else if (writemem.count()) { // writeMode
      DMem[addr] = WriteData.to_ulong() >> 24;
      DMem[addr + 1] = (WriteData.to_ulong() >> 16) & 0xff;
      DMem[addr + 2] = (WriteData.to_ulong() >> 8) & 0xff;
      DMem[addr + 3] = WriteData.to_ulong() & 0xff;
    }
    return readdata;
  }

  void OutputDataMem() {
    ofstream dmemout;
#ifdef __LOCAL_ENV__
    const string file = cwd + "dmemresult.txt";
#else
    const string file = "dmemresult.txt";
#endif
    dmemout.open(file);
    if (dmemout.is_open()) {
      for (int j = 0; j < 1000; j++) {
        dmemout << DMem[j] << endl;
      }

    } else
      cout << "Unable to open file: " << file << endl;
    dmemout.close();
  }

private:
  vector<bitset<8>> DMem;
};

int main() {
  int pc = 0x00;
  RF myRF;
  ALU myALU;
  INSMem myInsMem;
  DataMem myDataMem;

  while (pc < MemSize) {
    // Fetch
    auto instruction = myInsMem.ReadMemory(pc);
#ifdef __DEBUG__
    cout << "-----------------------------" << endl;
    cout << "Program counter: " << pc << endl;
#endif
    pc += 4;

    // If current insturciton is "11111111111111111111111111111111", then break;
    if (instruction.count() == 32) break;

    // decode(Read RF)
    // 0000 0000 0000 0000 0000 0000 0000 0000
    auto opCode = (instruction.to_ulong() & 0xfc000000) >> 26; // 31 to 26
    bool isRType = opCode == 0;
    bool isJType = opCode == 2 || opCode == 3;
    bool isIType = !isRType && !isJType;
    auto regS = (instruction.to_ulong() & 0x03e00000) >> 21; // 25 - 21
    auto regT = (instruction.to_ulong() & 0x001f0000) >> 16; // 20 - 16
    auto regD = (instruction.to_ulong() & 0x0000f800) >> 11; // 15 - 11
    auto shamt = (instruction.to_ulong() & 0x00007c00) >> 6; // 10 - 6
    auto funct = instruction.to_ulong() & 0x0000003f; // 5 - 0
    auto immediate = instruction.to_ulong() & 0x0000ffff; // 15 - 0
    auto signExtendedImmediate = (int32_t) (instruction[15] == 1 ? (immediate | 0xffff0000) : immediate);
    bool isLoad = opCode == 0x23;
    bool isStore = opCode == 0x2b;
    auto aluOp = 0;
    if (isLoad || isStore) {
      aluOp = ADDU;
    } else if (isRType) {
      aluOp = funct & 0x07; // last 3 bit of OPCODE
    } else if (isIType) {
      aluOp = opCode & 0x07;
    }
    bool isBranch = opCode == 0x04;
    bool writeEnable = !(isStore || isBranch || isJType);
#ifdef __DEBUG__
    cout << "Instruction Type: ";
    if (isRType) cout << "R";
    if (isIType) cout << "I";
    if (isJType) cout << "J";
    cout << endl;
    cout << "[31 - 26]: " << bitset<6>(opCode) << ", [25 - 21]: " << bitset<5>(regS) << ", [20 - 16]: "
         << bitset<5>(regT) << ", [15 - 11]: " << bitset<5>(regD) << endl;
    cout << "[10 - 6]: " << bitset<5>(shamt) << ", [5 - 0]: " << bitset<6>(funct) << ", [15 - 0]: "
         << bitset<16>(immediate) << endl;
#endif
    if (isJType) {
      pc = ((pc & 0xf0000000) << 28) | ((instruction.to_ulong() & 0x03ffffff) << 2) & 0xfffffffc;
      myRF.OutputRF(); // dump RF;
      continue;
    }

    // Read first
    myRF.ReadWrite(regS, regT, isIType ? regT : regD, {}, false);
#ifdef __DEBUG__
    cout << "[" << regS << "]: " << myRF.ReadData1 << endl;
    cout << "[" << regT << "]: " << myRF.ReadData2 << endl;
#endif
    int isEqual = myRF.ReadData1 == myRF.ReadData2;
    if (isIType && isEqual && isBranch) {
      pc += (signExtendedImmediate << 2);
      myRF.OutputRF(); // dump RF;
      continue;
    }

    // Execute
    auto output = myALU.ALUOperation(aluOp, myRF.ReadData1, isIType ? signExtendedImmediate : myRF.ReadData2);
#ifdef __DEBUG__
    cout << "ALU RESULT: " << output << endl;
#endif

    // Read/Write Mem
    auto loadData = myDataMem.MemoryAccess(output, myRF.ReadData2, isLoad, isStore);

    // Write back to RF
    myRF.ReadWrite(regS, regT, isIType ? regT : regD, isLoad ? loadData : output, writeEnable);

    myRF.OutputRF(); // dump RF;
  }
  myDataMem.OutputDataMem(); // dump data mem

  return 0;
}
