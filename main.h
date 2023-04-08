#ifndef __MAIN_H__
#define __MAIN_H__

#include <netdb.h>

typedef struct {
    int type;
    int value;
} terminal;

void cleanSpaces(char**);
void errprint(char*);
void parserTests();

int evaluation(char, int, int, int*);
int exprNonterminal(char**, int*);
int operatorNonterminal(char**, char*);
int spaceNonterminal(char**);
int queryNonterminal(char**, int*);

#endif