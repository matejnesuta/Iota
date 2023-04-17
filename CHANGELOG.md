
# Change Log
- Edited README.md, created CHANGELOG.md and fixed a few comments in the code.
- Modularization of the whole code. Comments and Python tests added.
- Added SIGINT handler for TCP mode.
- TCP mode working with the exception of signal handler. Unit tests added.
- Main function refactor, fixed some UDP mode edge cases and added a TCP mode boilerplate.
- Expression parser fixes and another refactor of the tests.
- UDP mode fully working and more tests added.
- Edited parser tests and moved them to a separate file.
- Argument parsing, expression parsing, Makefile, parser tests.


 
# Known limitations
- You cannot choose hostnames in the host parameter when running the server.
- Windows OS is not supported.
- ./ipkcpd can be run with arguments in any order, but all arguments must be specified (no default values are present).
- Server prints little or no output on it's stdout.
- The TCP mode currently only supports 50 clients at once. This can be easily changed in the tcp_mode.h file.
- I have not tested a scenario when more than 50 clients in TCP mode would try to connect, so I have no clue about what might happen.
- The server in TCP mode reads with the read function per one character. This might possibly slow down the server. 
- There is also no timeout mechanism, so the only way to disconnect from a server in TCP mode is to either close down the client, or wait for the server to be killed.
- There is no maximal buffer in the TCP mode, so it might be teoretically possible to segfault the server if huge amounts of data are sent with no newline.
- The parser works only with integers, so there also is a chance of overflowing when inputing huge numbers.
- There is commented out test in the tcp_tests.py. This test was supposed to check if graceful exitting works or not. The test is flaky however, so that is the reason I commented it out. 
- The UDP mode does return negative numbers, even though the IPKCP does not technically forbid it.
- The TCP mode is case-insensitive, meaning that something like *hElLo* is a completely valid hand-shake.
