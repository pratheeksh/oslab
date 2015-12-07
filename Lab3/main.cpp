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
int mapcount = 0;
int unmapcount = 0;
typedef struct instr{
    int rw;
    unsigned int page;
}instruction;
vector<int> randvals;
vector<unsigned int> rev_frametable;

vector<instr> instructions;
vector<unsigned int> frametable;
vector<unsigned int> pagetable(64,0);

unsigned int isPresent(unsigned int p) {
    return ((p >> 31) | 0);
}

void setFrameNumber(unsigned int &p,  int f) {
    p = (p & 0xf0000000) | f;
}

unsigned int getFrameNumber(unsigned int p) {
    return ((p << 4) >> 4);
}
void setPresentBit(unsigned int& p, int t) {
    if (t == 1){
        unsigned int mask = 0x80000000;
        p |= mask;
    }
    else{
        unsigned int mask = 0x7fffffff;
        p &= mask;
    }
}

unsigned int isModified(unsigned int p) {
    return ((p >> 30) & 1) | 0;
}
void setModifiedBit(unsigned int &p, int t){
    if (t == 1){
        unsigned int mask = 0x40000000;
        p |= mask;
    }
    else{
        unsigned int mask = 0xbfffffff;
        p &= mask;
    }
}

void setReferencedBit(unsigned int& p, int t) {
    if(t == 1){
        unsigned int mask = 0x20000000;
        p |= mask;

    }
    else{
        unsigned int mask = 0xdfffffff;
        p &= mask;

    }

}

unsigned int isReferenced(unsigned int p) {
    return ((p >> 29) & 1) | 0;
}
void setPageoutBit(unsigned int& p, int t) {
    if (t ==1 ){
        unsigned int mask = 0x10000000;
        p |= mask;
    }
    else{
        unsigned int mask = 0xefffffff;
        p &= mask;

    }

}

unsigned int isPagedout(unsigned int p) {
    return ((p >> 28) & 1) | 0;
}

class mmu{
    public:
        int numFrames;
        mmu(){return;}
        virtual void updateFrame(vector<unsigned int>&, unsigned int)=0;
        virtual unsigned int getFrame(vector<unsigned int>&,vector<unsigned int>&, vector<unsigned int>&) = 0;
        void setNumOfFrames(int n){
            numFrames = n;
        }

};
class LRU:public mmu{

    public:
        unsigned int getFrame(vector<unsigned int>& pagetable, vector<unsigned int>& frametable, vector<unsigned int>& rev_frametable){
            frametable.push_back(frametable.front());
            frametable.erase(frametable.begin());
            return frametable.back();
        }
        void updateFrame(vector<unsigned int>& frametable, unsigned int frameNo){
           vector<unsigned int>::iterator it = find(frametable.begin(), frametable.end(), frameNo);
            if(it!= frametable.end()){
                frametable.erase(it);
            }
            frametable.push_back(frameNo);
        }
};

int ofs = 0;
unsigned int myrandom(int burst) {
    if (ofs > randvals.size()){
        ofs = 0;
    }
    return  (randvals[ofs++] % burst);
}

class Random:public mmu{

    public:
        unsigned int getFrame(vector<unsigned int>& pagetable, vector<unsigned int>& frametable, vector<unsigned int>& rev_frametable){
            int t = frametable.size();
            return myrandom(t);
        }
        void updateFrame(vector<unsigned int>& frametable, unsigned int frameNo){
        }

};
class FIFO:public mmu{
    public:
        unsigned int getFrame(vector<unsigned int>& pagetable, vector<unsigned int>& frametable, vector<unsigned int>& rev_frametable){
            frametable.push_back(frametable.front());
            frametable.erase(frametable.begin());
            return frametable.back();
        }
        void updateFrame(vector<unsigned int>& frametable, unsigned int frameNo){
        }
};
class secondChance:public mmu{
    public:
        unsigned int getFrame(vector<unsigned int>& pagetable, vector<unsigned int>& frametable, vector<unsigned int>& rev_frametable){


            unsigned int firstFrame = frametable.front();
            unsigned int page = rev_frametable[firstFrame];
            while(isReferenced(pagetable[page])){
                setReferencedBit(pagetable[page],0);
                frametable.push_back(frametable.front());
                frametable.erase(frametable.begin());
                firstFrame = frametable.front();
                page = rev_frametable[firstFrame];

            }
            frametable.push_back(frametable.front());
            frametable.erase(frametable.begin());
            return frametable.back();
        }
        void updateFrame(vector<unsigned int>& frametable, unsigned int frameNo){
        }

};

class clockP:public mmu{
    private: int hand;

    public:

        unsigned int getFrame(vector<unsigned int>& pagetable, vector<unsigned int>& frametable, vector<unsigned int>& rev_frametable){
            unsigned int firstFrame,page;
            bool t = false;
            while(!t){
                firstFrame = frametable[hand];
                page = rev_frametable[firstFrame];
                if (isReferenced(pagetable[page]))
                    setReferencedBit(pagetable[page],0);
                else
                    t = true;
                hand = (hand+1)%frametable.size();
            }
            return firstFrame;
        }
        void updateFrame(vector<unsigned int>& frametable, unsigned int frameNo){
        }

};

class clockV:public mmu{
    private: int hand;

    public:

        unsigned int getFrame(vector<unsigned int>& pagetable, vector<unsigned int>& frametable, vector<unsigned int>& rev_frametable){
            unsigned int page;
            int saved;
            bool t = false;
            while(!t){
                if(isPresent(pagetable[hand])){
                    if (isReferenced(pagetable[hand])){
                        setReferencedBit(pagetable[hand],0);
                    }
                    else{
                        t = true;
                        saved = hand;
                    }
                }
                hand = (hand+1)%pagetable.size();
            }
            return getFrameNumber(pagetable[saved]);
        }
        void updateFrame(vector<unsigned int>& frametable, unsigned int frameNo){
        }

};
class NRU:public mmu{
    public:

        int timer;
        unsigned int getFrame(vector<unsigned int>& pagetable, vector<unsigned int>& frametable, vector<unsigned int>& rev_frametable){
            vector<vector<unsigned int> > Class(4, vector<unsigned int>());
            for (int i= 0;i<pagetable.size();++i){
               unsigned int page = pagetable[i];
                if (isPresent(page)){
                    int j = 2*isReferenced(page)+isModified(page);
                    Class[j].push_back(getFrameNumber(page));

                }
            }
            unsigned int ret;
            int i,j;

            for (i=0;i<4;i++)
            {
                if (Class[i].size()>0){
                    unsigned int frameIn = myrandom(Class[i].size());
                    ret = Class[i][frameIn];
                    break;
                }
            }
            timer++;
            if(timer == 10){
                timer = 0;
                for (int i=0;i<pagetable.size();i++){
                    if(isPresent(pagetable[i]))
                        setReferencedBit(pagetable[i],0);
                }
            }
            return ret;
        }


        void updateFrame(vector<unsigned int>& frametable, unsigned int frameNo){

        }

};
class agingP:public mmu{
    public:
      //  vector <unsigned int> counter;
        vector <unsigned int>counter;

        //agingP(int numFrames) {counter= vector <unsigned int>(numFrames,0);}

        unsigned int getFrame(vector<unsigned int>& pagetable, vector<unsigned int>& frametable, vector<unsigned int>& rev_frametable){
            unsigned int min_count = std::numeric_limits<unsigned int>::max();
            int i = 0;
            int min_in = -1;

            if (counter.size()==0)
                counter= vector <unsigned int>(numFrames,0);

            for(vector<unsigned int> ::iterator it= frametable.begin();it!=frametable.end();it++){
                unsigned int frNo = *it;
                unsigned int page = pagetable[rev_frametable[frNo]];
                counter[i] = counter[i]>>1;
                unsigned int isref = (isReferenced(page))<<31;
                counter[i] = counter[i]|isref;
                if(counter[i]<min_count){
                    min_count =counter[i];
                    min_in = i;

                }
                setReferencedBit(page,0);
                pagetable[rev_frametable[frNo]] = page;
                i++;
            }

            counter[min_in] = 0;
            return (frametable[min_in]);


        }
         void updateFrame(vector<unsigned int>& frametable, unsigned int frameNo){

        }
};
class agingV:public mmu{
    public:
      //  vector <unsigned int> counter;
        vector <unsigned int>counter;
        agingV() {counter= vector <unsigned int>(64,0);}

        unsigned int getFrame(vector<unsigned int>& pagetable, vector<unsigned int>& frametable, vector<unsigned int>& rev_frametable){
            unsigned int min_count = std::numeric_limits<unsigned int>::max();
            int i = 0;
            unsigned int min_in = -1;
            for(vector<unsigned int> ::iterator it= pagetable.begin();it!=pagetable.end();it++){
                unsigned int page = *it;
                counter[i] = counter[i]>>1;
                unsigned int isref = (isReferenced(page))<<31;
                counter[i] = counter[i]|isref;
                if(isPresent(page)){
                    if(counter[i]<min_count){
                        min_count =counter[i];
                        min_in = i;
                    }
                    setReferencedBit(page,0);
                    pagetable[i] = page;
                }
                i++;
            }
            counter[min_in] = 0;
            return (getFrameNumber(pagetable[min_in]));

        }


        void updateFrame(vector<unsigned int>& frametable, unsigned int frameNo){

        }
};


int algoType = 0, optType=0;


void printPT(vector<unsigned int>& pagetable){
    for (int i=0; i< pagetable.size();i++){
        if(isPresent(pagetable[i])){
            cout<<i<<":";
            if(isReferenced(pagetable[i]))
                cout<<"R";
            else
                cout <<"-";
            if(isModified(pagetable[i]))
                cout<<"M";
            else
                cout<<"-";
            if(isPagedout(pagetable[i]))
                cout<<"S";
            else
                cout<<"-";
        }
        else{
            if(isPagedout(pagetable[i]))
                cout<<"#";
            else
                cout<<"*";
        }
        cout<<" ";
    }
    cout <<endl;
}
void printFT(vector<unsigned int> rev_frametable){
    vector<unsigned int>::iterator it;
    for(it=rev_frametable.begin();it!=rev_frametable.end(); ++it){
        if(*it == -1)
            cout<< "* ";
        else
            cout<< *it<<" ";

    }
    cout <<endl;

}

int main(int argc, char* argv[]) {
     ifstream iFile, rFile;
     bool verbose = 0;
     int numFrames;
     string line;
     int aflag, bflag;
     const char* cvalue;
     unique_ptr<mmu> algorithm;
     bool O=false,P,F,S,p,f;

     char c;
     if (argc < 3) {
        printf("Invalid number of parameters.\n");
        return 1;
     }

     while ((c = getopt (argc, argv, "a:o:f:")) != -1)
     switch (c){
        case 'a':
            switch(optarg[0]){
                    case 'N': algorithm.reset(new NRU()); break;
                    case 'l': algorithm.reset(new LRU()); break;
                    case 'r': algorithm.reset(new Random());break;
                    case 'f': algorithm.reset(new FIFO()); break;
                    case 's': algorithm.reset(new secondChance()); break;
                    case 'c': algorithm.reset(new clockP());break;
                    case 'C': algorithm.reset(new clockV());break;
                    case 'X': algorithm.reset(new clockV());break;

                    case 'a': algorithm.reset(new agingP());break;
                    case 'A': algorithm.reset(new agingV());break;
                    case 'Y': algorithm.reset(new agingV());break;

                    default:  break;
            }
            break;
        case 'o':
            for (int i = 0; i < strlen(optarg); i++) {
                    switch (optarg[i]) {
                        case 'O': O = true; break;
                        case 'P': P = true; break;
                        case 'F': F = true; break;
                        case 'S': S = true; break;
                        case 'p': p = true; break;
                        case 'f': f = true; break;
                        default: break;
                    }
                }
            break;
        case 'f':
            cvalue = optarg;
            numFrames = atoi(cvalue);
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
      iFile.open(argv[argc - 2]);

      if (iFile == NULL) {
        fprintf(stderr, "Instruction file not found.\n");
        return 1;
      }
      rFile.open(argv[argc - 1]);

      if (rFile == NULL) {
        fprintf(stderr, "Random file not found.\n");
        return 1;
      }
      while(std::getline(rFile, line)){
        const char * c = line.c_str();
        int num = atoi(c);
        randvals.push_back(num);
      }
      randvals.erase(randvals.begin());
      /*while(std::getline(iFile, line)){
            int i;
            unsigned int p;
            instr ins;
            const char * buf = line.c_str();
            if (buf[0] != '#'){
                sscanf(buf, "%d%u", &i, &p);
                if(p>63)
                    return 1;
                ins.page = p;
                ins.rw = i;
                instructions.push_back(ins);
            }

      }*/
      //algorithm.reset(new LRU());
      algorithm->setNumOfFrames(numFrames);
      rev_frametable = vector <unsigned int>(numFrames, -1);
      int instrCount = 0;
      int frametableCount = 0;
      int pageoutCount=0;
      int pageinCount = 0;
      int zeroCount = 0;
      long long total = 0;
      int in;
      unsigned int pno;
      while(getline(iFile, line)){

            const char * buf = line.c_str();
            if (buf[0] != '#'){
                sscanf(buf, "%d%u", &in, &pno);

             if(p>63)
                    return 1;
            int pageIndex = pno;
            unsigned int page = pagetable[pageIndex];
            if (O)
                cout<<"==> inst: "<<in <<" "<<pno<<endl;
            //instructions.erase(instructions.begin());
            int frameNo;
            if(!isPresent(page)){
                frameNo = frametable.size();
                if(frametable.size()<numFrames){
                    frametable.push_back(frameNo);
                    mapcount++;
                    zeroCount++;
                    total+= 400;
                    if (O)
                    cout<<instrCount<<": ZERO " << frameNo<<endl;
                    total+=150;
                    if (O)
                    cout << instrCount << ": MAP " <<pageIndex<<" "<<frameNo<<endl;

                }
                else{
                        //get framenum from algo here
                        frameNo = algorithm->getFrame(pagetable,frametable,rev_frametable);
                        unsigned int inToReplace =  rev_frametable[frameNo];
                        unsigned int prePage  = pagetable[inToReplace];
                        //cout<<instrCount<<": ZERO " << frameNo<<endl;
                        if (O)
                        cout << instrCount << ": UNMAP " <<inToReplace<<" "<<frameNo<<endl;
                        unmapcount++;
                        total+=400;
                        setPresentBit(prePage,0);
                        setReferencedBit(prePage,0);
                        if(isModified(prePage)){
                            setModifiedBit(prePage,0);
                            setPageoutBit(prePage,1);
                            pageoutCount++;
                            total+=3000;
                            if (O)
                            cout << instrCount << ": OUT " <<inToReplace<<" "<<frameNo<<endl;
                        }
                        pagetable[inToReplace]=prePage;
                        //add the modified bit part for write

                        if(isPagedout(page)){
                            pageinCount++;
                            total+=3000;
                            if (O)
                            cout << instrCount << ": IN " <<pageIndex<<" "<<frameNo<<endl;
                        }else{
                                if (O)
                                cout<<instrCount<<": ZERO " << frameNo<<endl;
                                total+=150;
                                zeroCount++;

                        }
                        if (O)
                        cout << instrCount << ": MAP " <<pageIndex<<" "<<frameNo<<endl;
                        mapcount++;
                        total+=400;
                }
                 rev_frametable[frameNo] = pageIndex;
                 setFrameNumber(page,frameNo);
                 setPresentBit(page,1);
                 setReferencedBit(page,1);
                 if(in == 1){
                    setModifiedBit(page,1);
                 }
                 pagetable[pageIndex] = page;
            }
            else{

            //    cout <<instrCount<<" "<<pageIndex <<endl;
                frameNo = getFrameNumber(page);
                algorithm->updateFrame(frametable,frameNo);
                setPresentBit(page,1);
                setReferencedBit(page,1);
                if(in == 1){
                    setModifiedBit(page,1);
                }
                pagetable[pageIndex] = page;
            }
        instrCount++;
        total+=1;
        if(f)
            printFT(frametable);
        if(p)
            printPT(pagetable);

      }
    }
      if (P)
        printPT(pagetable);
      if (F)
      printFT(rev_frametable);
      if (S)
      cout<<"SUM "<<instrCount<<" U="<<unmapcount<<" M="<<mapcount<<" I="<<pageinCount<<" O="<<
      pageoutCount<<" Z="<<zeroCount<<" ===> "<<total<<endl;
      return 0;

}
