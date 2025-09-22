#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <iostream>

#include <iostream>
#include <vector>
#include <tuple>
#include "linker.h"
using namespace std;

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
            if (tok == NULL)
