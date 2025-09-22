#include "linker.h"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cstdio>

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
