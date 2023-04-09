#include "tests.h"
#include <stdio.h>
#include <stdlib.h>
#include "main.h"

int testHandler(char* input) {
    int result;
    if (queryNonterminal(&input, &result)) {
        return PARSE_FAIL;
    } else {
        printf("The calculated result was: %d.\n", result);
        return PARSE_SUCCESS;
    }
}

void parserTests() {
    printf("\n//////// PARSER TESTS ////////\n");
    if (testHandler("") == PARSE_FAIL) {
        printf("empty: ok\n");
    } else {
        printf("empty: fail\n");
    }
    if (testHandler("      ") == PARSE_FAIL) {
        printf("whitespaces: ok\n");
    } else {
        printf("whitespaces: fail\n");
    }
    printf("\nsmokes:\n");
    if (testHandler("(* 4 5)") == PARSE_SUCCESS) {
        printf("multiplication: ok\n");
    } else {
        printf("multiplication: fail\n");
    }
    if (testHandler("(+ 4 5)") == PARSE_SUCCESS) {
        printf("addition: ok\n");
    } else {
        printf("addition: fail\n");
    }
    if (testHandler("(- 4 5)") == PARSE_SUCCESS) {
        printf("subtraction: ok\n");
    } else {
        printf("subtraction: fail\n");
    }
    if (testHandler("(/ 4 5)") == PARSE_SUCCESS) {
        printf("division: ok\n");
    } else {
        printf("division: fail\n");
    }
    printf("\nspaces:\n");
    if (testHandler("    (* 4 5)") == PARSE_SUCCESS) {
        printf("spaces 1: ok\n");
    } else {
        printf("spaces 1: fail\n");
    }
    if (testHandler("(   *    4   5   )     ") == PARSE_SUCCESS) {
        printf("spaces 2: ok\n");
    } else {
        printf("spaces 2: fail\n");
    }
    printf("\ncomplex:\n");
    if (testHandler("(   *    (* 4 5)  (* 4 5) )     ") == PARSE_SUCCESS) {
        printf("complex: ok\n");
    } else {
        printf("complex: fail\n");
    }
    if (testHandler("(   *    (* 4 5  (* 4 5) )     ") == PARSE_SUCCESS) {
        printf("complex 2: fail\n");
    } else {
        printf("complex 2: ok\n");
    }
    if (testHandler("(   *    * 4 5)  (* 4 5) )     ") == PARSE_SUCCESS) {
        printf("complex 3: fail\n");
    } else {
        printf("complex 3: ok\n");
    }
    if (testHandler("(   *    (* 45)  (* 4 5) )     ") == PARSE_SUCCESS) {
        printf("complex 4: fail\n");
    } else {
        printf("complex 4: ok\n");
    }
    if (testHandler("(*(/ 4 0) (* 4 5))") == PARSE_SUCCESS) {
        printf("zero division: fail\n");
    } else {
        printf("zero division: ok\n");
    }
    if (testHandler("(* -4 5)") == PARSE_SUCCESS) {
        printf("negative number: ok\n");
    } else {
        printf("negative number: fail\n");
    }
    if (testHandler("(* --4 5)") != PARSE_SUCCESS) {
        printf("wrong negative number: ok\n");
    } else {
        printf("wrong negative number: fail\n");
    }
    if (testHandler("(* - 4 5)") != PARSE_SUCCESS) {
        printf("wrong negative number 2: ok\n");
    } else {
        printf("wrong negative number 2: fail\n");
    }
    if (testHandler("(*(/ 4 4))HELLO") == PARSE_SUCCESS) {
        printf("trailing characters: fail\n");
    } else {
        printf("trailing characters: ok\n");
    }
}
