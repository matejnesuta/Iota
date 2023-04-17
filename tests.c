#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "parser.h"
#include "tcp_mode.h"
#include "tests.h"

// Par unit testu, ktere jsem si napsal predevsim na parser. Jsou spusteny
// prikazem `make test`, ktery spusti podminenou kompilaci.

// Parser znici puvodni vstupni string. Proto pouzivam tento wrapper, abych
// neprisel o testovaci vstupni data a mohl je vytisknout na stdout.
int testHandler(char* input, int* result) {
    if (startParsing(input, result)) {
        return PARSE_FAIL;
    } else {
        return PARSE_SUCCESS;
    }
}

void unitTests() {
    int result = 0;
    int return_code = 0;
    printf("\n//////// PARSER TESTS ////////\n");

    // Vstupy, ktere by parserem nemely projit.

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
        if (testHandler(invalid_inputs[i], &result) == PARSE_FAIL) {
            printf("\"%s\": OK\n\n", invalid_inputs[i]);
        } else {
            fprintf(stderr, "\"%s\": FAIL\n\n", invalid_inputs[i]);
            return_code = 1;
        }
    }

    printf("\nvalid inputs:\n\n");

    // Vstupy, ktere by parserem mely projit. Vysledky nekterych vstupu mohou
    // byt zaporne. Je to dane tim, ze parser samotny nekontroluje zaporna cisla
    // ve vysledcich.
    int results[30] = {4,   12, 5,  3,   24,  3,  11, 4,   2,  0,
                       120, 5,  9,  5,   120, 10, 18, 20,  56, 0,
                       70,  -2, 14, 108, 0,   7,  2,  120, 0,  -10};
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
        if (testHandler(valid_inputs[i], &result) != PARSE_FAIL) {
            if (result == results[i]) {
                printf(
                    "\"%s\": OK (expected result: %d   actual result %d)\n\n",
                    valid_inputs[i], results[i], result);
            } else {
                printf(
                    "\"%s\": FAIL   (expected result: %d   actual result "
                    "%d)\n\n",
                    valid_inputs[i], results[i], result);
            }
        } else {
            fprintf(stderr, "\"%s\":PARSE FAIL\n\n", valid_inputs[i]);
            return_code = 1;
        }
    }

    // Rychly check meho wrapperu okolu strcmp funkce, ktery pouzivam v TCP
    // rezimu.
    printf("\ncase insensitive strcmp:\n\n");

    char caseInSensInputs[4][4] = {"ab", "Ab", "aB", "AB"};
    char caseInSensDesc[4][4] = {"ab", "Ab", "aB", "AB"};

    for (int i = 0; i < 4; i++) {
        if (caseInsensitiveStrcmp(caseInSensInputs[i], "AB") != PARSE_FAIL) {
            printf("\"%s == AB\": OK\n\n", caseInSensDesc[i]);
        } else {
            fprintf(stderr, "\"%s == AB\": FAIL\n\n", caseInSensDesc[i]);
            return_code = 1;
        }
    }

    exit(return_code);
}
