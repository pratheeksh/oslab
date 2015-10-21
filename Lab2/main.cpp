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
using namespace std;

vector<int> randvals;
int ofs = 0;
int schedType = 0;
int quant1;
int myrandom(int burst) {
    if (ofs > randvals.size()){
        ofs = 0;
    }
    return (1 + (randvals[ofs++] % burst));
}

class process{
    public:
    int at, tc, cb, io, rem, pid, priority, cw, ft, tt, it=0, rt = 0, burst=0,dp;
    bool exp = 0;
    process(int id, int a, int t, int c, int i, int p) {
    pid = id;
    at = a;
    tc = t;
    cb = c;
    io = i;
    rem = tc;
    priority = p;
    dp = p-1;
   // rt = at;
    }
    void setExp(bool b){
        exp = b;
    }
    bool getExp(){
        return exp;
    }
    void setDP(int b){
        dp = b;
    }
    int getDP(){
        return dp;
    }

    void setBurst(int b){
        burst = b;
    }
    int getBurst(){
        return burst;
    }
    void setRt(int readytime){
        rt = readytime;
    }
    int getRt(){
        return rt;
    }
    void setIt(int iotime){
        it = iotime;
    }
    int getIt(){
        return it;
    }
    void setFt(int finishtime){
        ft = finishtime;
    }
    int getFt(){
        return ft;
    }
    void setTt(int turntime){
        tt = turntime;
    }
    int getTt(){
        return tt;
    }
    void setCPUWait(int cwait){
        cw = cwait;
    }
    int getCPUWait(){
        return cw;
    }
    void setPid(int id) {
        pid = id;
    }
    int getPid() {
        return pid;
    }
    void setPrio(int p) {
        priority = p;
    }
    int getPrio() {
        return priority;
    }
    void setAt(int a) {
        at = a;
    }
    int getAt() {
        return at;
    }
    void setTc(int t) {
        tc = t;
    }
    int getTc() {
        return tc;
    }
    void setCb(int c) {
        cb = c;
    }
    int getCb() {
        return cb;
    }
    void setIo(int i) {
        io = i;
    }
    int getIo() {
        return io;
    }
    void setRem(int r){
        rem = r;
    }
    int getRem(){
        return rem;
    }
};
class event{

    public:
    int pid;
    int transition;
    int created;
    int timestamp;
    event(){ return;}
    event(int ts, int c, int p, int t){
        transition = t;
        created = c;
        pid = p;
        timestamp =ts;
    }
};

class Scheduler{

    public:
        int type;
        vector<process>readyQueue;
        vector<process>expiredQueue;
        Scheduler(){return;}
        void setType(int t)
        {
            type=t;
        }
        int getType(){
            return type;
        }
        virtual int getQuantum(){
            return 0;
        }
        virtual string getSchedName() = 0;
        virtual void putProcess(process) = 0;
        virtual process getProcess() = 0;
};
class fifo:public Scheduler{
    public:
        fifo(){return;}
        fifo(int t){
            type = t;
        }
        string getSchedName(){
            return "FCFS";
        }
        void putProcess(process p){
            readyQueue.push_back(p);
        }
        process getProcess(){
            if (!readyQueue.empty()){
                process p = readyQueue.front();
                readyQueue.erase(readyQueue.begin());
                return p;
               }
        }


};
class lifo:public Scheduler{
    public:
        lifo(){return;}
        lifo(int t){
            type = t;
        }

        string getSchedName(){
            return "LCFS";
        }
        void putProcess(process p){
            readyQueue.push_back(p);
        }
        process getProcess(){
            if(!readyQueue.empty()){
                process p = readyQueue.back();
                readyQueue.erase(readyQueue.end());
                return p;
            }
         }
};
class sjf:public Scheduler{
    public:
        int i;
        sjf(){return;}
        sjf(int t){
            type = t;
        }

        string getSchedName(){
            return "SJF";
        }
        void putProcess(process p){
            if(!readyQueue.empty()){
                for(i=0;i<readyQueue.size();i++){
                    if(readyQueue[i].getRem()>p.getRem())
                        break;
                }
                readyQueue.insert(readyQueue.begin()+i,p);
            }
            else{
                readyQueue.push_back(p);
            }

        }
        process getProcess(){
            if(!readyQueue.empty()){
                process p = readyQueue.front();
                readyQueue.erase(readyQueue.begin());
                return p;
            }
         }
};


class rr:public Scheduler{

    public:
        int quant;
        rr(){return;}
        rr(int t,int q){
            type = t;
            quant = q;
        }

        string getSchedName(){
            return "RR";
        }
        void putProcess(process p){
            readyQueue.push_back(p);
        }
        process getProcess(){
            if(!readyQueue.empty()){
                process p = readyQueue.front();
                readyQueue.erase(readyQueue.begin());
                return p;
            }
        }
        int getQuantum(){
            return quant;
        }

};
class prio:public Scheduler{

    public:
        int i=0;
        int quant;
        prio(){return;}
        prio(int t,int q){
            type = t;
            quant = q;
        }

        string getSchedName(){
            return "PRIO";
        }
        void putProcess(process p){
            // cout<<p.pid<<endl;
            if(p.getExp() == 0)
            {
                if(!readyQueue.empty()){
                    for(i=0; i<readyQueue.size(); i++)
                    {
                        if(readyQueue[i].getDP() < p.getDP())
                        break;
                    }
                     readyQueue.insert(readyQueue.begin()+i,p);
                }
                else{
                    readyQueue.push_back(p);
                }


              /*  cout<<"inserting "<<p.pid<<" pos"<<i<<endl;
                for(int j=0; j<readyQueue.size();j++)
                    cout<<readyQueue[j].pid<<" ";
                cout<<endl;*/
            }

            else{
                i=0;
                //p.setDP(p.getPrio()-1);
                if(!expiredQueue.empty())
                {
                    for(i=0; i<expiredQueue.size(); i++)
                    {
                        if(expiredQueue[i].getDP() < p.getDP())
                        break;
                    }
                     expiredQueue.insert(expiredQueue.begin()+i,p);

                }
                else{
                    expiredQueue.push_back(p);
                }


            }
        }
        process getProcess(){
            if(readyQueue.empty())

            {
              //  cout<<"switching"<<endl;

                if(!expiredQueue.empty())
                {
                    int t = expiredQueue.size();
                    readyQueue.reserve(t);
                    copy(expiredQueue.begin(),expiredQueue.end(),back_inserter(readyQueue));
                    expiredQueue.clear();

                }

            }
            if(!readyQueue.empty())

            {
                //
                //cout<<endl;
                process p = readyQueue.front();
                    //p.setPrio(p.getPrio()-1);
                readyQueue.erase(readyQueue.begin());

                return p;
            }




        }
        int getQuantum(){
            return quant;
        }

};
class eventHandler{


    public:
    int q=0;
    vector<event> eventList;
    eventHandler(){
        return;
    }
    void setQuantum(int qtm){
        q = qtm;
    }
    void insertEvent(event a){
        int i;

        for(i=0;i<eventList.size();i++)
        {
            event cur = eventList[i];
            if(a.timestamp < cur.timestamp)
            {
                break;
            }
        }
        eventList.insert(eventList.begin()+i,a);
    }
    event getEvent(){
        if(eventList.size()>0){

            event cur = eventList.front();
            eventList.erase(eventList.begin());

            return cur;
        }
    }
    Scheduler* getScheduler(int schedType){

        if(schedType == 0)
            return  new fifo(0);
        else if (schedType == 1)
            return new lifo(1);
        else if (schedType == 2)
            return new sjf(2);
        else if (schedType == 3)
            return new rr(3,q);
            return new prio(4,q);

    }

}handler;
vector<process> procList;

int main(int argc, char* argv[]) {
     ifstream iFile, rFile;
     bool verbose = 0;

     string line;
     quant1=0;
     Scheduler* scheduler;
     if (argc < 2) {
        cout << "Invalid paramters" << endl;
        return -1;
     }
     else {
        bool firstfile=1;
        for (int i=1;i<argc;i++)
        {
            if (strcmp(argv[i],"-v")==0)
                verbose = 1;

            else if(argv[i][0]== '-'){
                if(argv[i][1] == 's'){
                    if (argv[i][2]== 'F')
                        schedType = 0;
                    else if (argv[i][2]== 'L')
                        schedType = 1;
                    else if (argv[i][2]== 'S')
                        schedType = 2;
                    else if (argv[i][2]== 'R')
                    {
                        schedType = 3;
                        sscanf(argv[i] + 3, "%d", &quant1);
                    }

                    else if (argv[i][2]== 'P')
                    {
                        schedType = 4;
                        sscanf(argv[i] + 3, "%d", &quant1);
                    }

                    else {cout<<"wrong scheduler type"<<endl; return 1;}

                }
            }
            else if(firstfile){
                iFile.open(argv[i]);
                firstfile = 0;
            }
            else{
                rFile.open(argv[i]);
            }

        }
        if (quant1>0){
            handler.setQuantum(quant1);
        }
        scheduler = handler.getScheduler(schedType);
        //cout<<scheduler->getQuantum()<<" "<<quant1;
        while(std::getline(rFile, line))
        {
            const char * c = line.c_str();
            int num = atoi(c);
            randvals.push_back(num);
        }
        randvals.erase(randvals.begin());
        int pid = 0;
        while(getline(iFile,line))
        {

            int a,t,c,i,ppri;

            const char * buf = line.c_str();
            ppri=myrandom(4);
            sscanf(buf, "%d%d%d%d", &a, &t, &c, &i);
            procList.push_back(process(pid,a,t,c,i,ppri));
            pid++;

           //cout << pid;
        }
    }
    //return 0;
    for (int i=0; i<procList.size();i++)
    {
        event a;
        process tmp = procList[i];
        a = event(tmp.getAt(),tmp.getAt(),tmp.getPid(),0);
        handler.insertEvent(a);


    }
    for (int i=0; i< handler.eventList.size();i++)
    {
        event a = handler.eventList[i];
     //   cout << a.transition << " "<<endl;

    }
    int timer = procList[0].getAt();
    int startNext=0;
    int timelapse = 0;
    int blockedproc = 0;
    int startio = 0;
    double iolapse=0;
    int randCPU;
    while(handler.eventList.size()!=0){
        event cur = handler.getEvent();

     //   process procList[cur_pid] = procList[cur_pid];
        int cur_pid = cur.pid;
        //cout << cur_pid <<" *** "<< cur.transition <<endl;
        if(cur.timestamp > timer)
            timer=cur.timestamp;

        if(cur.transition == 0){
            //run
            if(verbose)
            cout<<timer<<" "<<cur_pid<<" "<<" "<<"CREATED -> READY"<<endl;
            procList[cur_pid].setRt(timer);
            procList[cur_pid].setCPUWait(0);
            event newEve(procList[cur_pid].getAt(),procList[cur_pid].getAt(),cur_pid,1); //goback and check
            handler.insertEvent(newEve);
            scheduler->putProcess(procList[cur_pid]);

          //  handler.schedule(procList[cur_pid],schedType);
        }
        else if(cur.transition == 1){
            //cout << startNext<<"**"<<cur.timestamp<<endl;
            if (cur.timestamp<startNext)
            {
                cur.timestamp = startNext;
                handler.insertEvent(cur);
                continue;
            }

            process p = scheduler->getProcess();

            cur_pid = p.pid;
            //cout << p.getRt()<<endl;
            int cw = p.getCPUWait();
            int rt = p.getRt();
            int x = cw + timer - rt;
 //           cout<<cur_pid<<" "<<timer<<" "<<x<<" "<<rt<<endl;
            procList[cur_pid].setCPUWait(x);
            procList[cur_pid].setExp(false);

            if(scheduler->getType() == 3 || scheduler->getType()==4){
                int q = scheduler->getQuantum();
                //cout<<"Pid:"<<cur_pid<<endl;

                if (procList[cur_pid].getBurst() == 0)
                {
                    randCPU =  myrandom(procList[cur_pid].getCb());
                    procList[cur_pid].setBurst(randCPU);

                }

                if (q >= procList[cur_pid].getBurst())
                {
                    if(procList[cur_pid].getRem() <= procList[cur_pid].getBurst() )
                    {
                //finish
                        if(verbose)

                        cout<<timer<<" "<<cur_pid<<" "<<"READY -> RUNNG "<<"cb:"<<procList[cur_pid].rem<<" rem:"<<procList[cur_pid].rem<<" Prio:"<<procList[cur_pid].getDP()<<endl;
                        startNext = timer + procList[cur_pid].rem;

                        procList[cur_pid].setBurst(0);
                        procList[cur_pid].setFt(timer+procList[cur_pid].rem);
                        int tt = timer+procList[cur_pid].rem - procList[cur_pid].getAt();
                        procList[cur_pid].setTt(tt);
                        procList[cur_pid].setRem(0);
                        if(verbose)
                        cout<<timer+procList[cur_pid].rem<<" "<<cur_pid<<" "<<"DONE"<<endl;

                        //event newEve(timer+procList[cur_pid].rem,timer,cur_pid,5); //goback and check
                        //handler.insertEvent(newEve);
                       // procList[cur_pid].setDP(procList[cur_pid].getDP()-1);

                    }
                    else{
                        if(verbose)
                        cout<<timer<<" "<<cur_pid<<" "<<"READY -> RUNNG "<<"cb:"<< procList[cur_pid].getBurst()<<" rem:"<<procList[cur_pid].rem<<" Prio:"<<procList[cur_pid].getDP()<<endl;
                        procList[cur_pid].setRem(procList[cur_pid].getRem()- procList[cur_pid].getBurst());
                        procList[cur_pid].setDP(procList[cur_pid].getPrio()-1);
                        event newEve(timer+procList[cur_pid].getBurst(),timer,cur_pid,3);
                        handler.insertEvent(newEve);
                        startNext = timer + procList[cur_pid].getBurst();
                        procList[cur_pid].setBurst(0);

                    }


                }
                else
                {
                    if(procList[cur_pid].getRem()>q)
                    {
                        if(verbose)
                        cout<<timer<<" "<<cur_pid<<" "<<"READY -> RUNNG "<<"cb:"<<procList[cur_pid].getBurst()<<" rem:"<<procList[cur_pid].rem<<" Prio:"<<procList[cur_pid].getDP()<<endl;
                        procList[cur_pid].setRem(procList[cur_pid].getRem()-q);
                        procList[cur_pid].setBurst(procList[cur_pid].getBurst()-q);

                        procList[cur_pid].setDP(procList[cur_pid].getDP()-1);


                        event newEve(timer+q,timer,cur_pid,6);
                        handler.insertEvent(newEve);

                        startNext = timer + q;

                    }
                    else
                    {
                        if(verbose)
                        cout<<timer<<" "<<cur_pid<<" "<<"READY -> RUNNG "<<"cb:"<<procList[cur_pid].getRem()<<" rem:"<<procList[cur_pid].rem<<" Prio:"<<procList[cur_pid].getDP()<<endl;

                        procList[cur_pid].setBurst(0);
                       //
                        //event newEve(timer+procList[cur_pid].getRem(),timer,cur_pid,5);
                        //handler.insertEvent(newEve);
                        procList[cur_pid].setFt(timer+procList[cur_pid].rem);
                        int tt = timer+procList[cur_pid].rem - procList[cur_pid].getAt();
                        procList[cur_pid].setTt(tt);
                        if(verbose)
                        cout<<timer+procList[cur_pid].rem<<" "<<cur_pid<<" "<<"DONE"<<endl;


                        startNext = timer + procList[cur_pid].getRem();
                        procList[cur_pid].setRem(0);
                    }

                }

            }
         //   cout << cur_pid <<endl;
            else{
                randCPU =  myrandom(procList[cur_pid].getCb());
          //  procList[cur_pid].setCb(procList[cur_pid].getCb()-randCPU);

                if(procList[cur_pid].getRem()< randCPU)
                {
                        if(verbose)
                        cout<<timer<<" "<<cur_pid<<" "<<"READY -> RUNNG "<<"cb:"<<procList[cur_pid].rem<<" rem:"<<procList[cur_pid].rem<<" Prio:"<<procList[cur_pid].getDP()<<endl;
                        startNext = timer + procList[cur_pid].rem;

                        procList[cur_pid].setBurst(0);
                        procList[cur_pid].setFt(timer+procList[cur_pid].rem);
                        int tt = timer+procList[cur_pid].rem - procList[cur_pid].getAt();
                        procList[cur_pid].setTt(tt);
                        procList[cur_pid].setRem(0);
                        if(verbose)
                        cout<<timer+procList[cur_pid].rem<<" "<<cur_pid<<" "<<"DONE"<<endl;

                }
                else
                {
                    //block
                    if(verbose)

                    cout<<timer<<" "<<cur_pid<<" "<<"READY -> RUNNG "<<"cb:"<<randCPU<<" rem:"<<procList[cur_pid].rem<<" Prio:"<<procList[cur_pid].getDP()<<endl;
                    procList[cur_pid].setRem(procList[cur_pid].getRem()-randCPU);

                    event newEve(timer+randCPU,timer,cur_pid,3);
                    handler.insertEvent(newEve);
                    startNext = timer + randCPU;
                  //  cout<<procList[cur_pid].getRem()<<endl;
                }
            }
        }
        else if(cur.transition == 3){
            if (procList[cur_pid].getRem()>0)
            {
                blockedproc++;
                if (blockedproc == 1)
                    startio = timer;
                int ioBurst= myrandom(procList[cur_pid].getIo());
                procList[cur_pid].setIt(procList[cur_pid].getIt()+ioBurst);
                event newEve(timer+ioBurst,timer,cur_pid,4);
                handler.insertEvent(newEve);
                if(verbose)
                cout<<timer<<" "<<cur_pid<<" "<<"RUNNG -> BLOCK "<<"ib:"<<ioBurst<<" rem:"<<procList[cur_pid].rem<<" Prio:"<<procList[cur_pid].getDP()<<endl;
            //timer = timer+ioBurst;//works for fifo
            }
            else
            {
                event newEve(timer,timer,cur_pid,5);
                handler.insertEvent(newEve);
            }

        }
        else if(cur.transition == 4)

        {
                blockedproc--;
                if (blockedproc == 0)
                   //cout<<"current time:"<<timer;
                   iolapse+= timer - startio;
                    //cout <<" Iolapse:"<< iolapse << endl;


                if(verbose)
                cout<<timer<<" "<<cur_pid<<" "<<"BLOCK -> READY "<<" Prio:"<<procList[cur_pid].getDP()<<endl;
                procList[cur_pid].setRt(timer);
                //procList[cur_pid].setDP(procList[cur_pid].getPrio()-1);
                scheduler->putProcess(procList[cur_pid]);
                event newEve(timer,timer,cur_pid,1);
                handler.insertEvent(newEve);

        }
        else if(cur.transition == 5)

        {
             procList[cur_pid].setFt(timer);
             int tt = timer - procList[cur_pid].getAt();
             procList[cur_pid].setTt(tt);
             if(verbose)
             cout<<timer<<" "<<cur_pid<<" "<<"DONE"<<endl;

        }
        else if(cur.transition == 6)
        {
            if (verbose)
            cout<<timer<<" "<<cur_pid<<" "<<"RUNNG -> READY "<<" Prio:"<<procList[cur_pid].getDP()<<endl;
       //     procList[cur_pid].setDP(procList[cur_pid].getDP()-1);
            procList[cur_pid].setRt(timer);

            if(procList[cur_pid].getDP()<0)
            {
                procList[cur_pid].setExp(1);
                procList[cur_pid].setDP(procList[cur_pid].getPrio()-1);
            }


            procList[cur_pid].setRt(timer);

            scheduler->putProcess(procList[cur_pid]);

            event newEve(timer,timer,cur_pid,1);

            handler.insertEvent(newEve);

        }
        //timelapse = timer - timelapse;
        //cout<<"current time:"<<timer<<" time lapsed:"<<timelapse<<" No of blocked procs:"<<blockedproc<<endl;
    }
    int lastfinished = 0;
    double cpuUtil = 0;
    double ioUtil = 0;
    double avgTurn = 0;
    double avgCW = 0;
    double avgP = procList.size();
    cout<<scheduler->getSchedName();
    if(quant1>0)
        cout<<" "<<quant1;
    cout<<endl;
    for (int i=0; i< procList.size();i++){
        if(lastfinished < procList[i].getFt())
            lastfinished = procList[i].getFt();
        cpuUtil+= procList[i].getTc();
        avgTurn+= procList[i].getTt();
        avgCW+= procList[i].getCPUWait();
        cout << std::setfill ('0') << std::setw (4)<<procList[i].getPid()<<": ";
        cout << setfill(' ')<<std::setw (4)<< procList[i].getAt() <<" "<<std::setw (4)<< procList[i].getTc()<<" ";
        cout<< std::setw (4)<< procList[i].getCb()<<" "<<std::setw (4)<<procList[i].getIo()<<" "<<procList[i].getPrio()<<" | ";
        cout<< setfill(' ')<<std::setw (5)<<procList[i].getFt()<<" "<<std::setw (5)<<procList[i].getTt()<<" "
        <<std::setw (5)<<procList[i].getIt()<<" "<<std::setw (5)<<procList[i].getCPUWait()<<endl;
    }

    cpuUtil = (cpuUtil/lastfinished)*100;
    ioUtil = (iolapse/lastfinished)*100;
    avgP = (avgP/lastfinished)*100;
    //cout << avgTurn<<endl;
    //cout << i << endl;

    avgTurn = (avgTurn/procList.size());
    avgCW = (avgCW/procList.size());


    cout<<"SUM: "<< setw (3)<<lastfinished<<" " <<fixed<<setprecision(2)<< cpuUtil<< " "<<setprecision(2)<< ioUtil<< " "<<setprecision(2)<< avgTurn<<" "<<setprecision(2)<<avgCW<<" "<<setprecision(3)<<avgP<<endl;



    return 0;
}
