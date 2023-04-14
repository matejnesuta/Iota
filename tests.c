#include "tests.h"
#include <stdio.h>
#include <stdlib.h>
#include "main.h"

int testHandler(char* input) {
    int result;
    if (startParsing(input, &result)) {
        return PARSE_FAIL;
    } else {
        printf("The calculated result was: %d.\n", result);
        return PARSE_SUCCESS;
    }
}

void parserTests() {
    int return_code = 0;
    printf("\n//////// PARSER TESTS ////////\n");

    printf("\ninvalid inputs:\n\n");

    char invalid_inputs[13][50] = {"",
                                   "      ",
                                   "     (* 4 5)",
                                   "(+ 1)",
                                   "(   *    4   5   )     ",
                                   "(   +    (* 4 5)  (* 4 5) )     ",
                                   "(   *    (* 4 5  (* 4 5) )     ",
                                   "(   *    (* 45)  (* 4 5) )     ",
                                   "(*(/ 4 0) (* 4 5))",
                                   "(* -4 5)",
                                   "(* --4 5)",
                                   "(* - 4 5)",
                                   "(*(/ 4 4))HELLO"};

    for (int i = 0; i < 13; i++) {
        if (testHandler(invalid_inputs[i]) == PARSE_FAIL) {
            printf("\"%s\": OK\n\n", invalid_inputs[i]);
        } else {
            fprintf(stderr, "\"%s\": FAIL\n\n", invalid_inputs[i]);
            return_code = 1;
        }
    }

    printf("\nvalid inputs:\n\n");

    char valid_inputs[30][50] = {"(+ 2 2)",
                                 "(* 3 4)",
                                 "(/ 10 2)",
                                 "(- 8 5)",
                                 "(* 2 3 4)",
                                 "(- 7 3 1)",
                                 "(+ 5 (* 3 2))",
                                 "(/ 16 2 2)",
                                 "(+ 1 (/ 5 5))",
                                 "(- 10 (/ 20 2))",
                                 "(* 2 3 4 5)",
                                 "(/ 100 2 2 5)",
                                 "(+ 1 (* 2 3) (/ 4 2))",
                                 "(- 12 4 2 1)",
                                 "(* 1 2 3 4 5)",
                                 "(/ 1000 5 2 2 5)",
                                 "(+ 2 (* 3 4) (- 10 6))",
                                 "(- 100 50 20 10)",
                                 "(* 2 (+ 3 4) (- 5 1))",
                                 "(/ 200 (+ 100 50) (* 2 2))",
                                 "(* (- 4 2) (+ 3 4) (/ 10 2))",
                                 "(- (* 3 4) (* 5 2) (/ 12 3))",
                                 "(+ (* 2 3) (/ 8 2) (- 10 6))",
                                 "(* (- 5 2) (+ 1 2 3) (/ 18 3))",
                                 "(/ (- 10 6) (+ 1 2 3) (* 3 4))",
                                 "(- (* 2 3 4) (/ 16 2) (+ 5 4))",
                                 "(+ (/ 10 2) (* (- 3 4) (+ 1 2)))",
                                 "(* (+ 3 4 5) (- 8 6) (/ 20 4))",
                                 "(/ (* 2 3 4) (- 10 5) (+ 2 2 2))",
                                 "(- (/ 12 3) (* 2 3) (+ 7 1))"};

    for (int i = 0; i < 30; i++) {
        if (testHandler(valid_inputs[i]) != PARSE_FAIL) {
            printf("\"%s\": OK\n\n", valid_inputs[i]);
        } else {
            fprintf(stderr, "\"%s\": FAIL\n\n", valid_inputs[i]);
            return_code = 1;
        }
    }
    exit(return_code);
}
