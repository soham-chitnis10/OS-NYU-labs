#ifndef LINKER_H
#define LINKER_H

#include <vector>
#include <string>
#include <tuple>
#include <cstdio>

using namespace std;
using vecit = vector<tuple<string,int, int, bool, string>>::iterator;

char * getToken(FILE *  file, char ** lineptr, int * nreadptr, int * nlineptr, int * last_line_nread_ptr, int * lineoffset_ptr);
void __parseerror(int errcode, int linenum, int lineoffset);
int readInt(FILE *  file, char ** lineptr, int * nreadptr, int * nlineptr, int * last_line_nread_ptr, int * lineoffset_ptr);
char * readSymbol(FILE *  file, char ** lineptr, int * nreadptr, int * nlineptr, int * last_line_nread_ptr, int * lineoffset_ptr);
char readMARIE(FILE *  file, char ** lineptr, int * nreadptr, int * nlineptr, int * last_line_nread_ptr, int * lineoffset_ptr);
vecit find_symbol(vecit itr, vecit enditr, string s);
void pass1(FILE * file, vector<tuple<string,int,int,bool,string>>  * symbol_table, vector<int> * module_base);
void pass2(FILE * file, vector<tuple<string,int, int, bool, string>>  * symbol_table, vector<int> * module_base);

#endif // LINKER_H
