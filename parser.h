#ifndef __PARSER_H__
#define __PARSER_H__

#define PARSE_SUCCESS 0
#define PARSE_FAIL -1

int startParsing(char*, int*);
int evaluation(char, int, int, int*);
int exprNonterminal(char**, int*);
int operatorNonterminal(char**, char*);
int spaceTerminal(char**);
int queryNonterminal(char**, int*);
int queryEndNonterminal(char**, int*, char);

#endif