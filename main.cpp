#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <deque>
#include <map>
#include <bitset>
using namespace std;

ofstream output;

int iTypeLen = 11;
int iTypeCode[11] = {4, 5, 8, 9, 12, 13, 10, 11, 15, 35, 43};
int min(int a, int b){return a>b?b:a;}
string sg(string a, int len){string r="";while(len-->0)r+=a;return r;}
bool findAt(int target, int array[], int len){
  for(int i=0;i<len;i++)if(array[i]==target)return true;
  return false;
}
int str2int(string s){
  stringstream ss;
  int i;
  ss << s;
  ss >> i;
  ss.clear();
  return i;
}
string int2str(int s){
  stringstream ss;
  string i;
  ss << s;
  ss >> i;
  ss.clear();
  return i;
}
int bin2int(string s, int a, int b){
  int r = 0;
  for(int i=a;i<b;i++){
    r*=2;
    r+=(s[i]=='1'?1:0);
  }
  return r;
}
int bin2int(string s){
  int r = 0;
  for(int i=0;i<s.length();i++){
    r*=2;
    r+=(s[i]=='1'?1:0);
  }
  return r;
}
class Instruction{ // IF, Instruction Fetch
public:
  string raw;
  Instruction(string binary){
    // cout << binary << endl;
    raw = binary;
    op = bin2int(binary.substr(0, 6));
    if(op==0)
      type = 0; // R
    else
      type = 1; // I
    // if(op==0){ // R type
      // type = 0; // R type = 0
      rs = bin2int(binary.substr(6, 5));
      rt = bin2int(binary.substr(11, 5));
      rd = bin2int(binary.substr(16, 5));
      shamt = bin2int(binary.substr(21, 5));
      funct = bin2int(binary.substr(26, 6));
    // }else if(findAt(op, iTypeCode, iTypeLen)){ // I type
      // type = 1; // I type = 1
      // rs = bin2int(binary.substr(6, 5));
      // rt = bin2int(binary.substr(11, 5));
      immediate = bin2int(binary.substr(16, 16));
    // }else{ // J type
      // type = 2; // J type = 2
      // immediate = bin2int(binary.substr(6, 26));
    // }
    // cout << "OP: " << op << endl;
  }

  // read private variabel zone
  int RS(){return rs;}
  int RT(){return rt;}
  int RD(){return rd;}
  int OP(){return op;}
  int TYPE(){return type;}
  int FUNCT(){return funct;}
  int IMMEDIATE(){return immediate;}
  int RALUS(){return readFromALU;}
  // read private variabel zone

  // dont need
  int update(int t, int data){
    int fin = 1;
    switch(t){
      case 0: rt = data;break;
      case 1: rs = data;break;
      case 2: rd = data;break;
      case 3: readFromALU = data;break;
      default: 
        fin = 0;
    }
    return fin; // if fin==1 information is update successful
  }
  int OPTYPE(){
    return type;
  }
  int PC = 0;
private:
  int op = 0,
      rs = 0, rt = 0, rd = 0,
      shamt = 0, funct = 0,
      immediate, type = -1;
  int readFromALU = -1;
  // int schedule = 0; i dont need that anymore
};
string names[4] = {"ID", "EX", "ME", "WB"};
class processor{
public:
  processor(){
    //R type             |functi      CtrExecMemWb
    controlSignal[0b000000000000] = 0b000000000000; // add
    controlSignal[0b000000100000] = 0b010110000010; // add
    controlSignal[0b000000100010] = 0b110110000010; // sub
    controlSignal[0b000000100100] = 0b000110000010; // and
    controlSignal[0b000000100101] = 0b001110000010; // or
    controlSignal[0b000000101010] = 0b111110000010; // slt
    //I type        operan|           CtrExecMemWb
    controlSignal[0b001000000000] = 0b010000100010; // addi
    controlSignal[0b001100000000] = 0b000011100010; // andi
    controlSignal[0b100001000000] = 0b010000101011; // lw
    controlSignal[0b100001000000] = 0b010000100100; // sw
    controlSignal[0b100001000000] = 0b110001010000; // beq
  }
  map<int, int> controlSignal;
  void memwrite(int addr, int val){mem[addr] = val & 0xffff;}
  int memread(int addr){
    return mem[addr/0x4];
  }
  void run(){
    int binlen = raw.length();
    PC = 0x0; CC = 0x0;
    ALUop  = 0x00; 
    ALUout = 0x00; 
    offset = 0x00;

    while(CC < INSMEM.size() + 4){//CC < INSMEM.size() + 3
      // Instruction Fetch
      if((PC+4)*8 - 1 < binlen){ // Simulate real processor reading instruction
        string bin = raw.substr( PC * 8, 32 );
        Instruction newIns( bin );
        INSMEM.push_back(newIns);
      }

      CC += 0x01; // next Clock cycle
      PC += 0x04; // default PC + 4
      for(int n = 0; n < 4 && n < CC; n++){ // loop for runing ID, EX, MEM, WB
        int func = ((CC - n - 1) % 4); // Read Stage
        int pc = CC - func - 1;        // read ins from INSMEM
        if(pc < INSMEM.size()){
          switch(func){
            case 0: // IF/ID
              // cout << "IFID: " << INSMEM.size() << ", " << pc << endl;
              ID(INSMEM[pc]);
            break;
            
            case 1: // ID/EX
              // cout << "IDEX: " << INSMEM.size() << ", " << pc << endl;
              if(pc < INSMEM.size())
                EX(INSMEM[pc]);
            break;
            case 2: // EX/MEM
              // cout << "EXMEM: " << INSMEM.size() << ", " << pc << endl;
              if(pc < INSMEM.size())
                MEM(INSMEM[pc]);
            break;
            
            case 3: // MEM/WB
              // cout << "MEMWB: " << INSMEM.size() << ", " << pc << endl;
              if(pc < INSMEM.size())
                WB(INSMEM[pc]);
              // feature" work done, try to remove instruction from INSMEM
            break;
          
          }
        }
      }
      PC += offset;

      output << "CC " << CC << ":\n\n";
      output << "Registers:\n";
      for(int i=0;i<10;i++)output << "$" << i << ": " << reg[i] << endl;
      output << endl;

      output << "Data memory:\n";
      for(int i=0;i<5;i++)output << "$" << i << ": " << mem[i] << endl;
      output << endl;

      output << "IF/ID :\n";
      output << "PC\t\t" << PC << endl;
      output << "Instruction\t" << ( (CC-1==0)?( INSMEM[CC - 1].raw ):sg("0", 32) ) << endl;
      output << endl;

      // int op = (ins.OP() << 6 ), func = ins.FUNCT() & (op==0?0b111111:0b000000);
      // int ctr = controlSignal[op+func];
      Instruction EXins(sg("0", 32));
      if(CC-2==0){
        EXins = INSMEM[CC - 2];
      }
      // Instruction EXins = ( (CC-2==0)?( INSMEM[CC - 2] ):emptyIns );
      int EXsignal = (EXins.OP() << 6 ) + EXins.FUNCT() & (EXins.OP()==0?0b111111:0b000000);
      int EXctr = controlSignal[EXsignal] & 0b000111111111;
      output << "ID/EX :\n"
            << "ReadData1\t" << ( (CC-2==0)?( reg[INSMEM[CC - 2].RS()] ):0 ) << "\n"
            << "ReadData2\t" << ( (CC-2==0)?( reg[INSMEM[CC - 2].RT()] ):0 ) << "\n"
            << "sign_ext\t" << INSMEM[CC - 2].IMMEDIATE() << "\n"
            << "Rs\t" << ( (CC-2==0)?( INSMEM[CC - 2].RS() ):0 ) << "\n"
            << "Rt\t" << ( (CC-2==0)?( INSMEM[CC - 2].RT() ):0 ) << "\n"
            << "Rd\t" << ( (CC-2==0)?( INSMEM[CC - 2].RD() ):0 ) << "\n"
            << "Control signals " <<  std::bitset<9>(EXctr)//sg("0", 9)
            << endl;
      output << endl;


      Instruction MEMins(sg("0", 32));// = ( (CC-3==0)?( INSMEM[CC - 3] ):emptyIns );
      if(CC-3==0){
        EXins = INSMEM[CC - 3];
      }
      int MEMsignal = (MEMins.OP() << 6 ) + MEMins.FUNCT() & (MEMins.OP()==0?0b111111:0b000000);
      int MEMctr = controlSignal[MEMsignal] & 0b000000011111;
      output << "EX/MEM :\n"
            << "ALUout\t" << ALUout << "\n"
            << "WriteData\t" << "" << "\n"
            << "Rt/Rd\t" << "" << "\n"
            << "Control signals " << sg("0", 5)
            << endl;
      output << endl;


      Instruction WBins(sg("0", 32));// = ( (CC-4==0)?( INSMEM[CC - 4] ):emptyIns );
      if(CC-4==0){
        WBins = INSMEM[CC - 4];
      }
      int WBsignal = (WBins.OP() << 6 ) + WBins.FUNCT() & (WBins.OP()==0?0b111111:0b000000);
      int WBctr = controlSignal[WBsignal] & 0b000000000011;
      output << "MEM/WB :\n"
           << "ReadData\t" << "" << "\n"
           << "ALUout\t" << "" << "\n"
           << "Rt/Rd\t" << "" << "\n"
           << "Control signals " << sg("0", 2)//std::bitset<5>(WBctr)
           << endl;
      output << endl;

      output << "=================================================================\n";
    }
  }
  void loadBinary(string bin){
    raw = bin;
  }
  void addIntruction(Instruction ins){
    // INSMEM.push_back(ins);
    trunk.push_back(ins);
    if(INSMEM.size()==0)
      INSMEM.push_back(ins);
  }
private:
  string raw;
  int PC = 0x0, CC = 0x0;
  int ALUop  = 0x00;
  int ALUout = 0x00, dst = 0x00,
      offset = 0x04;
  
  int WriteData = 0x00,
      EXMEMRtRd = 0x00,
      WBRtRd    = 0x00;
  vector<Instruction> trunk;
  vector<Instruction> INSMEM;
  int reg[10] = {
    0x0000, 0x0009, 0x0005, 0x0007, 0x0001,
    0x0002, 0x0003, 0x0004, 0x0005, 0x0006
  }; //register $0 ~ $9
  
  int mem[5] = {
    0x0005, 0x0009, 0x0004, 0x0008, 0x0007
  }; // memory

  void ID(Instruction ins){ // Instruction decode
    // if( branch )
      // offset = branch offset
    
    offset = 0x00;
  }

  int OFFSET(Instruction ins){
    // OPTYPE = 0 => R TYPE, = 1 => I TYPE, = 2 => J TYPE
    int op = ins.OP();
    if(op==0b000100||op==0b000101){
      return ins.IMMEDIATE();
    }
    return 0;
  }
  void EX(Instruction ins){ // execute
    int op = (ins.OP() << 6 ), func = ins.FUNCT() & (op==0?0b111111:0b000000);
    int ctr = controlSignal[op+func];
    int ALUctr   = (ctr >> 9);
    int EACstage = (ctr & 0b111100000) >> 5;
    int MAstage  = (ctr & 0b11100) >> 2;
    int RegDst = EACstage & 0b1000 >> 3,
        ALUop1 = EACstage & 0b0100 >> 2,
        ALUop0 = EACstage & 0b0010 >> 1,
        ALUsrc = EACstage & 0b0001;
    // int MAstage  = (ctr & 0xfff00) >> 2;
    // int WBstage  = (ctr & 0xff);
    int d1 = reg[ins.RS()], d2 = (ALUctr==0?reg[ins.RT()]:ins.IMMEDIATE());
    switch(ALUctr){
      case 0b000: ALUout = d1 & d2; break; // and
      case 0b010: ALUout = d1 + d2; break; // add
      case 0b110: ALUout = d1 - d2; break; // sub
      case 0b001: ALUout = d1 | d2; break; // or
      case 0b111: ALUout = d1 < d2 ? 1 : 0; break; // Set on less then
    }

    if(( MAstage & 0b100 ) >> 2 == 1)
      offset += ins.IMMEDIATE() * 4;
  }
  void MEM(Instruction ins){// Memory access
    int op = (ins.OP() << 6 ), func = ins.FUNCT() & (op==0?0b111111:0b000000);
    int ctr = controlSignal[op+func];
    int MAstage  = (ctr & 0b1100) >> 2;
    if(MAstage & 0b10 >> 1){ // mem read
      dst = mem[ALUout];
    }
    if(MAstage & 0b01){ // mem write
      mem[ins.RT()] = ALUout;
    }
  }
  void WB(Instruction ins){ // write back
    int op = (ins.OP() << 6 ), func = ins.FUNCT() & (op==0?0b111111:0b000000);
    int ctr = controlSignal[op+func];
    int WBstage  = (ctr & 0b11);
    int MAstage  = (ctr & 0b1100) >> 2;
    if(MAstage & 0b10 >> 1){ // mem read
      reg[ins.RT()] = dst;
    }
  }
};
//[lw], [sw], [add], [addi], [sub], [and], [andi], [or], [slt], [beq]
//, , , , , P, B, z[data hazard], [hazard with load], [branchhazard]
Instruction emptyIns(sg("0", 32));
int main(){
  const int FC = 5;
  char infile[5][20] = {"SampleInput.txt", "General.txt", "Datahazard.txt", "Lwhazard.txt", "Branchhazard.txt"};
  char outfile[5][50] = {"MySampleOutput.txt", "GeneralOutput.txt", "DatahazardOutput.txt", "Lwhazard.txt", "BranchhazardOutput.txt"};
  for(int n=0;n<FC;n++){
    ifstream input(infile[n]);
    string binary, raw;
    processor computer;
    // myfile << "Writing this to a file.\n";

    while(getline(input, binary)){
      raw+=binary;
    }

    computer.loadBinary(raw);

    output.open(outfile[n]);
    computer.run(); 
    output.close();

    // for(int i=0;i<5;i++)cout << "===\t";cout << endl;
  }
  return 0;
}
