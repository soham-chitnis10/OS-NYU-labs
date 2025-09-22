#include "linker.h"
#include <iostream>
using namespace std;

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
        if(module_base->size()==128)
        {
            __parseerror(8,nline,lineoffset);
        }
        vector<tuple<string,int,bool>> deflist;
        for(int i = 0; i < defcount;i++)
        {
            string symbol = string(readSymbol(file,&line,&nread,&nline,&last_line_nread, &lineoffset));
            int val = readInt(file,&line,&nread,&nline,&last_line_nread, &lineoffset);
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
