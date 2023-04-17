#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "parser.h"
#include "udp_mode.h"

// Socket serveru, ktera je definovana v main.c
extern int server_socket;

// Pomocna funkce pro seskladani error zpravy
void prepareUDPErrResponse(char (*buf)[UDP_BUFSIZE], char* message) {
    bzero(*buf, UDP_BUFSIZE);
    strcpy((*buf) + 3, message);
    *buf[0] = '\001';
    (*buf)[1] = '\001';
    (*buf)[2] = strlen((*buf) + 3);
    return;
}

// tato hlavni funkce byla vytvorena na tomto:
// https://git.fit.vutbr.cz/NESFIT/IPK-Projekty/src/branch/master/Stubs/cpp/DemoUdp/server.c
int udpMode(struct sockaddr_in server_address) {
    socklen_t clientlen;
    char buf[UDP_BUFSIZE];
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
    // Zde program bezi v nekonecne smycce, dokud ho nekdo neprerusi.

    while (1) {
        // Priprava bufferu pro prijeti zpravy.
        bzero(buf, UDP_BUFSIZE);
        clientlen = sizeof(client_address);
        bytesrx = recvfrom(server_socket, buf, UDP_BUFSIZE, 0,
                           (struct sockaddr*)&client_address, &clientlen);
        if (bytesrx < 0) {
            perror("ERROR: recvfrom:");
        }
        // kontrola spravneho opcode podle IPKCP
        if (buf[0] != '\0') {
            prepareUDPErrResponse(&buf, "Wrong opcode");
        } else {
            // oriznuti UDP zpravy podle delky v druhem bytu
            payload_length = buf[1];
            buf[payload_length + 2] = '\0';
            // parsovani vyrazu
            if (startParsing(buf + 2, &result) == PARSE_FAIL) {
                prepareUDPErrResponse(&buf,
                                      "Could not parse the message payload");
                // ackoliv UDP zaporne vysledky nezakazuje, pro konzistenci jsem
                // se rozhodl je nepovolit a podle fora predpokladam, ze se
                // tento pripad ani nebude testovat
            } else if (result < 0) {
                prepareUDPErrResponse(
                    &buf, "IPKCP does not support negative numbers as results");
            } else {
                // priprava bufferu pro odeslani zpravy pri uspesnem zpracovani
                bzero(buf, UDP_BUFSIZE);
                // vlozeni opcode a informace, jestli se jedna o OK zpravu
                buf[0] = '\1';
                buf[1] = '\0';
                // vlozeni samotneho vysledku
                sprintf(buf + 3, "%d", result);
                result_length = strlen(buf + 3);
                // pokud by vysledna zprava byla moc dlouha pro IPKCP, posle se
                // error (takovyto pripad ale momentalne nemuze nastat, protoze
                // parser pocita s integery a ty pretecou)
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