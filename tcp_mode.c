#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "main.h"
#include "parser.h"
#include "tcp_mode.h"

// Globalni promenne pro dynamicky buffer, sockety klientu a socket serveru.
// Dynamicke z toho duvodu, ze s nimi manipuluji v SIGINT handleru.
char* tcp_buffer;
int client_socket[MAX_CLIENTS];
extern int server_socket;

// Tyto heuristiky slouzi k urceni toho, zda-li jiz nejaky klient provedl
// handshake pomoci zpravy "HELLO". Pokud ano, je mu prirazeno cislo 2, pokud ne
// tak 1. 0 slouzi pro nepripojene a nebo odpojene klienty.
enum State { uninit = 0, init = 1, established = 2 };
int client_state[MAX_CLIENTS] = {0};

// Wrapper okolo Strcmp, ktery je case insensitive. Vraci ve skutecnosti jen 0
// nebo -1, takze se nechova jako skutecna funkce strcmp, ale pro muj use-case
// to staci.

// Podle fora nakonec zbytecny kus kodu, avsak ABNF notace naznacovala, ze
// prikazy HELLO a SOLVE nemusi byt case-sensitive.
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

// pomocna funkce pro ukonceni spojeni s klientem. V praxi bych musel mit
// volani send ve smycce, ale odpovedi serveru maji malou velikost, takze toto
// neresim a nebo jsem zatim nenarazil na scenar, kdy se toto mohlo projevit
// jako bug.
void closeTCPConnection(int sd, int* socket, int* state) {
    send(sd, "BYE\n", 4, 0);
    close(sd);
    *socket = 0;
    *state = 0;
}

// Sighandler. Nejdriv zavre master socket, aby se nemohl nekdo novy pripojit,
// pak odpoji vsechny klientske sockety, pak vycisti zbytek bufferu a ukonci
// cely program.
static void sig_handler(int _) {
    (void)_;
    close(server_socket);
    printf("Disconnecting all clients.\n");
    for (int i = 0; i < MAX_CLIENTS; i++) {
        send(client_socket[i], "BYE\n", 4, 0);
        close(client_socket[i]);
    }
    free(tcp_buffer);
    exit(0);
}

// Funkce na pripravu a odeslani TCP odpovedi v zavislosti na tom, jestli je
// format zpravy spravne a jestli prislo/neprislo HELLO.
void prepareTCPResponse(int client_index, int sd, int buffer_size) {
    // Potencialni vysledek
    int result = 0;
    // Rozvetveni podle toho, zda-li klient jiz provedl handshake, nebo ne.
    switch (client_state[client_index]) {
        case init:
            // zde se ocekava handshake. Pokud dojde cokoliv jineho, server
            // odpovi BYE a ukonci spojeni.
            if (caseInsensitiveStrcmp(tcp_buffer, "HELLO\n")) {
                closeTCPConnection(sd, &client_socket[client_index],
                                   &client_state[client_index]);
            } else {
                send(sd, "HELLO\n", 6, 0);
                client_state[client_index] = established;
            }
            break;
        case established:
            // tato funkce odrezava newline, ktery by mohl delat bordel v
            // parseru newline se zde nachazi vzdy, jelikoz tato funkce se
            // pousti po kazdem nalezeni newline
            tcp_buffer[buffer_size - 2] = 0;
            // Priprava na kontrolu headeru.
            char header[7];
            strncpy(header, tcp_buffer, 6);
            // Pokud selze parsovani, server ukonci spojeni.
            if (caseInsensitiveStrcmp(header, "SOLVE ") ||
                startParsing(tcp_buffer + 6, &result) == PARSE_FAIL) {
                closeTCPConnection(sd, &client_socket[client_index],
                                   &client_state[client_index]);
                // Pokud parsovani neselze, ale vysledek bude zaporny,
                // server take ukonci spojeni.
            } else if (result < 0) {
                closeTCPConnection(sd, &client_socket[client_index],
                                   &client_state[client_index]);
                // Jinak se vycisti buffer a do nej vlozi vysledek. Ten se posle
                // klientovi.
            } else {
                bzero(tcp_buffer, buffer_size);
                sprintf(tcp_buffer, "RESULT %d\n", result);
                send(sd, tcp_buffer, strlen(tcp_buffer), 0);
            }
            break;
    }
}

// Funkce zajistujici komunikaci serveru pomoci TCP. Pouzivam zde neblokujici
// I/O s pomoci selectu.
int tcpMode(struct sockaddr_in server_address) {
    // Tato cast byla vybudovana na tomto prikladu:
    // https://www.binarytides.com/multiple-socket-connections-fdset-select-linux/

    int addrlen, new_socket, activity, i, valread, sd;
    int max_sd;

    // alokace bufferu na jeden znak, abych nahodou v sigint handleru
    //  nedaval free na nealokovanou pamet.
    tcp_buffer = malloc(sizeof(char));

    // Socket descriptory.
    fd_set readfds;

    // Inicializace vsech klient socketu na 0.
    for (i = 0; i < MAX_CLIENTS; i++) {
        client_socket[i] = 0;
    }

    //  Vytvoreni master socketu slouziciho k zajisteni novych pripojeni.
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (bind(server_socket, (struct sockaddr*)&server_address,
             sizeof(server_address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    addrlen = sizeof(server_address);
    puts("Waiting for connections ...");

    // Inicializace sighandleru, jelikoz pote nasleduje hlavni smycka.
    struct sigaction sa = {.sa_handler = sig_handler};
    sigaction(SIGINT, &sa, 0);

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);
        max_sd = server_socket;

        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = client_socket[i];
            if (sd > 0)
                FD_SET(sd, &readfds);
            if (sd > max_sd)
                max_sd = sd;
        }
        // Cekani na aktivitu na nejakem socketu. Zde se ceka donekonecna,
        // jelikoz timeout je nastaven na NULL.
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            printf("select error");
        }

        // Kontrola, zda-li se neco stalo na master socketu a pokud ano, jedna
        // se o nove pripojeni.
        if (FD_ISSET(server_socket, &readfds)) {
            if ((new_socket =
                     accept(server_socket, (struct sockaddr*)&server_address,
                            (socklen_t*)&addrlen)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            // Debug print
            printf(
                "New connection , socket fd is %d , ip is : %s , port : %d \n",
                new_socket, inet_ntoa(server_address.sin_addr),
                ntohs(server_address.sin_port));

            // Prirazeni nove pripojeneho klienta mezi klientske sockety,
            // ktere jiz slouzi k funkcim kalkulacky.
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    client_state[i] = init;
                    printf("Adding to list of sockets as %d\n", i);
                    break;
                }
            }
        }

        // kontrola operaci na klientskych socketech
        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = client_socket[i];
            if (FD_ISSET(sd, &readfds)) {
                // buffer o 1 znaku slouzici pro cteni z read.
                char c;
                size_t buffer_size = 1;

                // Jelikoz u TCP neni zaruceno, ze 1 paket = 1 cela zprava
                // pro IPKCP, ctu ze socketu po znaku, dokud nenarazim na
                // newline. Az narazim na newline, tento buffer posilam ke
                // zpracovani funkci prepareTCPResponse.
                while (1) {
                    valread = read(sd, &c, 1);
                    // Pokud je navratova hodnota read 0, znamena to, ze je
                    // jiz klient odpojen a v tom pripade uzavreme jeho socket
                    // a nechame ho pro pouziti nekomu jinemu
                    if (valread == 0) {
                        // Close the socket and mark as 0 in list for reuse
                        closeTCPConnection(sd, &client_socket[i],
                                           &client_state[i]);
                        break;
                    }

                    // Jinak realokujeme dany buffer a cteme znak po znaku,
                    // dokud nenarazime na newline.
                    else {
                        buffer_size += 1;
                        tcp_buffer =
                            realloc(tcp_buffer, buffer_size * sizeof(char));
                        tcp_buffer[buffer_size - 2] = c;
                        tcp_buffer[buffer_size - 1] = 0;
                        if (c != '\n') {
                            continue;
                        } else {
                            prepareTCPResponse(i, sd, buffer_size);
                        }
                        break;
                    }
                }
            }
        }
    }
}