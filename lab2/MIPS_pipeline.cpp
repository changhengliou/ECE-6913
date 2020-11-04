#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>

using namespace std;
#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

#define __LOCAL_ENV__
#ifdef __LOCAL_ENV__
const string cwd = "/Users/line/Desktop/nyu/Computer architecture ECE-GY 6913/lab2/Testbenches/Testbench1/";
#endif

struct IFStruct {
  bitset<32> PC;
  bool nop;
};

struct IDStruct {
  bitset<32> Instr;
  bool nop;
};

struct EXStruct {
  bitset<32> Read_data1;
  bitset<32> Read_data2;
  bitset<16> Imm;
  bitset<5> Rs;
  bitset<5> Rt;
  bitset<5> Wrt_reg_addr;
  bool is_I_type;
  bool rd_mem;
  bool wrt_mem;
  bool alu_op;     //1 for addu, lw, sw, 0 for subu
  bool wrt_enable;
  bool nop;
};

struct MEMStruct {
  bitset<32> ALUresult;
  bitset<32> Store_data;
  bitset<5> Rs;
  bitset<5> Rt;
  bitset<5> Wrt_reg_addr;
  bool rd_mem;
  bool wrt_mem;
  bool wrt_enable;
  bool nop;
};

struct WBStruct {
  bitset<32> Wrt_data;
  bitset<5> Rs;
  bitset<5> Rt;
  bitset<5> Wrt_reg_addr;
  bool wrt_enable;
  bool nop;
};

struct stateStruct {
  IFStruct IF;
  IDStruct ID;
  EXStruct EX;
  MEMStruct MEM;
  WBStruct WB;
};

class RF {
public:
  bitset<32> Reg_data;

  RF() {
    Registers.resize(32);
    Registers[0] = bitset<32>(0);
  }

  bitset<32> readRF(bitset<5> Reg_addr) {
    Reg_data = Registers[Reg_addr.to_ulong()];
    return Reg_data;
  }

  void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data) {
    Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
  }

  void outputRF() {
    ofstream rfout;
    rfout.open("RFresult.txt", std::ios_base::app);
    if (rfout.is_open()) {
      rfout << "State of RF:\t" << endl;
      for (int j = 0; j < 32; j++) {
        rfout << Registers[j] << endl;
      }
    } else cout << "Unable to open file";
    rfout.close();
  }

private:
  vector <bitset<32>> Registers;
};

class INSMem {
public:
  bitset<32> Instruction;

  INSMem() {
#ifdef __LOCAL_ENV__
    string path = cwd + "imem.txt";
#else
    string path = "imem.txt";
#endif
    IMem.resize(MemSize);
    ifstream imem;
    string line;
    int i = 0;
    imem.open(path);
    if (imem.is_open()) {
      while (getline(imem, line)) {
        IMem[i] = bitset<8>(line);
        i++;
      }
    } else cout << "Unable to open file";
    imem.close();
  }

  bitset<32> readInstr(bitset<32> ReadAddress) {
    string insmem;
    insmem.append(IMem[ReadAddress.to_ulong()].to_string());
    insmem.append(IMem[ReadAddress.to_ulong() + 1].to_string());
    insmem.append(IMem[ReadAddress.to_ulong() + 2].to_string());
    insmem.append(IMem[ReadAddress.to_ulong() + 3].to_string());
    Instruction = bitset<32>(insmem);    //read instruction memory
    return Instruction;
  }

private:
  vector <bitset<8>> IMem;
};

class DataMem {
public:
  bitset<32> ReadData;

  DataMem() {
#ifdef __LOCAL_ENV__
    string path = cwd + "dmem.txt";
#else
    string path = "dmem.txt";
#endif
    DMem.resize(MemSize);
    ifstream dmem;
    string line;
    int i = 0;
    dmem.open(path);
    if (dmem.is_open()) {
      while (getline(dmem, line)) {
        DMem[i] = bitset<8>(line);
        i++;
      }
    } else cout << "Unable to open file";
    dmem.close();
  }

  bitset<32> readDataMem(bitset<32> Address) {
    string datamem;
    datamem.append(DMem[Address.to_ulong()].to_string());
    datamem.append(DMem[Address.to_ulong() + 1].to_string());
    datamem.append(DMem[Address.to_ulong() + 2].to_string());
    datamem.append(DMem[Address.to_ulong() + 3].to_string());
    ReadData = bitset<32>(datamem);    //read data memory
    return ReadData;
  }

  void writeDataMem(bitset<32> Address, bitset<32> WriteData) {
    DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0, 8));
    DMem[Address.to_ulong() + 1] = bitset<8>(WriteData.to_string().substr(8, 8));
    DMem[Address.to_ulong() + 2] = bitset<8>(WriteData.to_string().substr(16, 8));
    DMem[Address.to_ulong() + 3] = bitset<8>(WriteData.to_string().substr(24, 8));
  }

  void outputDataMem() {
    ofstream dmemout;
    dmemout.open("dmemresult.txt");
    if (dmemout.is_open()) {
      for (int j = 0; j < 1000; j++) {
        dmemout << DMem[j] << endl;
      }

    } else cout << "Unable to open file";
    dmemout.close();
  }

private:
  vector <bitset<8>> DMem;
};

void printState(stateStruct state, int cycle) {
  ofstream printstate;
  printstate.open("stateresult.txt", std::ios_base::app);
  if (printstate.is_open()) {
    printstate << "State after executing cycle:\t" << cycle << endl;

    printstate << "IF.PC:\t" << state.IF.PC.to_ulong() << endl;
    printstate << "IF.nop:\t" << state.IF.nop << endl;

    printstate << "ID.Instr:\t" << state.ID.Instr << endl;
    printstate << "ID.nop:\t" << state.ID.nop << endl;

    printstate << "EX.Read_data1:\t" << state.EX.Read_data1 << endl;
    printstate << "EX.Read_data2:\t" << state.EX.Read_data2 << endl;
    printstate << "EX.Imm:\t" << state.EX.Imm << endl;
    printstate << "EX.Rs:\t" << state.EX.Rs << endl;
    printstate << "EX.Rt:\t" << state.EX.Rt << endl;
    printstate << "EX.Wrt_reg_addr:\t" << state.EX.Wrt_reg_addr << endl;
    printstate << "EX.is_I_type:\t" << state.EX.is_I_type << endl;
    printstate << "EX.rd_mem:\t" << state.EX.rd_mem << endl;
    printstate << "EX.wrt_mem:\t" << state.EX.wrt_mem << endl;
    printstate << "EX.alu_op:\t" << state.EX.alu_op << endl;
    printstate << "EX.wrt_enable:\t" << state.EX.wrt_enable << endl;
    printstate << "EX.nop:\t" << state.EX.nop << endl;

    printstate << "MEM.ALUresult:\t" << state.MEM.ALUresult << endl;
    printstate << "MEM.Store_data:\t" << state.MEM.Store_data << endl;
    printstate << "MEM.Rs:\t" << state.MEM.Rs << endl;
    printstate << "MEM.Rt:\t" << state.MEM.Rt << endl;
    printstate << "MEM.Wrt_reg_addr:\t" << state.MEM.Wrt_reg_addr << endl;
    printstate << "MEM.rd_mem:\t" << state.MEM.rd_mem << endl;
    printstate << "MEM.wrt_mem:\t" << state.MEM.wrt_mem << endl;
    printstate << "MEM.wrt_enable:\t" << state.MEM.wrt_enable << endl;
    printstate << "MEM.nop:\t" << state.MEM.nop << endl;

    printstate << "WB.Wrt_data:\t" << state.WB.Wrt_data << endl;
    printstate << "WB.Rs:\t" << state.WB.Rs << endl;
    printstate << "WB.Rt:\t" << state.WB.Rt << endl;
    printstate << "WB.Wrt_reg_addr:\t" << state.WB.Wrt_reg_addr << endl;
    printstate << "WB.wrt_enable:\t" << state.WB.wrt_enable << endl;
    printstate << "WB.nop:\t" << state.WB.nop << endl;
  } else cout << "Unable to open file";
  printstate.close();
}

void stats(stateStruct& state) {
  const auto& id = state.ID;
  const auto& ex = state.EX;
  const auto& mem = state.MEM;
  const auto& wb = state.WB;

  cout << "------------------------------" << endl;
  cout << "[IF] => PC: " << state.IF.PC.to_ulong() << ", nop: " << state.IF.nop << endl;
  cout << "[ID] => Instr: " << id.Instr << ", nop: " << id.nop << endl;
  cout << "[EX] => Rs: " << ex.Rs.to_ulong() << ", Rt: " << ex.Rt.to_ulong() <<
       ", Wrt_reg: " << ex.Wrt_reg_addr.to_ulong() <<
       ", Read1: " << ex.Read_data1.to_ulong() << ", Read2: " << ex.Read_data2.to_ulong() <<
       ", nop: " << ex.nop << endl;
  cout << "[MEM] => Rs: " << mem.Rs.to_ulong() << ", Rt: " << mem.Rt.to_ulong() <<
       ", Wrt_reg: " << mem.Wrt_reg_addr.to_ulong() << ", nop: " << mem.nop << endl;
  cout << "[WB] => Rs: " << wb.Rs.to_ulong() << ", Rt: " << wb.Rt.to_ulong() <<
       ", Wrt_reg: " << wb.Wrt_reg_addr.to_ulong() << ", nop: " << wb.nop << endl;
}

long to_long(bitset<32>& x) {
  return (int32_t) x.to_ulong();
}
// R-type opcode(6) rs(5) rt(5) rd(5) shamt(5) funct(6)
// I-type opcode(6) rs(5) rt(5) immediate(16)
//
//        type   opcode    funct
// addu    R       00        21    $rd = $rs + $rt
// subu    R       00        23    $rd = $rs - $rt
// lw      I       23              $rt = mem[$rs + imme]
// sw      I       2b              mem[$rs + imme] = $rt
// beq     I       04              if $rs != $rt pc + 4 + imme; else pc + 4
// halt            ff
int main() {
  RF myRF;
  INSMem myInsMem;
  DataMem myDataMem;

  stateStruct state;
  int cycle = 0;
  bool isHalt = false;
  bool stall = false;

  while (true) {
    stateStruct newState = state;
    auto& id = state.ID;
    auto& ex = state.EX;
    auto& wb = state.WB;
    auto& mem = state.MEM;
    bool nop = false;
    long jmpPC = -1;

    const auto instr = id.Instr.to_ulong();
    const auto opCode = (instr & 0xfc000000) >> 26;
    const auto rs = (instr & 0x03e00000) >> 21;
    const auto rt = (instr & 0x001f0000) >> 16;
    const auto rd = (instr & 0x0000f800) >> 11;
    const auto funct = instr & 0x0000003f;
    const auto imme = instr & 0x0000ffff;
    const auto signExtendImme = (int32_t) (id.Instr[15] == 1 ? (imme | 0xffff0000) : imme);
    const auto isRType = opCode == 0;
    const auto isIType = !isRType;
    const auto isBranch = opCode == 0x04;

    // LOAD-ADD hazard, stall here (notice that we don't have to stall when there is LOAD-STORE dependency)
    if (mem.rd_mem && !ex.wrt_mem && (ex.Rs == mem.Wrt_reg_addr.to_ulong() || ex.Rt == mem.Wrt_reg_addr.to_ulong())) {
      cout << "------------------------------" << endl;
      cout << "LOAD-ADD hazard STALL" << endl;
      stall = true;
    } else if (isBranch && (ex.Wrt_reg_addr == rs || ex.Wrt_reg_addr == rt)) {
      cout << "------------------------------" << endl;
      cout << "Control hazard STALL" << endl;
      stall = true;
    } else {
      stall = false;
    }
    /* --------------------- WB stage --------------------- */
    bool wb_nop = cycle < 3 || (mem.Rs == 31 && mem.Rt == 31 && mem.Wrt_reg_addr == 31);
    if (mem.wrt_mem && !wb_nop) {
      myDataMem.writeDataMem(mem.ALUresult, wb.Wrt_reg_addr == mem.Rt ? wb.Wrt_data : mem.Store_data);
    }
    const auto writeBackData = mem.rd_mem ? myDataMem.readDataMem(mem.ALUresult) : mem.ALUresult;
    newState.WB.wrt_enable = !wb_nop && mem.wrt_enable;
    newState.WB.Wrt_reg_addr = mem.Wrt_reg_addr;
    newState.WB.Rs = mem.Rs;
    newState.WB.Rt = mem.Rt;
    newState.WB.Wrt_data = writeBackData;
    newState.WB.nop = wb_nop;

    /* --------------------- MEM stage --------------------- */
    if (stall) {
      newState.MEM = MEMStruct();
      newState.MEM.nop = true;
    } else {
      nop = cycle < 2 || (ex.Rs == 31 && ex.Rt == 31 && ex.Wrt_reg_addr == 31);
      unsigned long data1;
      unsigned long data2;
      if (mem.Wrt_reg_addr == ex.Rs) {
        // ex-mem forwarding
        data1 = to_long(mem.ALUresult);
      } else if (wb.Wrt_reg_addr == ex.Rs) {
        // mem-ex forwarding
        data1 = to_long(wb.Wrt_data);
      } else {
        data1 = to_long(ex.Read_data1);
      }
      if (mem.Wrt_reg_addr == ex.Rt) {
        // ex-mem forwarding
        data2 = to_long(mem.ALUresult);
      } else if (wb.Wrt_reg_addr == ex.Rt) {
        // mem-ex forwarding
        data2 = to_long(wb.Wrt_data);
      } else {
        data2 = to_long(ex.Read_data2);
      }
      long aluResult = data1 + (ex.alu_op ? 1 : -1) * data2;
      if (ex.is_I_type) {
        aluResult = data1 + static_cast<int16_t>(ex.Imm.to_ulong());
      }
      newState.MEM.ALUresult = aluResult;
      newState.MEM.Wrt_reg_addr = ex.Wrt_reg_addr;
      newState.MEM.Store_data = ex.Read_data2;
      newState.MEM.Rs = ex.Rs;
      newState.MEM.Rt = ex.Rt;
      newState.MEM.wrt_enable = !nop && ex.wrt_enable;
      newState.MEM.rd_mem = !nop && ex.rd_mem;
      newState.MEM.wrt_mem = !nop && ex.wrt_mem;
      newState.MEM.nop = nop;
    }

    /* --------------------- EX stage --------------------- */
    nop = (rs == rt && rt == rd && rs == 0) || instr == 0xffffffff || stall;
    // write before read to prevent structure hazard
    if (wb.wrt_enable && !wb_nop) {
      myRF.writeRF(wb.Wrt_reg_addr, wb.Wrt_data);
    }
    if (!stall) {
      newState.EX.Rs = rs;
      newState.EX.Rt = rt;
      newState.EX.Wrt_reg_addr = isIType ? rt : rd;
      newState.EX.Imm = signExtendImme;
      newState.EX.Read_data1 = myRF.readRF(rs);
      newState.EX.Read_data2 = myRF.readRF(rt);
      newState.EX.alu_op = nop || funct != 0x23; // addu, lw, sw = 1, subu = 0
      newState.EX.is_I_type = isIType;
      newState.EX.rd_mem = opCode == 0x23; // lw
      newState.EX.wrt_mem = opCode == 0x2b; // sw
      newState.EX.wrt_enable = !nop && !(opCode == 0x2b || isBranch); // not sw, beq;

      // TODO: resolve branch here
      if (isBranch) {
        auto mem_reg_wrt_addr = mem.Wrt_reg_addr.to_ulong();
        // forward from the EX stage in the below case
        // we need to detect if branch is taken or not at the ID stage
        // but at the ID stage, $2 from the first addu is still at the
        // EX stage. So we forward it to the ID stage for comparing.
        // IF ID EX MEM WB
        //    IF ID EX  MEM WB
        //       IF ID  EX  MEM WB
        // addu $2, $1, $2
        // addu $1, $0, $1
        // beq $2, $3, -3
        long data1 = rs == mem_reg_wrt_addr ? to_long(mem.ALUresult) : to_long(newState.EX.Read_data1);
        long data2 = rt == mem_reg_wrt_addr ? to_long(mem.ALUresult) : to_long(newState.EX.Read_data2);
        if (data1 != data2) {
          // TODO: flush all and jump back
          newState.EX = EXStruct();
          nop = true;
          cout << "------------------------------" << endl;
          cout << "FLUSH ALL: " << data1 << " != " << data2 <<
               ", jump to " << (to_long(state.IF.PC) + signExtendImme * 4) << endl;
          jmpPC = to_long(state.IF.PC) + signExtendImme * 4;
        }
      }
    } else {
      newState.EX.Read_data1 = myRF.readRF(ex.Rs);
      newState.EX.Read_data2 = myRF.readRF(ex.Rt);
    }
    newState.EX.nop = nop;

    /* --------------------- ID stage --------------------- */
    if (!isHalt && myInsMem.Instruction.to_ulong() == 0xffffffff)
      isHalt = true;
    nop = stall || isHalt || jmpPC >= 0;
    if (jmpPC >= 0) {
      newState.ID.Instr = myInsMem.readInstr(jmpPC);
    } else if (!nop) {
      newState.ID.Instr = myInsMem.readInstr(state.IF.PC);
    }
    newState.ID.nop = nop;

    /* --------------------- IF stage --------------------- */
    nop = stall || isHalt;
    newState.IF.nop = nop;
    auto currPC = jmpPC >= 0 ? jmpPC : state.IF.PC.to_ulong();
    if (!nop) {
      newState.IF.PC = currPC + (nop ? 0 : 4);
    }

    stats(newState);

    if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
      break;

    printState(newState, cycle++); //print states after executing cycle 0, cycle 1, cycle 2 ...

    state = newState; /*The end of the cycle and updates the current state with the values calculated in this cycle */

  }

  myRF.outputRF(); // dump RF;
  myDataMem.outputDataMem(); // dump data mem

  return 0;
}