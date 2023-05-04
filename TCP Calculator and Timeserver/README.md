# TCP Time Server and Calculator

This folder contains the completed solution for Assignment 1 of TCP Client-Server Programming. The assignment requires implementing two problems using TCP iterative servers and clients.

## Problem 1: Get System Date and Time

A TCP iterative server is implemented to allow client programs to get the system date and time from the server. When a client connects to the server, the server gets the local time on the machine and sends it to the client. The client displays the date and time on the screen, and terminates.

## Problem 2: Evaluate Arithmetic Expressions

A simple TCP iterative server and client are implemented to evaluate arithmetic expressions. The client-server system has the following functionalities:

- The client asks for an arithmetic expression from the user.
- The user enters an arithmetic expression containing real numbers, the binary operators +, -, *, and /, and left and right brackets (( and )). 
- The client sends the expression to the server. The arithmetic expression can be of any length.
- The server computes the expression and sends the result (a real number) back to the client.
- The client displays the result on the screen. It then prompts the user to enter the next expression.
- The client terminates when the user enters a –1.

The submitted files are as follows:

- `time_server.c` and `time_client.c` for Problem 1
- `server_it.c` and `client.c` for Problem 2

For more details, refer to the original assignment description in [Assgn-1.pdf](Assgn-1.pdf).

## How to Run the Code

1. Compile the server and client files using the following commands:
   - For Problem 1:
   
      ```
      gcc -o time_server time_server.c
      gcc -o time_client time_client.c
      ```

   - For Problem 2:
   
      ```
      gcc -o server_it server_it.c
      gcc -o client client.c
      ```

2. Start the server first by running the executable file, like this:

   ```
   ./time_server
   ```
   or 
   ```
   ./server_it
   ```

3. Start the client next by running the executable file with the port number of the server, like this:

   ```
   ./time_client <port-number>
   ```
   or
   ```
   ./client <port-number>
   ```

   Replace `<port-number>` with the port number on which the server is listening(20000 set as default).

   ```
   ./time_client 20000
   ```

   For more details, refer to the comments in the source code.