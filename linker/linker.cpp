#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <iostream>
#include <vector>
#include <utility>
#include <map>
#include <iterator>
#include <tuple>
#include <cmath>

using namespace std;
using vecit=vector<tuple<string,int, int, bool, string>>::iterator;

char * getToken(FILE *  file, char ** lineptr, int * nreadptr, int * nlineptr, int * last_line_nread_ptr, int * lineoffset_ptr)
{
    size_t len = 0;
    char * tok;
    if(ftell(file)==0)
    {
        (*nreadptr) = getline(lineptr, &len, file);
        if( (*nreadptr) != -1)
        {
            tok = strtok(*lineptr, " \t\n\r");
            (*nlineptr)++;
            (*last_line_nread_ptr) = (*nreadptr);
            (*lineoffset_ptr) = (tok - (*lineptr) +1);
        }
        
        return tok;
    }
    if ( (tok = strtok(NULL, " \t\n\r")) == NULL)
    {
        (*nreadptr) = getline(lineptr, &len, file);
        if( (*nreadptr) != -1)
        {
            tok = strtok(*lineptr, " \t\n\r");
            (*nlineptr)++;
            (*last_line_nread_ptr) = (*nreadptr);
            if (tok == NULL)
            {
                return getToken(file, lineptr, nreadptr, nlineptr, last_line_nread_ptr, lineoffset_ptr);
            }
            
        }
        
    }
    (*lineoffset_ptr) = (tok - (*lineptr) +1);

    return tok;

}

void __parseerror(int errcode, int linenum, int lineoffset) 
{
    static string errstr[] = {
    "TOO_MANY_DEF_IN_MODULE", // > 16
    "TOO_MANY_USE_IN_MODULE", // > 16
    "TOO_MANY_INSTR", // total num_instr exceeds memory size (512)
    "NUM_EXPECTED", // Number expect, anything >= 2^30 is not a number either
    "SYM_EXPECTED", // Symbol Expected
    "MARIE_EXPECTED", // Addressing Expected which is M/A/R/I/E
    "SYM_TOO_LONG", // Symbol Name is too long
    "TOO_MANY_SYMBOLS", // Symbols exceeded 256
    "TOO_MANY_MODULES", // Modules exceeded 128
    };
    printf("Parse Error line %d offset %d: %s\n", linenum, lineoffset, errstr[errcode].c_str());
    exit(1);
}


int readInt(FILE *  file, char ** lineptr, int * nreadptr, int * nlineptr, int * last_line_nread_ptr, int * lineoffset_ptr)
{
    char* token  = getToken(file, lineptr, nreadptr, nlineptr, last_line_nread_ptr, lineoffset_ptr);
    char * endptr;
    if(token == NULL)
    {
        return -1;
    }
    long num = strtol(token,&endptr,10);
    if(endptr==token)
    {
        __parseerror(3,*nlineptr,*lineoffset_ptr);
    }
    if(num >=((long)pow(2,30)) || num < 0)
    {
        __parseerror(3,*nlineptr,*lineoffset_ptr);
    }
    int i = (long)num;
    if(*endptr != '\0')
    {
        __parseerror(3,*nlineptr,*lineoffset_ptr);
    }
    return i;
    
}

char * readSymbol(FILE *  file, char ** lineptr, int * nreadptr, int * nlineptr, int * last_line_nread_ptr, int * lineoffset_ptr)
{
    char * token = getToken(file, lineptr, nreadptr, nlineptr, last_line_nread_ptr, lineoffset_ptr);
    if(token == NULL)
    {
        __parseerror(4,*nlineptr,*last_line_nread_ptr);
    }
    int len = strlen(token);
    if(len >  16)
    {
        __parseerror(6,*nlineptr,*lineoffset_ptr);
    }
    if((isalpha((int)*token))==0)
    {
        __parseerror(4,*nlineptr,*lineoffset_ptr);
    }
    for(char *ptr = token;(*ptr)!='\0';ptr++)
    {
        if((isalnum((int)*ptr))==0)
        {
            __parseerror(4,*nlineptr,*lineoffset_ptr);
        }
    }
    return token;
}

char readMARIE(FILE *  file, char ** lineptr, int * nreadptr, int * nlineptr, int * last_line_nread_ptr, int * lineoffset_ptr)
{
    char * token = getToken(file, lineptr, nreadptr, nlineptr, last_line_nread_ptr, lineoffset_ptr);
    if(token == NULL)
    {
        __parseerror(5,*nlineptr,*last_line_nread_ptr);
    }
    int len = strlen(token);
    if(len!=1)
    {
        __parseerror(5,*nlineptr,*lineoffset_ptr);
    }
    if(*token == 'M' || *token == 'A' || *token == 'R' || *token == 'I' || *token == 'E')
    {
        return *token;
    }
    else
    {
        __parseerror(5,*nlineptr,*lineoffset_ptr);
    }
    return *token;
}

vecit find_symbol(vecit itr, vecit enditr, string s)
{
    for(;itr!=enditr;itr++)
    {
        if(get<0>(*itr)==s)
        {
            return itr;
        }
    }
    return itr;
}

void pass1(FILE * file, vector<tuple<string,int,int,bool,string>>  * symbol_table, vector<int> * module_base)
{
    rewind(file);
    char *line;
    size_t len=0;
    int nread;
    int nline = 0;
    int last_line_nread = -1;
    int lineoffset;
    int base_addr = 0;
    int nmodule = 0;
    while(true)
    {
        int defcount = readInt(file,&line,&nread,&nline,&last_line_nread, &lineoffset);
        if(defcount < 0)
        {
            break;
        }
        if(defcount>16)
        {
            __parseerror(0,nline,lineoffset);
        }
        // cout<<defcount<<endl;
        if(module_base->size()==128)
        {
            __parseerror(8,nline,lineoffset);
        }
        vector<tuple<string,int,bool>> deflist;
        for(int i = 0; i < defcount;i++)
        {
            string symbol = string(readSymbol(file,&line,&nread,&nline,&last_line_nread, &lineoffset));
            int val = readInt(file,&line,&nread,&nline,&last_line_nread, &lineoffset);
            // cout<<symbol<<" "<<val<<endl;
            auto itr1 = find_symbol(symbol_table->begin(),symbol_table->end(),symbol);
            if(symbol_table->size()==256)
            {
                __parseerror(7,nline,lineoffset);
            }
            if(itr1==symbol_table->end())
            {
                symbol_table->push_back(make_tuple(symbol,base_addr+val,nmodule,false, ""));
                deflist.push_back(make_tuple(symbol,val,false));
            }
            else
            {
                deflist.push_back(make_tuple(symbol,val,true));
                get<4>(*itr1) = "Error: This variable is multiple times defined; first value used";
            }
            
        }
        int usecount = readInt(file,&line,&nread,&nline,&last_line_nread, &lineoffset);
        if(usecount>16)
        {
            __parseerror(1,nline,lineoffset);
        }
        for(int i = 0; i < usecount; i++)
        {
            string symbol = string(readSymbol(file,&line,&nread,&nline,&last_line_nread, &lineoffset));
        }

        int instcount = readInt(file,&line,&nread,&nline,&last_line_nread, &lineoffset);
        if((instcount+base_addr)>512 || instcount>512)
        {
            __parseerror(2,nline,lineoffset);
        }
        for(int i = 0;i < instcount; i++)
        {
            char addressmode = readMARIE(file,&line,&nread,&nline,&last_line_nread, &lineoffset);
            int operand = readInt(file,&line,&nread,&nline,&last_line_nread, &lineoffset);
        }
        for(auto itr: deflist)
        {
            if(get<2>(itr))
            {
                cout<<"Warning: Module "<<nmodule<<": "<<get<0>(itr)<<" redefinition ignored\n";
            }
            else if(get<1>(itr) > (instcount-1))
            {
                cout<<"Warning: Module "<<nmodule<<": "<<get<0>(itr)<<"="<<(get<1>(itr))<<" valid=[0.."<<(instcount-1)<<"] assume zero relative\n";
                get<1>(*(find_symbol(symbol_table->begin(),symbol_table->end(),get<0>(itr)))) = base_addr;
            }
        }
        module_base->push_back(base_addr);
        nmodule++;
        base_addr += instcount;
    }
}

void pass2(FILE * file, vector<tuple<string,int, int, bool, string>>  * symbol_table, vector<int> * module_base)
{
    rewind(file);
    char *line;
    size_t len=0;
    int nread;
    int nline = 0;
    int last_line_nread = -1;
    int lineoffset;
    int base_addr = 0;
    int nmodule = 0;
    while(true)
    {
        vector<pair<string, bool>> uselist;
        int defcount = readInt(file,&line,&nread,&nline,&last_line_nread, &lineoffset);
        if(defcount < 0)
        {
            break;
        }
        if(defcount>16)
        {
            __parseerror(0,nline,lineoffset);
        }
        for(int i = 0; i < defcount;i++)
        {
            string symbol = string(readSymbol(file,&line,&nread,&nline,&last_line_nread, &lineoffset));
            int val = readInt(file,&line,&nread,&nline,&last_line_nread, &lineoffset);
        }
        int usecount = readInt(file,&line,&nread,&nline,&last_line_nread, &lineoffset);
        if(usecount>16)
        {
            __parseerror(1,nline,lineoffset);
        }
        for(int i = 0; i < usecount; i++)
        {
            string symbol = string(readSymbol(file,&line,&nread,&nline,&last_line_nread, &lineoffset));
            uselist.push_back(make_pair(symbol,false));
        }

        int instcount = readInt(file,&line,&nread,&nline,&last_line_nread, &lineoffset);
        if((instcount+base_addr)>512 || instcount>512)
        {
            __parseerror(2,nline,lineoffset);
        }
        for(int i = 0;i < instcount; i++)
        {
            char addressmode = readMARIE(file,&line,&nread,&nline,&last_line_nread, &lineoffset);
            int inst = readInt(file,&line,&nread,&nline,&last_line_nread, &lineoffset);
            int opcode = inst / 1000;
            int operand = inst % 1000;
            if(opcode >= 10)
            {
                printf("%03d: %04d Error: Illegal opcode; treated as 9999\n",base_addr+i,9999);
                continue;
            }
            if(addressmode == 'I')
            {
                if((operand >= 900))
                {
                    printf("%03d: %d%03d Error: Illegal immediate operand; treated as 999\n",base_addr+i,opcode,999);
                }
                else
                {
                    printf("%03d: %d%03d\n",base_addr+i,opcode, operand);
                }
            }
            else if(addressmode == 'M')
            {
                if(operand > module_base->size()-1)
                {
                    printf("%03d: %d%03d Error: Illegal module operand ; treated as module=0\n",base_addr+i,opcode,0);
                }
                else
                {
                    printf("%03d: %d%03d\n",base_addr+i,opcode,module_base->at(operand));
                }
            }
            else if(addressmode == 'A')
            {
                if(operand >= 512)
                {
                    printf("%03d: %d%03d Error: Absolute address exceeds machine size; zero used\n",base_addr+i,opcode,0);
                }
                else
                {
                    printf("%03d: %d%03d\n",base_addr+i,opcode,operand);
                }
            }
            else if(addressmode == 'R')
            {
                if(operand > instcount)
                {
                    printf("%03d: %d%03d Error: Relative address exceeds module size; relative zero used\n",base_addr+i,opcode,base_addr);
                }
                else
                {
                    printf("%03d: %d%03d\n",base_addr+i,opcode,operand+base_addr);
                }
            }
            else if(addressmode == 'E')
            {
                if(operand >= usecount)
                {
                    printf("%03d: %d%03d Error: External operand exceeds length of uselist; treated as relative=0\n",base_addr+i,opcode,base_addr);
                }
                else
                {
                    uselist[operand].second = true;
                    auto itr = find_symbol(symbol_table->begin(),symbol_table->end(),uselist[operand].first);
                    if( itr == symbol_table->end())
                    {
                        printf("%03d: %d%03d Error: %s is not defined; zero used\n",base_addr+i,opcode,0, uselist[operand].first.c_str());
                    }
                    else
                    {
                        printf("%03d: %d%03d\n",base_addr+i,opcode,get<1>(*itr));
                        get<3>(*itr) = true;
                    }
                }
            }

        }
        for(int i = 0; i < usecount;i++)
        {
            if(!uselist[i].second)
            {
                printf("Warning: Module %d: uselist[%d]=%s was not used\n",nmodule,i,uselist[i].first.c_str());
            }
        }
        base_addr += instcount;
        nmodule++;
    }
    cout<<"\n";
    for(auto itr= symbol_table->begin();itr!=symbol_table->end();itr++)
    {
        if(!get<3>(*itr))
        {
            printf("Warning: Module %d: %s was defined but never used\n",get<2>(*itr),get<0>(*itr).c_str());
        }
    }
    cout<<"\n";
}


int main(int argc, char** argv)
{
    
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE * file;
    file = fopen(argv[1], "r");
    
    if(file == NULL)
    {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    vector<tuple<string,int,int, bool,string>> symbol_table;
    vector<int> module_base;
    pass1(file, &symbol_table, &module_base);
    cout<<"Symbol Table\n";
    for(auto itr: symbol_table)
    {
        cout<<get<0>(itr)<<"="<< get<1>(itr)<<" "<<get<4>(itr)<<"\n";
    }
    cout<<"\nMemory Map\n";
    pass2(file, &symbol_table, &module_base);
    return 0;
}
