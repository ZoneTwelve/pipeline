#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
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
  Instruction(string binary){
    cout << binary << endl;
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
    cout << "OP: " << op << endl;
  }
  int RS(){return rs;}
  int RT(){return rt;}
  int RD(){return rd;}
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
private:
  int op = 0,
      rs = 0, rt = 0, rd = 0,
      shamt = 0, funct = 0,
      immediate, type = -1;
  int readFromALU = -1;
  // int schedule = 0; i dont need that anymore
  string raw;
};
string names[4] = {"ID", "EX", "ME", "WB"};
class processor{
public:
  processor(){}
  int memwrite(int addr, int val){mem[addr] = val & 0xffff;}
  int memread(int addr){
    return mem[addr/0x4];
  }
  void run(){
    int data;
    if(INS.size()==0){
      // cout << "No instruction to run";
      return;
    }

    cout << "Processor is runing..." << endl;
    for(int i=0;i<5;i++)cout << "---\t";cout << endl;
    while(PC < INS.size() + 3){
      PC += 0x01;
      cout << "PC: " << PC << "\t-\t";
      for(int n = 0; n < 4 && n < PC; n++){
        int func = ((PC - n - 1) % 4);
        int pc = PC - func - 1;
        if(pc < INS.size()){
          cout << names[func] << " " << pc << ",\t";
          switch(func){
            case 0: // ID
              if(pc-1>0)
                ID(INS[pc], INS[pc-1]);
            break;
            
            case 1: // EX
            break;
            
            case 2: // MEM
            break;
            
            case 3: // WB
            break;
          
          }      
        }
      }
      cout << endl;
    }
    for(int i=0;i<5;i++)cout << "---\t";cout << endl;
  }
  void addIntruction(Instruction ins){
    INS.push_back(ins);
  }
private:
  vector<Instruction> INS;
  int PC = 0x0;
  int reg[10] = {
    0x0000, 0x0009, 0x0005, 0x0007, 0x0001,
    0x0002, 0x0003, 0x0004, 0x0005, 0x0006
  }; //register $0 ~ $9
  
  int mem[5] = {
    0x0005, 0x0009, 0x0004, 0x0008, 0x0007
  }; // memory

  int ID(Instruction cur, Instruction pre){ // Instruction decode
    int res = pre.RT()==cur.RS()?1:0 +  // update(rs=1, data)
              pre.RT()==cur.RD()?2:0;   // update(rd=2, data)
    if(res>0)cur.update(3, res); // update readFromALU Status
    else cur.update(3, -1);
  }
  void EX(Instruction ins){ // execute

  }
  void MEM(Instruction ins){// Memory access

  }
  void WB(Instruction ins){ // write back

  }
};
//[lw], [sw], [add], [addi], [sub], [and], [andi], [or], [slt], [beq]
//, , , , , P, B, z[data hazard], [hazard with load], [branchhazard]

int main(){
  const int FC = 5;
  char infile[FC][20] = {"SampleInput.txt", "General.txt", "Datahazard.txt", "Lwhazard.txt", "Branchhazard.txt"};
  for(int n=0;n<FC;n++){
    ifstream input(infile[n]);
    string tmp;
    processor computer;

    // for(int i=0;i<n+1;i++){
    //   Instruction ins("00000000000000000000000000000000");
    //   computer.addIntruction(ins);
    // }
    while(getline(input, tmp)){
      Instruction ins(tmp);
      computer.addIntruction(ins);
    }

    computer.run(); 

    for(int i=0;i<5;i++)cout << "===\t";cout << endl;
  }
  return 0;
}
