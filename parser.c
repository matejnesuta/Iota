#include <stdlib.h>

#include "main.h"
#include "parser.h"

/* Pro parsovani vyrazu jsem se rozhodl pouzit rekurzivni sestup a jednoduchou
gramatiku o 10 pravidlech, ktera vypada nasledovne (neterminaly jsou popsany
velkym a terminaly malym pismem):

S -> QUERY eps
QUERY -> left_bracket OPERATOR space EXP space QUERY_END
OPERATOR -> plus
OPERATOR -> minus
OPERATOR -> multiplication
OPERATOR -> division
EXP -> QUERY
EXP -> number
QUERY_END -> EXP END
QUERY_END -> right_bracket

Pri parsovani rovnou dochazi i k vyhodnoceni vyrazu, ktery se dostava ven
pomoci ukazatele result. Parser vraci chybu pri spatnem formatu vstupu nebo
pokud behem vyhodnoceni muze dojit k deleni nulou. Parser je testovan unit
testy v souboru tests.c.
*/

int evaluation(char operation, int operand1, int operand2, int* result) {
    switch (operation) {
        case '+':
            *result = operand1 + operand2;
            break;
        case '-':
            *result = operand1 - operand2;
            break;
        case '*':
            *result = operand1 * operand2;
            break;
        case '/':
            if (operand2 == 0) {
                return PARSE_FAIL;
            } else {
                *result = operand1 / operand2;
            }
            break;
        default:
            errprint("something bad happend in the evaluation function pal.\n");
            return PARSE_FAIL;
    }
    return PARSE_SUCCESS;
}

/* tato funkce je prezitek z dob, kdy jsem jeste nevedel, ze je protokolem
pevne dany pocet mezer.*/
int spaceTerminal(char** cursor) {
    if (*cursor[0] != ' ') {
        return PARSE_FAIL;
    }
    (*cursor)++;
    return PARSE_SUCCESS;
}

int operatorNonterminal(char** cursor, char* op) {
    if (*cursor[0] == '+' || *cursor[0] == '-' || *cursor[0] == '*' ||
        *cursor[0] == '/') {
        *op = *cursor[0];
        (*cursor)++;
        return PARSE_SUCCESS;
    } else {
        return PARSE_FAIL;
    }
}

int startParsing(char* cursor, int* result) {
    if (queryNonterminal(&cursor, result)) {
        return PARSE_FAIL;
    } else {
        // kontrola, zda za danym vyrazem jeste nelezi nejake znaky navic
        if (cursor[0] != '\0') {
            return PARSE_FAIL;
        }
        return PARSE_SUCCESS;
    }
}

int exprNonterminal(char** cursor, int* result) {
    if (*cursor[0] == '(') {
        if (queryNonterminal(cursor, result)) {
            return PARSE_FAIL;
        } else {
            return PARSE_SUCCESS;
        }
    } else if ((*cursor[0] >= '0' && *cursor[0] <= '9')) {
        *result = strtol(*cursor, cursor, 10);
        return PARSE_SUCCESS;
    } else {
        return PARSE_FAIL;
    }
}

int queryEndNonterminal(char** cursor, int* result, char operator) {
    if (*cursor[0] == ')') {
        (*cursor)++;
        return PARSE_SUCCESS;
    } else if (spaceTerminal(cursor)) {
        return PARSE_FAIL;
    }
    int operand2;

    if (exprNonterminal(cursor, &operand2)) {
        return PARSE_FAIL;
    }

    if (evaluation(operator, * result, operand2, result)) {
        return PARSE_FAIL;
    }

    if (queryEndNonterminal(cursor, result, operator)) {
        return PARSE_FAIL;
    } else {
        return PARSE_SUCCESS;
    }
}

int queryNonterminal(char** cursor, int* result) {
    char operator;
    int operand1;
    int operand2;
    if (*cursor[0] != '(') {
        return PARSE_FAIL;
    }
    (*cursor)++;
    if (operatorNonterminal(cursor, &operator)) {
        return PARSE_FAIL;
    }
    if (spaceTerminal(cursor)) {
        return PARSE_FAIL;
    }
    if (exprNonterminal(cursor, &operand1)) {
        return PARSE_FAIL;
    }
    if (spaceTerminal(cursor)) {
        return PARSE_FAIL;
    }
    if (exprNonterminal(cursor, &operand2)) {
        return PARSE_FAIL;
    }
    if (evaluation(operator, operand1, operand2, result)) {
        return PARSE_FAIL;
    }
    if (queryEndNonterminal(cursor, result, operator)) {
        return PARSE_FAIL;
    }
    return PARSE_SUCCESS;
}
