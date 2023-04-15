#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "main.h"
#include "tests.h"

#define MAX_CLIENTS 30

#define BUFSIZE 1024
// sources:
// https://www.geeksforgeeks.org/recursive-descent-parser/?ref=lbp
// https://linux.die.net/man/3/inet_aton
// https://www.binarytides.com/multiple-socket-connections-fdset-select-linux/

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

// TODO: rename to terminal
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

int nextExpNonterminal(char** cursor, int* result, char operator) {
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

    if (nextExpNonterminal(cursor, result, operator)) {
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
    if (nextExpNonterminal(cursor, result, operator)) {
        return PARSE_FAIL;
    }
    return PARSE_SUCCESS;
}

void errprint(char* err) {
    fprintf(stderr, "%s", err);
}

void argparse(int argc,
              char* argv[],
              struct in_addr* addr,
              long* port,
              int* server_mode) {
    if (argc != 7) {
        errprint("Wrong number of arguments. Exiting.\n");
        exit(1);
    }

    int addr_found = -1;

    for (int i = 1; i < argc; i++) {
        if (!strcmp("-h", argv[i]) && addr_found == -1) {
            i++;
            addr_found = 0;
            if (inet_aton(argv[i], addr) == 0) {
                errprint("Wrong IP address. Exiting.\n");
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
        } else if (!strcmp("-m", argv[i]) && *server_mode == -1) {
            i++;
            if (!strcmp("tcp", argv[i])) {
                *server_mode = 1;
            } else if (!strcmp("udp", argv[i])) {
                *server_mode = 0;
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

int caseInsensitiveStrcmp(char* string1, char* string2) {
    size_t length = strlen(string1);
    if (length != strlen(string2)) {
        return -1;
    } else {
        for (size_t x = 0; x < length; x++) {
            if (string1[x] >= 'a' && string1[x] <= 'z') {
                string1[x] -= 32;
            } else if (string2[x] >= 'a' && string2[x] <= 'z') {
                string2[x] -= 32;
            }
        }
        if (strcmp(string1, string2) != 0) {
            return -1;
        } else {
            return 0;
        }
    }
}

void prepareUDPErrResponse(char (*buf)[BUFSIZE], char* message) {
    bzero(*buf, BUFSIZE);
    strcpy((*buf) + 3, message);
    *buf[0] = '\001';
    (*buf)[1] = '\001';
    (*buf)[2] = strlen((*buf) + 3);
    return;
}

void closeTCPConnection(int sd, int* socket, int* state) {
    send(sd, "BYE\n", 4, 0);
    close(sd);
    *socket = 0;
    *state = 0;
}

int udp_mode(struct sockaddr_in server_address, int server_socket) {
    socklen_t clientlen;
    char buf[BUFSIZE];
    struct sockaddr_in client_address;
    int bytestx, bytesrx;

    /* Vytvoreni soketu */
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) <= 0) {
        perror("ERROR: socket");
        exit(EXIT_FAILURE);
    }

    if (bind(server_socket, (struct sockaddr*)&server_address,
             sizeof(server_address)) < 0) {
        perror("ERROR: binding");
        exit(EXIT_FAILURE);
    }

    int payload_length = 0;
    int result = 0;
    int result_length = 0;

    while (1) {
        printf("INFO: Ready.\n");
        bzero(buf, BUFSIZE);
        clientlen = sizeof(client_address);
        bytesrx = recvfrom(server_socket, buf, BUFSIZE, 0,
                           (struct sockaddr*)&client_address, &clientlen);
        if (bytesrx < 0) {
            perror("ERROR: recvfrom:");
        }

        if (buf[0] != '\0') {
            prepareUDPErrResponse(&buf, "Wrong opcode");
        } else {
            payload_length = buf[1];
            buf[payload_length + 2] = '\0';
            if (startParsing(buf + 2, &result) == PARSE_FAIL) {
                prepareUDPErrResponse(&buf,
                                      "Could not parse the message payload");
            } else if (result < 0) {
                prepareUDPErrResponse(
                    &buf, "IPKCP does not support negative numbers as results");
            } else {
                bzero(buf, BUFSIZE);
                buf[0] = '\1';
                buf[1] = '\0';
                sprintf(buf + 3, "%d", result);
                result_length = strlen(buf + 3);
                if (result_length > 255) {
                    prepareUDPErrResponse(&buf,
                                          "result was too long to be sent");
                }
                buf[2] = strlen(buf + 3);
            }
        }
        /* odeslani zpravy zpet klientovi  */
        bytestx = sendto(server_socket, buf, strlen(buf + 3) + 3, 0,
                         (struct sockaddr*)&client_address, clientlen);
        if (bytestx < 0) {
            perror("ERROR: sendto:");
        }
    }
}

int tcp_mode(struct sockaddr_in server_address, int server_socket) {
    int addrlen, new_socket, client_socket[30], activity, i, valread, sd;
    int max_sd;

    int client_state[MAX_CLIENTS] = {0};
    enum State { uninit = 0, init = 1, established = 2 };

    // set of socket descriptors
    fd_set readfds;
    // initialise all client_socket[] to 0 so not checked

    for (i = 0; i < MAX_CLIENTS; i++) {
        client_socket[i] = 0;
    }

    // create a master socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // bind the socket to localhost port 8888
    if (bind(server_socket, (struct sockaddr*)&server_address,
             sizeof(server_address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // try to specify maximum of 3 pending connections for the master socket
    if (listen(server_socket, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // accept the incoming connection
    addrlen = sizeof(server_address);
    puts("Waiting for connections ...");

    while (1) {
        // clear the socket set
        FD_ZERO(&readfds);

        // add master socket to set
        FD_SET(server_socket, &readfds);
        max_sd = server_socket;

        // add child sockets to set
        for (i = 0; i < MAX_CLIENTS; i++) {
            // socket descriptor
            sd = client_socket[i];

            // if valid socket descriptor then add to read list
            if (sd > 0)
                FD_SET(sd, &readfds);

            // highest file descriptor number, need it for the select function
            if (sd > max_sd)
                max_sd = sd;
        }

        // wait for an activity on one of the sockets , timeout is NULL , so
        // wait indefinitely
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            printf("select error");
        }

        // If something happened on the master socket , then its an incoming
        // connection
        if (FD_ISSET(server_socket, &readfds)) {
            if ((new_socket =
                     accept(server_socket, (struct sockaddr*)&server_address,
                            (socklen_t*)&addrlen)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            // inform user of socket number - used in send and receive commands
            printf(
                "New connection , socket fd is %d , ip is : %s , port : %d \n",
                new_socket, inet_ntoa(server_address.sin_addr),
                ntohs(server_address.sin_port));

            // add new socket to array of sockets
            for (i = 0; i < MAX_CLIENTS; i++) {
                // if position is empty
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    client_state[i] = init;
                    printf("Adding to list of sockets as %d\n", i);

                    break;
                }
            }
        }

        // else its some IO operation on some other socket :)
        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = client_socket[i];

            if (FD_ISSET(sd, &readfds)) {
                // Check if it was for closing , and also read the incoming
                // message
                char c[1];
                char* buffer = malloc(sizeof(char));  // data buffer of 1K
                size_t buffer_size = 1;
                while (1) {
                    valread = read(sd, c, 1);
                    if (valread == 0) {
                        // Somebody disconnected , get his details and print
                        getpeername(sd, (struct sockaddr*)&server_address,
                                    (socklen_t*)&addrlen);
                        printf("Host disconnected , ip %s , port %d \n",
                               inet_ntoa(server_address.sin_addr),
                               ntohs(server_address.sin_port));

                        // Close the socket and mark as 0 in list for reuse
                        closeTCPConnection(sd, &client_socket[i],
                                           &client_state[i]);
                        break;
                    }

                    // Echo back the message that came in
                    else {
                        buffer_size += 1;
                        buffer = realloc(buffer, buffer_size * sizeof(char));
                        buffer[buffer_size - 2] = *c;
                        buffer[buffer_size - 1] = 0;
                        if (*c != '\n') {
                            continue;
                        } else {
                            int result = 0;
                            switch (client_state[i]) {
                                case init:
                                    if (caseInsensitiveStrcmp(buffer,
                                                              "HELLO\n")) {
                                        closeTCPConnection(sd,
                                                           &client_socket[i],
                                                           &client_state[i]);
                                    } else {
                                        send(sd, "HELLO\n", 6, 0);
                                        client_state[i] = established;
                                    }
                                    break;
                                case established:
                                    buffer[buffer_size - 2] = 0;
                                    char header[7];
                                    strncpy(header, buffer, 6);
                                    if (caseInsensitiveStrcmp(header,
                                                              "SOLVE ") ||
                                        startParsing(buffer + 6, &result) ==
                                            PARSE_FAIL) {
                                        closeTCPConnection(sd,
                                                           &client_socket[i],
                                                           &client_state[i]);
                                    } else if (result < 0) {
                                        closeTCPConnection(sd,
                                                           &client_socket[i],
                                                           &client_state[i]);
                                    } else {
                                        bzero(buffer, buffer_size);
                                        sprintf(buffer, "RESULT %d\n", result);
                                        send(sd, buffer, strlen(buffer), 0);
                                    }
                                    break;
                            }
                        }
                        free(buffer);
                        break;
                    }
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {
#ifdef TESTING
    parserTests();
#endif
    // This is set to 0 when UDP mode or to 1 when the TCP mode is used.
    int server_mode = -1;
    struct in_addr addr;  // This is set tu NULL due to the IP address
                          // validation
    long port_number = -1;
    int optval;
    argparse(argc, argv, &addr, &port_number, &server_mode);
    struct sockaddr_in server_address;
    int server_socket = 0;

    /* adresa serveru, potrebuje pro prirazeni pozadovaneho portu */
    bzero((char*)&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = addr.s_addr;
    server_address.sin_port = htons((unsigned short)port_number);

    /* potlaceni defaultniho chovani rezervace portu ukonceni aplikace */
    optval = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval,
               sizeof(int));

    if (server_mode == 1) {
        tcp_mode(server_address, server_socket);
    } else {
        udp_mode(server_address, server_socket);
    }

    return 0;
}