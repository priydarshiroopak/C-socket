# TCP and UDP Client-Server Programming Assignment

This repository contains the solutions for Assignment 2 of TCP and UDP Client-Server Programming. The assignment consists of two parts, each requiring the implementation of a client-server system. The detailed description of the assignment can be found in the file `Assgn-2.pdf`.

## Part 1 - UDP Time Client-Server

In the first part of the assignment, I implemented a UDP client-server system to make the time server I did in Assignment 1. The client sends a request for time to the server and waits for a maximum of 3 seconds for the result. If the time is not received within 3 seconds, the client tries again. The client tries a maximum of 5 times, after which if it still cannot get the time, it exits with an error message "Timeout exceeded". I used the poll() function to wait to receive on the UDP socket with timeout. 

The implementation of the UDP client-server system can be found in the files `timeserv.c` and `timeclient.c`.

## Part 2 - TCP Shell Client-Server

In the second part of the assignment, I implemented a TCP client-server system where the client and the server interact using a communication protocol as described in the assignment. The server sends the string `LOGIN:` to the client, and the client displays the string to the user. The user enters a username, which the client sends to the server. The server checks the username against a list of usernames present in a file named users.txt. If the given username occurs in the file, the server sends the string `FOUND` to the client, else it sends the string `NOT-FOUND` to the client. 

If the server has sent `NOT-FOUND`, the client prints an error message saying `Invalid username`, closes the connection, and exits. If the server has sent `FOUND`, the client asks the user for a shell command to be executed on the server side. The user enters a command from the keyboard, which the client sends to the server side for execution. The server executes the command and sends the results of the execution to the client. If the command is invalid, the server sends the special string `$$$$`. If the command is valid but there is an error in running it, the server returns the special string `####`.

The set of valid shell commands accepted by the server (along with the corresponding C library function that will give you the result) are `pwd` (getcwd), `dir` (opendir + readdir), `cd` (chdir) only which are available as C library functions. The server sends the result of the command execution to the client, which displays it on the screen. The client then asks the user for the next command to be executed. The client terminates when the user enters the command `exit`.

We ensured the following in our code:

1. The server is a concurrent TCP server.
2. No buffer of size > 50 is used in either client or server.
3. We send only the bytes for the strings specified, the commands (and their arguments if any), and the results and not send any extra bytes in either direction.
4. All commands, results, etc. are sent as null-terminated strings only.

The implementation of the TCP client-server system can be found in the files `sh_server.c` and `sh_client.c`.

## Files

- timeserv.c: The server implementation for the UDP server.
- timeclient.c: The client implementation for the UDP client.
- sh_server.c: The server implementation for the TCP shell concurent server.
- sh_client.c: The client implementation for the TCP shell client.

The complete assignment description can be found in the [Assgn-2.pdf](Assgn-2.pdf) file.