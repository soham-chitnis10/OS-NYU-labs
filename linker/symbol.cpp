#include "linker.h"

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
