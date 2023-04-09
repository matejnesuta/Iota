#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "main.h"
#include "tests.c"

void cleanSpaces(char** cursor) {
    while (*cursor[0] == ' ') {
        (*cursor)++;
    }
    return;
}

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

int spaceNonterminal(char** cursor) {
    if (*cursor[0] != ' ') {
        return PARSE_FAIL;
    }
    (*cursor)++;
    cleanSpaces(cursor);
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

int exprNonterminal(char** cursor, int* result) {
    if (*cursor[0] == '(') {
        if (queryNonterminal(cursor, result)) {
            return PARSE_FAIL;
        } else {
            return PARSE_SUCCESS;
        }
    } else if ((*cursor[0] >= '0' && *cursor[0] <= '9') || *cursor[0] == '-') {
        *result = strtol(*cursor, cursor, 10);
        // printf("%d\n", number);
        return PARSE_SUCCESS;
    } else {
        return PARSE_FAIL;
    }
}

int queryNonterminal(char** cursor, int* result) {
    char operator;
    int operand1;
    int operand2;
    cleanSpaces(cursor);
    if (*cursor[0] != '(') {
        return PARSE_FAIL;
    }
    (*cursor)++;
    cleanSpaces(cursor);
    if (operatorNonterminal(cursor, &operator)) {
        return PARSE_FAIL;
    }
    if (spaceNonterminal(cursor)) {
        return PARSE_FAIL;
    }
    if (exprNonterminal(cursor, &operand1)) {
        return PARSE_FAIL;
    }
    if (spaceNonterminal(cursor)) {
        return PARSE_FAIL;
    }
    if (exprNonterminal(cursor, &operand2)) {
        return PARSE_FAIL;
    }
    cleanSpaces(cursor);
    if (*cursor[0] != ')') {
        return PARSE_FAIL;
    }
    (*cursor)++;
    if (evaluation(operator, operand1, operand2, result)) {
        return PARSE_FAIL;
    }
    return PARSE_SUCCESS;
}

void errprint(char* err) {
    fprintf(stderr, "%s", err);
}

void argparse(int argc,
              char* argv[],
              struct hostent** server,
              long* port,
              int* tcp_mode) {
    // TO BE DELETED (for testing purposes only)
    if (argc == 2) {
        if (!strcmp("tests", argv[1])) {
            parserTests();
            exit(0);
        }
    }

    if (argc != 7) {
        errprint("Wrong number of arguments. Exiting.\n");
        exit(1);
    }

    for (int i = 1; i < argc; i++) {
        if (!strcmp("-h", argv[i]) && *server == NULL) {
            i++;
            if ((*server = gethostbyname(argv[i])) == NULL) {
                errprint("Wrong host. Exiting.\n");
                exit(1);
            }
            continue;
        } else if (!strcmp("-p", argv[i]) && *port == -1) {
            i++;
            char* pEnd = NULL;
            *port = strtol(argv[i], &pEnd, 10);
            if (pEnd[0] != '\0' || *port < 0 || *port > 65535) {
                errprint("Wrong port number. Exiting.\n");
                exit(1);
            }
            continue;
        } else if (!strcmp("-m", argv[i]) && *tcp_mode == -1) {
            i++;
            if (!strcmp("tcp", argv[i])) {
                *tcp_mode = 1;
            } else if (!strcmp("udp", argv[i])) {
                *tcp_mode = 0;
            } else {
                errprint("Wrong mode. Exiting.\n");
                exit(1);
            }
            continue;
        } else {
            errprint("Wrong type of arguments. Exiting.\n");
            exit(1);
        }
    }
}

int main(int argc, char* argv[]) {
    // This is set to 0 when UDP mode or to 1 when the TCP mode is used.
    int tcp_mode = -1;
    struct hostent* server = NULL;  // This is set tu NULL due to the IP address
                                    // validation
    long port = -1;

    argparse(argc, argv, &server, &port, &tcp_mode);
    // struct sockaddr_in server_address;

    // /* Taken from the stub:
    //  * https://git.fit.vutbr.cz/NESFIT/IPK-Projekty/src/branch/master/Stubs/cpp/DemoTcp/client.c*/
    // bzero((char*)&server_address, sizeof(server_address));
    // server_address.sin_family = AF_INET;
    // bcopy((char*)server->h_addr, (char*)&server_address.sin_addr.s_addr,
    //       server->h_length);
    // server_address.sin_port = htons(port);

    // if (tcp_mode == 1) {
    // } else {
    // }
    return 0;
}