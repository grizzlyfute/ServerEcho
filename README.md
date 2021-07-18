# Server echo
This is a simple server echo. This server is unbuffer, and reply immediatly,
without waiting any end of line.
It is provided with a c client for demostration.

# Compilation
You need a Unix like system with a gcc

    gcc -O3 -Wall -Wextra client.c -o client
    gcc -O3 -Wall -Wextra tcp_echo.c -o server

# Usage

Run the server
    ./server 2050
Run the client
    ./client localhost 2050

