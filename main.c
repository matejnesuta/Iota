#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// knihovny, ktere jsou specificke pro Linux.
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "main.h"
#include "tcp_mode.h"
#include "tests.h"
#include "udp_mode.h"

// Vetsina main.c byla prevzata z 1. projektu.

// Promenna pro socket serveru. Promenna je globalni, jelikoz v TCP rezimu
// zaviram master_socket pri SIGINT pred vsemi ostatnimi sockety.
int server_socket = 0;

void errprint(char* err) {
    fprintf(stderr, "%s", err);
}

// Zpracovavani vstupnich argumentu.
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

int main(int argc, char* argv[]) {
    // Pokud se projekt prelozi pomoci `make test`, misto serveru se spusti
    // unit testy nekterych funkci.
#ifdef TESTING
    unitTests();
#endif
    //  Nastavi se na 0 pri spusteni v UDP modu a na 1 v pripade TCP modu.
    int server_mode = -1;
    struct in_addr addr;  // This is set tu NULL due to the IP address
                          // validation
    long port_number = -1;
    argparse(argc, argv, &addr, &port_number, &server_mode);
    struct sockaddr_in server_address;

    /* Nastaveni vstupni adresy a portu.*/
    bzero((char*)&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = addr.s_addr;
    server_address.sin_port = htons((unsigned short)port_number);

    /* potlaceni defaultniho chovani rezervace portu ukonceni aplikace */
    int optval = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval,
               sizeof(int));
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEPORT, (const void*)&optval,
               sizeof(int));

    // Spusteni serveru bud v UDP, nebo TCP rezimu.
    if (server_mode == 1) {
        tcpMode(server_address);
    } else {
        udpMode(server_address);
    }
    return 0;
}
