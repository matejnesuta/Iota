#ifndef __MAIN_H__
#define __MAIN_H__

#include <netdb.h>

#define PARSE_SUCCESS 0
#define PARSE_FAIL -1

typedef struct {
    int type;
    int value;
} terminal;

void errprint(char*);
int startParsing(char*, int*);
int evaluation(char, int, int, int*);
int exprNonterminal(char**, int*);
int operatorNonterminal(char**, char*);
int spaceTerminal(char**);
int queryNonterminal(char**, int*);
int caseInsensitiveStrcmp(char*, char*);

#endif