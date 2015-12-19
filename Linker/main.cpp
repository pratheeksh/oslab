#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include <cstdio>
#include <string>
#include <vector>
#include <cctype>
#include <queue>
#include <map>
using namespace std;
const string ERRORCODE="ERROR";
map<string, int> symbolTable;
map<string, int>usedUselist;
map<string, int> usedDefinitions;
typedef struct TOKEN {
    string data;
    int lineNum;
    int pos;
}token;
typedef struct SYM{
    string name;
    int value;
}symbol;
typedef struct INSTRUCTION {
    string code;
    string instr;
} instruction;
queue<token> tokenqueue1;
queue<token> tokenqueue2;
vector<int> baseaddress;
vector<vector<instruction> > instr;
vector<vector<symbol> > globalSymDef;
vector<vector<string> > uselistSym;
bool generate_tokens() {
    string line;
    int lineCount = 0;
    bool retVal;
    int lastLine, lastPos;
    while (getline(cin, line)) {
        retVal = true;
        lineCount++;
        for (int i = 0; i < line.size(); ++i) {
            if (!isspace(line[i])) {
                retVal = false;
                int j;
                for (j = i; j < line.size(); ++j) {
                    if (isspace(line[j])) {
                        break;
                    }
                }
                token new_token;
                new_token.data = line.substr(i, j - i);
                new_token.lineNum = lineCount;
                new_token.pos = i + 1;
                lastPos = new_token.pos + new_token.data.length();
                tokenqueue1.push(new_token);
                tokenqueue2.push(new_token);
                i = j;
            }
        }
        lastLine = line.size();
    }
    //indicate end of tokenqueue1
    token t;
    t.data="prattest";
    t.lineNum=lineCount;
    if(retVal){
        if(lastLine>0){
            t.pos= lastLine;
        }
        else{
            t.pos = 1;
        }
    }
    else{
        t.pos = lastPos;
    }
    tokenqueue1.push(t);
    tokenqueue2.push(t);
    return retVal;
}
int toNum(string t){
    int n=0;
    for (int i=0;i<t.size();i++){
        n = n*10 + t[i] - '0';
    }
    return n;
}
int checkNum(token t){
    int len=t.data.size();
    int n = 0;
    int i;
    for (i=0;i<len;i++)
    {
        if(!isdigit(t.data[i]))
            return -1;
    }
    n=toNum(t.data);
    return n;
}
string checkSym(token t){
    if (!isalpha(t.data[0])) {
        return ERRORCODE;
    }
    for (int i = 1; i < t.data.size(); ++i) {
        if (!isalnum(t.data[i])) {
            return ERRORCODE;
        }
    }
    return t.data;
}
string checkAddr(token t) {
    string ad=t.data;
    if (ad.length() > 1) {
        return ERRORCODE;
    } else {
        if (ad[0] != 'I' && ad[0] != 'A'&&ad[0] != 'R' && ad[0] != 'E') {
            return ERRORCODE;
        } else {
            return ad;
        }
    }
}

void handle_error(token t,int errcode){
    static string errortypes[]={
    "NUM_EXPECTED",
    "SYM_EXPECTED",
    "ADDR_EXPECTED",
    "SYM_TOLONG",
    "TO_MANY_DEF_IN_MODULE",
    "TO_MANY_USE_IN_MODULE",
    "TO_MANY_INSTR"
    };
    cout<<"Parse Error line "<<t.lineNum<<" offset "<<t.pos<<": "<<errortypes[errcode].c_str()<<endl;
}
bool buildDefn(){
        vector<symbol> definition;
        token t = tokenqueue1.front();
        tokenqueue1.pop();
        if (tokenqueue1.size() == 0) {
            handle_error(t, 0);
            return false;
        }
        int defcount = checkNum(t);
        if (defcount == -1) {
            handle_error(t, 0);
            return false;
        } else if (defcount > 16) {
            handle_error(t, 4);
            return false;
        }
        for (int i = 0; i < defcount; ++i) {
            t = tokenqueue1.front();
            tokenqueue1.pop();
            if (tokenqueue1.size() == 0) {
                handle_error(t, 1);
                return false;
            }
            string sym = checkSym(t);
            if (sym == ERRORCODE) {
                handle_error(t, 1);
                return false;
            } else if (sym.length() > 16) {
                handle_error(t, 3);
                return false;
            }
            t = tokenqueue1.front();
            tokenqueue1.pop();
            if (tokenqueue1.size() == 0) {
                handle_error(t, 0);
                return false;
            }
            int val = checkNum(t);
            if (val == -1) {
                handle_error(t, 0);
                return false;
            }
            symbol defSym;
            defSym.name = sym;
            defSym.value = val;
            definition.push_back(defSym);
        }
        globalSymDef.push_back(definition);
        return true;
}
bool checkUselist(){
        token t = tokenqueue1.front();
        tokenqueue1.pop();
        if (tokenqueue1.size() == 0) {
            handle_error(t, 0);
            return false;
        }
        int usecount = checkNum(t);
        if (usecount == -1) {
            handle_error(t, 0);
            return false;
        } else if (usecount > 16) {
            handle_error(t, 5);
            return false;
        }
        for (int i = 0; i < usecount; ++i) {
            t = tokenqueue1.front();
            tokenqueue1.pop();
            if (tokenqueue1.size() == 0) {
                handle_error(t, 1);
                return false;
            }
            string sym = checkSym(t);
            if (sym == ERRORCODE) {
                handle_error(t, 1);
                return false;
            } else if (sym.length() > 16) {
                handle_error(t, 3);
                return false;
            }

        }
        return true;
}
bool checkInstr(int* instrCount){

        token t = tokenqueue1.front();
        tokenqueue1.pop();
        if (tokenqueue1.size() == 0) {
            handle_error(t, 0);
            return false;
        }
        int codecount = checkNum(t);
        if (codecount == -1) {
            handle_error(t, 0);
            return false;
        } else if (codecount + *instrCount > 512) {
            handle_error(t, 6);
            return false;
        }
        for (int i = 0; i < codecount; ++i) {
            ++*instrCount;
            t = tokenqueue1.front();
            tokenqueue1.pop();

            if (tokenqueue1.size() == 0) {
                handle_error(t, 2);
                return false;
            }
            string insCode = checkAddr(t);
            if (insCode == ERRORCODE) {
                handle_error(t, 2);
            }
            t = tokenqueue1.front();
            tokenqueue1.pop();

            if (tokenqueue1.size() == 0 || checkNum(t) == -1) {
                handle_error(t, 0);
                return false;
            }
        }
    baseaddress.push_back(*instrCount);
    return true;
}
bool parse_tokens(){
    int instrCount=0;
    baseaddress.push_back(instrCount);

    while (tokenqueue1.size() > 1) {
        bool ret_value = buildDefn();
        if (!ret_value)
            return false;
        ret_value = checkUselist();
         if (!ret_value)
            return false;
        ret_value=checkInstr(&instrCount);
            if (!ret_value)
            return false;
        }
    return true;
}
void second_parse_tokens(){
    int instrCount = 0;
    while (tokenqueue2.size() > 1) {
        vector<string> usetoken;
        vector<instruction> instrtoken;
        token t = tokenqueue2.front();
        tokenqueue2.pop();
        int defcount = checkNum(t);
        for (int i = 0; i < defcount; ++i) {
            tokenqueue2.pop();
            tokenqueue2.pop();
        }
        t = tokenqueue2.front();
        tokenqueue2.pop();
        int usecount = checkNum(t);
        for (int i = 0; i < usecount; ++i) {
            t = tokenqueue2.front();
            tokenqueue2.pop();
            string sym = t.data;
            usetoken.push_back(sym);
        }
        uselistSym.push_back(usetoken);
        t = tokenqueue2.front();
        tokenqueue2.pop();
        int codecount = checkNum(t);
        for (int i = 0; i < codecount; ++i) {
            ++instrCount;
            t = tokenqueue2.front();
            tokenqueue2.pop();
            string insCode = t.data;
            t = tokenqueue2.front();
            tokenqueue2.pop();
            string addr = t.data;
            instruction ins;
            ins.code = insCode;
            ins.instr = addr;
            instrtoken.push_back(ins);
        }
        instr.push_back(instrtoken);
    }
}

void printSymboltable() {
    map<string, int> symbolHash;
    int key=1;
    vector<string> symtableOut;
    for (int i = 0; i < globalSymDef.size(); ++i) {
        for (int j = 0; j < globalSymDef[i].size(); ++j) {
            symbol sym = globalSymDef[i][j];
            int hashRet = symbolHash[sym.name];
            if (hashRet == 0) {
                symbolHash[sym.name] = key;
                int relativePos = sym.value + baseaddress[i];
                if (relativePos >= baseaddress[i + 1]) {
                        symbolTable[sym.name] = 1;
                        symtableOut.push_back(sym.name+"="+"0");
                        key++;
                        cout << "Warning: Module " << i + 1 << ": " <<sym.name << " to big " << relativePos << " (max=" <<
                        baseaddress[i + 1] - 1 << ") assume zero relative" << endl;
                } else {
                    symbolTable[sym.name] = sym.value + baseaddress[i] + 1;
                    int def_add=sym.value + baseaddress[i];
                    stringstream def_add_s;
                    def_add_s<<def_add;
                    symtableOut.push_back(sym.name+"="+def_add_s.str());
                    key++;
                }
            } else {
                    symtableOut[hashRet-1]+=" Error: This variable is multiple times defined; first value used";
                    symbolHash[sym.name] = -1;
                    globalSymDef[i].erase(globalSymDef[i].begin() + j);
                    --j;
            }
        }
    }
    cout << "Symbol Table" << endl;
    for (int i = 0; i < symtableOut.size(); ++i) {
        cout << symtableOut[i] << endl;
    }
    tokenqueue1.pop();
}

string check_I(string addr){
    int n = toNum(addr);
    if (n<0 || n>9999) {
        addr = "9999 Error: Illegal immediate value; treated as 9999";
    } else {
        char buffer[10];
        stringstream buff;
        sprintf(buffer, "%04d", n);
        buff << buffer;
        addr = buff.str();
    }
    return addr;
}
 string check_A(string addr)
 {
        int n = toNum(addr);
        if (n<0 || n>9999) {
        addr = "9999 Error: Illegal opcode; treated as 9999";
        }
        else{
            n=toNum(addr.substr(1,addr.size()));
            if (n >= 512) {
                addr = string(1, addr[0]) +"000 Error: Absolute address exceeds machine size; zero used";
            }
            else {
                char target[4];
                stringstream buff1,buff2;
                sprintf(target, "%03d", n);
                buff1 << target;
                buff2 << addr[0];
                addr = buff2.str() + buff1.str();
            }
        }
    return addr;
}
string check_R(string addr,int ma){

        int n = toNum(addr);

        if (n<0 || n>9999) {
        addr = "9999 Error: Illegal opcode; treated as 9999";
        }
        else {
            n=toNum(addr.substr(1,addr.size()));
            if (baseaddress[ma] + n >= baseaddress[ma + 1]) {
                char target[4] = {0};
                stringstream buff1,buff2;
                sprintf(target, "%03d", baseaddress[ma]);
                buff1 << target;
                buff2 << addr[0];
                addr = buff2.str() + buff1.str() +
                    " Error: Relative address exceeds module size; zero used";
            } else {
                char target[4] = {0};
                stringstream buff2,buff1;
                sprintf(target, "%03d", n + baseaddress[ma]);
                buff1 << target;
                buff2 << addr[0];
                addr =  buff2.str() + buff1.str();
            }
        }
    return addr;
}
string check_E(string addr, int ma)
{
    int n = toNum(addr);
    if (n<0 || n>9999) {
        addr = "9999 Error: Illegal opcode; treated as 9999";
    }
    else {
        n = toNum(addr.substr(1,addr.size()));
        if (n >= uselistSym[ma].size()) {
            addr += " Error: External address exceeds length of uselist; treated as immediate";
        }
        else {
            string replaceSymbol = uselistSym[ma][n];
            usedUselist[replaceSymbol]=1;
            if (symbolTable[replaceSymbol] == 0) {
                addr += " Error: " + replaceSymbol +" is not defined; zero used";
            }
            else {
                usedDefinitions[replaceSymbol] = 1;
                char target[4] = {0};
                stringstream buff1,buff2;
                sprintf(target, "%03d", symbolTable[replaceSymbol] - 1);
                buff1 << target;
                buff2 << addr[0];
                addr = buff2.str() + buff1.str();
            }
        }
    }
    return addr;
}

string secondPass(instruction in, int ma) {
    string addr = in.instr;
    if (in.code == "I")
    {
        addr=check_I(addr);
    }
    else if (in.code == "A")
    {
        addr=check_A(addr);
    }
    else if (in.code == "R")
    {
        addr=check_R(addr,ma);

    } else if (in.code == "E")
    {
        addr=check_E(addr,ma);
    }
    return addr;
}
void printMemoryMap(){
    cout<<"Memory Map"<<endl;
    for (int i=0; i<instr.size();i++){
        for (int j=0;j<instr[i].size();j++){
            printf("%03d: ", baseaddress[i] + j);
            cout<<secondPass(instr[i][j],i)<<endl;
        }
        for (int j = 0; j < uselistSym[i].size(); ++j) {
            string s = uselistSym[i][j];
            if (usedUselist[s] == 0) {
                stringstream ss;
                cout << "Warning: Module " << i + 1 << ": " << s <<
                    " appeared in the uselist but was not actually used" << endl;
            }
        }
    }
    for (int i = 0; i < globalSymDef.size(); ++i) {
        for (int j = 0; j < globalSymDef[i].size(); ++j) {
            symbol s= globalSymDef[i][j];
            if (usedDefinitions[s.name] == 0) {
                cout << "Warning: Module " << i + 1 << ": " << s.name <<
                    " was defined but never used" << endl;
            }
        }
    }
}
int main(int argc, char* argv[]) {
     if (argc == 1) {
         cout << "Pass the file name.as an argument" << endl;
         return -1;
     } else {
         if (!freopen(argv[1], "r", stdin)) {
             cout << "Failed to open the file." << endl;
             return -1;
         }
     }
    bool ret_value = generate_tokens();
    ret_value=parse_tokens();
    if (ret_value){
        printSymboltable();
        second_parse_tokens();
        printMemoryMap();
    }

    return 0;
}
