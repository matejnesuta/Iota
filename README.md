### IPK 2023 - Projekt 2
### Varianta IOTA - vzdálená kalkulačka

#### Matěj Nešuta - xnesut00
#### Finální hodnocení: 17.82/20 bodů
### Popis projektu

Cílem projektu je implementovat server pro vzdálenou kalkulačku výrazů ve variaci na prefixovou notaci,
která je daná protokolem IPKCP. Kalkulačka je implementovaná jako klient-server aplikace podporující komunikaci jak přes UDP, tak přes TCP. 

### Spuštění

Server se přeloží pomocí příkazu make
```console
$ make
```
Poté se dá server spustit pomocí příkazu
```console
$ ./ipkcpd [-h host] [-p port] [-m mode]
```
Kde:
- host - adresa, na které má server běžet
- port - číslo portu
- mode - režim komunikace (TCP nebo UDP)

Argumenty jsou podporovány v libovolném pořadí, ale musí být použity všechny 3, jelikož kalkulačka nemá nastavené defaultní hodnoty. \
Kalkulačka také není schopna přeložit název hostitele (např. localhost). Je tedy třeba zadávat vždy konkrétní adresu.

### Implementace 

Server je napsán v jazyce C pro platformu Linux. Jednotlivé moduly se vždy skládají z jednoho zdrojového a jednoho hlavičkového souboru.
- **main.{c, h}** - hlavní modul, který obsahuje hlavně funkci main a funkci na zpracování argumentů
- **parser.{c, h}** - modul obsahující funkce pro zpracování a evaluaci výrazů 
- **udp_mode.{c, h}** - modul obsahující funkce a hlavní event loop pro komunikaci serveru pomocí protokolu UDP
- **tcp_mode.{c, h}** - modul obsahující funkce, zpracování SIGINT signálu a hlavní event loop pro komunikaci serveru pomocí protokolu TCP
- **tests.{c, h}** - modul obsahující unit testy pro parser 
- **tcp_tests.py** - modul obsahující Python testy pro TCP modul
- **udp_tests.py** - modul obsahující Python testy pro UDP modul

Běh programu začíná ve funkci main, která inicializuje některé společné proměnné, zpracuje a zvaliduje vstupní argumenty a poté podle přepínače \
mode rozhodne, zda server poběží v UDP a nebo TCP režimu.

### Zpracování výrazů

Zpracování a evaluace výrazů probíhá metodou rekurzivního sestupu pomocí následujících pravidel gramatiky.\
Neterminály jsou značeny velkým písmem, terminály malým písmem a počáteční neterminál je neterminál S:

```
S -> QUERY eps
QUERY -> left_bracket OPERATOR space EXP space QUERY_END
OPERATOR -> plus
OPERATOR -> minus
OPERATOR -> multiplication
OPERATOR -> division
EXP -> QUERY
EXP -> number
QUERY_END -> space EXP END
QUERY_END -> right_bracket
```
Každý neterminál je pak podle této gramatiky přepsán do vlastní funkce. 
Tyto funkce se pak volají rekurzivně mezi sebou, předávají si mezi sebou vstupní řetězec, který postupně zpracovávají. \
Zároveň si mezi sebou předávají výsledek již provedených výpočtů pomocí ukazatele. \
K evaluaci výrazů dochází ve funkci pro zpracování neterminálu QUERY a v případě více \
než 2 operandů také ve funkci pro zpracování neterminálu QUERY_END.

V případě nedodržení pravidel gramatiky a nebo případného dělení nulou parser okamžitě vrací chybovou hodnotu. \
Parser nevrací chybovou hodnotu v případě konečného výsledku. Tuto věc kontrolují UDP a TCP režim separátně. Parser také počítá pouze s integery a u velmi velkých čísel hrozí přetečení.
### UDP

Při spuštění serveru v režimu UDP komunikace se vytvoří socket typu SOCK_DGRAM a poté se přejde do nekonečné smyčky, ze které se dá dostat pouze pomocí SIGINT signálu. V této smyčce se nejdříve vynuluje buffer pro přijmutí zprávy a poté se čeká pomocí funkce recvfrom.

Jakmile dojde zpráva, zkontroluje se nejdříve opcode. Poté se pomocí délky na druhém bytu ořeže zbytek zprávy na takto specifikovaný počet bytů a tento výraz je následně poslán pro validaci a případnou evaluaci parseru. Pokud následná kontrola dopadne dobře a výsledek nebude záporný, sestaví se odpověď podle IPKCP protokolu (binární varianta) a tato zpráva se pak odešle jako odpověď zpátky tomuto uživateli.

V případě, že se nějaká z těchto podmínek nedodrží, server uživateli odešle zprávu, která má status kód 1 a relevantní hlášku o tom, k jaké chybě na straně klienta došlo.

### TCP

Při spuštění serveru v režimu TCP komunikace se vytvoří master socket typu SOCK_STREAM, klientské sockety pro udržování spojení, buffer pro načítání zprávy, SIGINT handler a také socket descriptory. Poté se již přejde do nekonečné smyčky, ze které se dá dostat pouze posláním SIGINT signálu. 

Pro komunikaci více klientů současně se využívá neblokující komunikace, a to konkrétně select() multiplexing. Server nejdříve ve smyčce čeká, zda-li se neděje nějaká aktivita na nějakém ze socketů. Pokud zaznamená aktivitu na svém master socketu, znamená to, že se chce připojit nový klient, pro kterého se poté potvrdí připojení a najde volný klientský socket. Takto můžou čekat na připojení až 3 klienti naráz. 

Pokud server zaznamená aktivitu na nějakém z klientských socketů, vyhledá v cyklu pomocí makra FD_ISSET konkrétní socket a tento socket obslouží. Obsluhou se v tomto případě myslí 2 věci a to čtení dat od klienta a následná odpověď v podobě funkce send, nebo detekce odpojení klienta. Obojí se děje pomocí funkce read a její návratové hodnoty. 

Pokud funkce read vrátí nulu, znamená to, že se klient sám odpojil. V takovém případě přijde na řadu funkce closeTCPConnection, která zavře tento klientský socket a vynuluje ho, aby mohl být použit pro dalšího nového klienta. Stejná operace se zároveň děje i pokud zadá klient nějaký nevalidní vstup. Nevalidním vstupem se myslí buď výraz, který nesplňuje gramatiku parseru, nebo záporný výsledek výrazu, případně absence klíčových slov *HELLO* a *SOLVE*.

Server od klienta načítá vstup do svého dynamicky alokovaného bufferu po znaku až dokud nenarazí na newline znak nebo klient neukončí spojení. Důvod je takový, že nutně v jednom paketu nemusí přijít celá zpráva a zároveň může být v jednom paketu zpráv více. Proto server předpokládá, že má celou zprávu jen tehdy, pokud má přesně 1 newline znak. Tuto zprávu pak předá funkci prepareTCPResponse.

Funkce prepareTCPResponse rozhoduje, zda-li na základě návratových hodnot z parseru a správné syntaxe a sémantiky související s příkazy *HELLO* a *SOLVE* potvrdit klientovi handshake, vrátit výsledek vypočítaného výrazu a nebo případně klienta odpojit na nějaké chybě. K této činnosti je také k dispozici statické pole client_state, ve kterém je pro všechny klienty zaznamenáno, zda-li u nich proběhl handshake a nebo ne. Není tedy možné handshake vynechat, nebo handshake provádět vícekrát za 1 spojení.

V neposlední řadě má ještě tento soubor funkci pro zpracování signálu SIGINT. Pokud je takový signál poslán, server nejdříve zavře svůj master socket, aby nemohl přijímat nová spojení, poté pošle všem klientům zprávu *BYE\n*, poté zavře všechny klientské sockety a případně vyčistí buffer a v neposlední řadě se ukončí s návratovým kódem 0.

### Testování

- Testování bylo vykonané pomocí unit testů v souboru **tests.c** a pomocí menších Python skriptů **tcp_tests.py** a **udp_tests.py**.
- Složitější testy byly vykonány manuálně. Jde o testy více klientů u TCP a také o test signal handleru v režimu TCP.
- Veškeré testování kromě testů v Python skriptech bylo vykonáno na referenčním stroji, kterým je NixOS virtuální stroj. Klient běžel na lokálním stroji. Python skripty jsem spouštěl s verzí Python 3.11.2.

### Unit test cases

Tyto unit testy jsou v souboru tests.c a dají se spustit pomocí příkazu `make tests`. Výsledky některých validních vstupů jsou záporné, ale nepovažuji to za problém, jelikož parser samotný nekontroluje zápornost výsledku a tato práce se nechává až na modulech zajišťujících komunikaci buď pomocí UDP, nebo TCP. Tato funkcionalita je tedy testována pomocí již zmíněných Python skriptů.

#### Invalid parser inputs

| Input | Status |
|-------|--------|
| "" | OK |
| "      " | OK |
| "     (* 4 5)" | OK |
| "(+ 1)" | OK |
| "(   *    4   5   )     " | OK |
| "(   +    (* 4 5)  (* 4 5) )     " | OK |
| "(*(* 4 5(* 4 5))" | OK |
| "(*(* 45)(* 4 5) )" | OK |
| "(*(/ 4 0) (* 4 5))" | OK |
| "(* -4 5)" | OK |
| "(* --4 5)" | OK |
| "(* - 4 5)" | OK |
| "(*(/ 4 4))HELLO" | OK |

#### Valid parser inputs

| Input | Output | Status |
|-------|--------|--------|
|"(+ 2 2)"| expected result: 4   actual result: 4 | OK |
|"(* 3 4)"| expected result: 12   actual result: 12 | OK |
|"(/ 10 2)"| expected result: 5   actual result: 5 | OK |
|"(- 8 5)"| expected result: 3   actual result: 3 | OK |
|"(* 2 3 4)"| expected result: 24   actual result: 24 | OK |
|"(- 7 3 1)"| expected result: 3   actual result: 3 | OK |
|"(+ 5 (* 3 2))"| expected result: 11   actual result: 11 | OK |
|"(/ 16 2 2)"| expected result: 4   actual result: 4 | OK |
|"(+ 1 (/ 5 5))"| expected result: 2   actual result: 2 | OK |
|"(- 10 (/ 20 2))"| expected result: 0   actual result: 0 | OK |
|"(* 2 3 4 5)"| expected result: 120   actual result: 120 | OK |
|"(/ 100 2 2 5)"| expected result: 5   actual result: 5 | OK |
|"(+ 1 (* 2 3) (/ 4 2))"| expected result: 9   actual result: 9 | OK |
|"(- 12 4 2 1)"| expected result: 5   actual result: 5 | OK |
|"(* 1 2 3 4 5)"| expected result: 120   actual result: 120 | OK |
|"(/ 1000 5 2 2 5)"| expected result: 10   actual result: 10 | OK |
|"(+ 2 (* 3 4) (- 10 6))"| expected result: 18   actual result: 18 | OK |
|"(- 100 50 20 10)"| expected result: 20   actual result: 20 | OK |
|"(* 2 (+ 3 4) (- 5 1))"| expected result: 56   actual result: 56 | OK |
|"(/ 200 (+ 100 50) (* 2 2))"| expected result: 0   actual result: 0 | OK |
|"(* (- 4 2) (+ 3 4) (/ 10 2))"| expected result: 70   actual result: 70 | OK |
|"(- (* 3 4) (* 5 2) (/ 12 3))"| expected result: -2   actual result: -2 | OK |
|"(+ (* 2 3) (/ 8 2) (- 10 6))"| expected result: 14   actual result: 14 | OK |
|"(* (- 5 2) (+ 1 2 3) (/ 18 3))"| expected result: 108   actual result: 108 | OK |
|"(/ (- 10 6) (+ 1 2 3) (* 3 4))"| expected result: 0   actual result: 0 | OK |
|"(- (* 2 3 4) (/ 16 2) (+ 5 4))"| expected result: 7   actual result: 7 | OK |
|"(+ (/ 10 2) (* (- 3 4) (+ 1 2)))"| expected result: 2   actual result: 2 | OK |
|"(* (+ 3 4 5) (- 8 6) (/ 20 4))"| expected result: 120   actual result: 120 | OK |
|"(/ (* 2 3 4) (- 10 5) (+ 2 2 2))"| expected result: 0   actual result: 0 | OK |
|"(- (/ 12 3) (* 2 3) (+ 7 1))"| expected result: -10   actual result: -10 | OK |


### UDP tests
| Input | Output | Status |
|-------|--------| ------ |
| b"\x00\x07(* 4 5)" | b"\x01\x00\x0220" | OK|
| b"\x00\x07(- 4 5)" | b"\x01\x01\x32IPKCP does not support negative numbers as results" | OK |
| b"\x01\x07(* 4 5)" | b"\x01\x01\x0cWrong opcode" | OK |

### TCP tests
| Name | Input | Output | Status |
|------|-------|--------| ------ |
| Simple query | b"HELLO\nSOLVE (+ 1 2)\nBYE\n" | b"HELLO\nRESULT 3\nBYE\n" | OK |
| More commands in one query | b"HELLO\nSOLVE (+ 1 2)\nBYE\n" | b"HELLO\nRESULT 3\nBYE\n" | OK |
| Command in multiple queries | b"HELLO\nSOLVE (+ 1 2)\nBYE\n" | b"HELLO\nRESULT 3\nBYE\n" | OK |
| Negative result | b"HELLO\nSOLVE (- 1 2)\nBYE\n" | b"HELLO\nBYE\n" | OK |
| No hello | b"SOLVE (+ 1 2)\n" | b"BYE\n" | OK |
| Double hello | b"HELLO\nHELLO\n" | b"HELLO\nBYE\n" | OK |


## Zdroje
* Rekurzivní sestup: https://www.geeksforgeeks.org/recursive-descent-parser/
* Funkce inet_aton: https://linux.die.net/man/3/inet_aton
* Použití neblokující TCP komunikace pomocí selectu v C: https://www.binarytides.com/multiple-socket-connections-fdset-select-linux/
* Funkce close: https://pubs.opengroup.org/onlinepubs/009604499/functions/close.html
* Protokol IPKCP: https://git.fit.vutbr.cz/NESFIT/IPK-Projekty/src/branch/master/Project%201/Protocol.md
* Ukázka UDP serveru: https://git.fit.vutbr.cz/NESFIT/IPK-Projekty/src/branch/master/Stubs/cpp/DemoSelect/server.c
* Vysvětlení neblokující komunikace za pomocí select() multiplexování: https://www.gta.ufrj.br/ensino/eel878/sockets/advanced.html
