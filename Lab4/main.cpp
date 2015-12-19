#include <iostream>
#include <cstring>
#include <sstream>
#include <cstdio>
#include <string>
#include <vector>
#include <cctype>
#include <queue>
#include <map>
#include <fstream>
#include <iomanip>
#include <stdlib.h>
#include <string>
#include <ctype.h>
#include <unistd.h>
#include<algorithm>
#include<memory>
#include <limits>
using namespace std;
typedef struct instr{
    int id;
    int loc;
    int ts;
}instruction;
vector<instr> instructions;
vector<instr> readyq;
vector<instr> altq;
struct less_than_key
{
    inline bool operator() (const instruction& struct1, const instruction& struct2)
    {
        if(struct1.loc == struct2.loc){
            if(struct1.id < struct2.id)
                return true;
            else
                return false;
        }
        return (struct1.loc < struct2.loc);
    }
};
struct greater_than_key
{
    inline bool operator() (const instruction& struct1, const instruction& struct2)
    {
        if(struct1.loc == struct2.loc){
            if(struct1.id > struct2.id)
                return true;
            else
                return false;
        }
        return (struct1.loc < struct2.loc);
    }
};
class Scheduler{

    public:
        Scheduler(){return;}
        virtual void putInst(instruction,bool) = 0;
        virtual instruction getInst(instruction,bool) = 0;
};
class fifo:public Scheduler{
    public:
        fifo(){return;}
        void putInst(instruction i, bool seeking){
            readyq.push_back(i);
        }
        instruction getInst(instruction lastInst, bool right){
            instruction ret = readyq.front();
            readyq.erase(readyq.begin());
            return ret;
        }
};

class sstf:public Scheduler{
    public:
        sstf(){return;}
        void putInst(instruction i, bool seeking){
            readyq.push_back(i);
        }
        instruction getInst(instruction lastInst, bool right){
            int minVal =  std::numeric_limits<int>::max();
            int minIn;
            int i;
            for(i=0; i<readyq.size();i++){
                int val = abs(readyq[i].loc-lastInst.loc);
                if(val<minVal){
                    minVal =val;
                    minIn = i;
                }
            }
            instruction ret = readyq[minIn];
            readyq.erase(readyq.begin()+minIn);
            return ret;
        }
};
class scan:public Scheduler{
    public:
        scan(){return;}
        void putInst(instruction i, bool seeking){
            readyq.push_back(i);
        }
        instruction getInst(instruction lastInst, bool right){

            int minVal =  std::numeric_limits<int>::max();
            int minIn;
            int i;
            if (right){
                std::sort(readyq.begin(), readyq.end(),less_than_key());
                minIn = readyq.size() - 1;
                for(i=0; i<readyq.size();i++){
                    if(readyq[i].loc >= lastInst.loc){
                        minIn = i;
                        break;
                    }
                }
            }
            else{
                 minIn = 0;
                 std::sort(readyq.begin(), readyq.end(),greater_than_key());
                   for(i=readyq.size()-1; i>0;i--){
                        if(readyq[i].loc <= lastInst.loc){
                        minIn = i;
                        break;
                    }
                }

            }

            instruction ret = readyq[minIn];
            readyq.erase(readyq.begin()+minIn);
            return ret;
        }
};

class cscan:public Scheduler{
    public:
        cscan(){return;}
        void putInst(instruction i, bool seeking){
            readyq.push_back(i);
        }
        instruction getInst(instruction lastInst, bool right){

            int minVal =  std::numeric_limits<int>::max();
            int minIn=0;
            int i;

            std::sort(readyq.begin(), readyq.end(),less_than_key());

            for(i=0; i<readyq.size();i++){
                    if(readyq[i].loc >= lastInst.loc){
                        minIn = i;
                        break;
                    }
            }
            instruction ret = readyq[minIn];
            readyq.erase(readyq.begin()+minIn);
            return ret;
        }
};


class fscan:public Scheduler{
    public:
        fscan(){return;}
        void putInst(instruction i, bool seeking){
            if(seeking)
                    altq.push_back(i);


            else
                    readyq.push_back(i);


        }
        instruction getInst(instruction lastInst, bool right){

            int minVal =  std::numeric_limits<int>::max();
            int minIn;
            int i;
            if (right){
                std::sort(readyq.begin(), readyq.end(),less_than_key());
                minIn = readyq.size() - 1;
                for(i=0; i<readyq.size();i++){
                    if(readyq[i].loc >= lastInst.loc){
                        minIn = i;
                        break;
                    }
                }
            }
            else{
                    minIn = 0;
                    std::sort(readyq.begin(), readyq.end(),greater_than_key());
                    for(i=readyq.size()-1; i>0;i--){

                        if(readyq[i].loc <= lastInst.loc){
                            minIn = i;
                            break;
                    }
                }

            }

            instruction ret = readyq[minIn];
            readyq.erase(readyq.begin()+minIn);
            return ret;
        }
};


int main(int argc, char** argv)
{
     ifstream iFile, rFile;
     bool verbose = 0;
     int numFrames;
     string line;
     int aflag, bflag;
     const char* cvalue;
     bool O=false,P,F,S,p,f;
     unique_ptr<Scheduler> sched;
     bool fs=false;

     char c;
     if (argc < 3) {
        printf("Invalid number of parameters.\n");
        return 1;
     }

     while ((c = getopt (argc, argv, "s:")) != -1)
     switch (c){
        case 's':
            switch(optarg[0]){

                    case 'i':sched.reset(new fifo());break;
                    case 'f': fs=true;
                            sched.reset(new fscan());break;
                    case 's':sched.reset(new scan());break;
                    case 'c':sched.reset(new cscan());break;
                    case 'j':sched.reset(new sstf());break;


                    default:  break;
            }
            break;
        case '?':
            if (optopt == 'c')
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
            return 1;
        default:
            abort ();
      }
      iFile.open(argv[argc - 1]);

      if (iFile == NULL) {
        fprintf(stderr, "Instruction file not found.\n");
        return 1;
      }

            int id =0;
           while(std::getline(iFile, line)){
            int i;
            unsigned int p;
            instr ins;
            const char * buf = line.c_str();
            if (buf[0] != '#'){
                sscanf(buf, "%d%u", &i, &p);
                ins.loc = p;
                ins.id = id++;
                ins.ts=i;
                instructions.push_back(ins);
            }

      }
      int curLoc =0;
      int preLoc =0;
      bool seeking = false;
      int clock = 0;
      instruction curIns;
      int totalM = 0 ;
      int totalW = 0;
      int turnTime = 0;
      int instcount = 0;
      bool  sQueue;
      int mWait = std::numeric_limits<int>::min();

      int size_inst = instructions.size();
      bool right;
      while(instcount<size_inst){

            if(!seeking && !readyq.empty()){
                {

                    //introduce algo here
                    if(preLoc > curLoc)
                        right  = false;
                    else
                        right = true;
                    curIns = sched->getInst(curIns,right);
                    seeking = true;

                    totalM+= abs(curIns.loc-curLoc);
                    totalW+= abs(curIns.ts-clock);
                    if(abs(curIns.ts-clock) > mWait){
                        mWait = abs(curIns.ts-clock);
                    }
                  //  cout<<clock<<": "<<curIns.id<<" issue "<<curIns.loc<<" "<<curLoc<<endl;

                }

            }
            if (seeking){
                    int finTime = clock+abs(curIns.loc-curLoc);


                    while(instructions.size()>0 && instructions[0].ts <= finTime){
                        instruction temp = instructions.front();
                        instructions.erase(instructions.begin());
                        clock = temp.ts;
                        sched->putInst(temp,seeking);
                      //  cout<<temp.ts<<": "<<temp.id<<" add "<<temp.loc<<endl;

                    }
                    clock = finTime;
                    if(curLoc!=curIns.loc)
                        preLoc = curLoc;
                    curLoc = curIns.loc;

                    seeking = false;
                    turnTime+=clock - curIns.ts;

                    if( fs){
                        if(readyq.size() == 0){
                            readyq = altq;
                            altq.clear();
                            preLoc = curLoc;

                        }
                    }


                    instcount++ ;
            }
            else{
                  curIns = instructions.front();
                    instructions.erase(instructions.begin());
                    sched->putInst(curIns,seeking);
                    clock= curIns.ts;
                   // cout<<clock<<": "<<curIns.id<<" add "<<curIns.loc<<endl;
            }
      }
      cout<<"SUM: "<<clock<<" "<<totalM<<" "<<fixed<<setprecision(2)<<double(turnTime)/instcount<<" "
      <<fixed<<setprecision(2)<<double(totalW)/instcount<<" "<<mWait<<endl;
    return 0;
}
