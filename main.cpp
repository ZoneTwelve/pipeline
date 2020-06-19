#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <deque>
#include <map>
using namespace std;

int iTypeLen = 11;
int iTypeCode[11] = {4, 5, 8, 9, 12, 13, 10, 11, 15, 35, 43};
int min(int a, int b){return a>b?b:a;}
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
    if(op==0){ // R type
      type = 0; // R type = 0
      rs = bin2int(binary.substr(6, 5));
      rt = bin2int(binary.substr(11, 5));
      rd = bin2int(binary.substr(16, 5));
      shamt = bin2int(binary.substr(21, 5));
      funct = bin2int(binary.substr(26, 6));
    }else if(findAt(op, iTypeCode, iTypeLen)){ // I type
      type = 1; // I type = 1
      rs = bin2int(binary.substr(6, 5));
      rt = bin2int(binary.substr(11, 5));
      immediate = bin2int(binary.substr(16, 16));
    }else{ // J type
      type = 2; // J type = 2
      immediate = bin2int(binary.substr(6, 26));
    }
    // cout << "OP: " << op << endl;
  }
  int RS(){return rs;}
  int RT(){return rt;}
  int RD(){return rd;}
  int OP(){return op;}
  int TYPE(){return type;}
  int FUNCT(){return funct;}
  int IMMEDIATE(){return immediate;}
  int RALUS(){return readFromALU;}
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
    // I - type
    controlSignal[0b001100000000] = 0b11000; // and
    controlSignal[0b001000000000] = 0b00010; // add 
    // controlSignal[0] = 0; // or
    // controlSignal[0] = 0; // slt
    // controlSignal[0] = 0; // beq

    // R - type
    // control   [ OPOPOP| Funct]
    controlSignal[0b000000100000] = 0b10010; // add 
    controlSignal[0b000000100010] = 0b10010; // subtract
    controlSignal[0b000000100100] = 0b10010; // And
    controlSignal[0b000000100101] = 0b10001; // or
    controlSignal[0b000000101010] = 0b10111; // Set on < (set on less then)
  }
  map<int, int> controlSignal;
  int memwrite(int addr, int val){mem[addr] = val & 0xffff;}
  int memread(int addr){
    return mem[addr/0x4];
  }
  void run(){
    PC = 0x0; CC = 0x0;
    ALUop  = 0x00; 
    ALUout = 0x00; 
    offset = 0x04;
    while(CC < INSMEM.size() + 3){

      // Instruction Fetch
      if((PC+4)*8 - 1 < raw.length()){
        string bin = raw.substr( PC * 8, 32 );
        Instruction newIns( bin );
        cout << "Fetch: " << bin << endl;
        INSMEM.push_back(newIns);
      }

      CC += 0x01;
      PC += offset;
      // cout << "PC: " << PC << "\t-\t";
      for(int n = 0; n < 4 && n < CC; n++){
        offset = 0x01;
        int func = ((CC - n - 1) % 4);
        int pc = CC - func - 1;
        cout << "CC " << CC << ":\n\n";
        if(pc < INSMEM.size()){
          switch(func){
            case 0: // IF/ID
              ID(INSMEM[pc]);
            break;
            
            case 1: // ID/EX
              if(pc < INSMEM.size())
                EX(INSMEM[pc]);
            break;
            case 2: // EX/MEM
              if(pc < INSMEM.size())
                MEM(INSMEM[pc]);
            break;
            
            case 3: // MEM/WB
              if(pc < INSMEM.size())
                WB(INSMEM[pc]);
              // work done, try to remove instruction from INSMEM
            break;
          
          }
        }
      }

      cout << "=================================================================\n";
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
  int ALUout = 0x00, 
      offset = 0x04;
  vector<Instruction> trunk;
  deque<Instruction> INSMEM;
  int reg[10] = {
    0x0000, 0x0009, 0x0005, 0x0007, 0x0001,
    0x0002, 0x0003, 0x0004, 0x0005, 0x0006
  }; //register $0 ~ $9
  
  int mem[5] = {
    0x0005, 0x0009, 0x0004, 0x0008, 0x0007
  }; // memory

  void ID(Instruction ins){ // Instruction decode
    // int res = pre.RT()==cur.RS()?1:0 +  // update(rs=1, data)
    //           pre.RT()==cur.RD()?2:0;   // update(rd=2, data)
    // if(res>0)cur.update(3, res); // update readFromALU Status
    // else cur.update(3, -1);
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
    switch((ins.OP()<<6) + ins.FUNCT()){
      case 0b000000100000: // I type add
        cout << "I type add" << endl;
      break;
    }
  }
  void MEM(Instruction ins){// Memory access

  }
  void WB(Instruction ins){ // write back

  }
};
//[lw], [sw], [add], [addi], [sub], [and], [andi], [or], [slt], [beq]
//, , , , , P, B, z[data hazard], [hazard with load], [branchhazard]

int main(){
  const int FC = 1;
  char infile[5][20] = {"SampleInput.txt", "General.txt", "Datahazard.txt", "Lwhazard.txt", "Branchhazard.txt"};
  for(int n=0;n<FC;n++){
    ifstream input(infile[n]);
    string binary, raw;
    processor computer;

    while(getline(input, binary)){
      raw+=binary;
    }

    computer.loadBinary(raw);
    computer.run(); 

    // for(int i=0;i<5;i++)cout << "===\t";cout << endl;
  }
  return 0;
}
