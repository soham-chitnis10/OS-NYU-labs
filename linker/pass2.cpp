#include "linker.h"
#include <iostream>
using namespace std;

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
