#ifndef __MAIN_H__
#define __MAIN_H__

#include <netdb.h>

#define PARSE_SUCCESS 0
#define PARSE_FAIL -1

typedef struct {
    int type;
    int value;
} terminal;

void cleanSpaces(char**);
void errprint(char*);

int evaluation(char, int, int, int*);
int exprNonterminal(char**, int*);
int operatorNonterminal(char**, char*);
int spaceNonterminal(char**);
int queryNonterminal(char**, int*);

#endif