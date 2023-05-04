# Simple HTTP Client-Server

This repository contains the submission for Assignment 4 - Simple HTTP Client-Server.

In this assignment, we have implemented a simplified TCP concurrent Hyper Text Transfer Protocol (HTTP) client and server. The client and server communicate with each other over a TCP connection, and the client can request resources from the server using HTTP GET requests, or upload resources to the server using HTTP PUT requests. The implementation follows the HTTP specification defined in the RFCs.

The complete assignment description can be found in the "Assgn-3.pdf" file included in this repository.

## Implemented Functionality

The implemented HTTP client and server support the following features:

### HTTP Client (MyOwnBrowser)

- Connects to the HTTP server over a TCP connection
- Sends HTTP GET requests to retrieve resources from the server
- Sends HTTP PUT requests to upload resources to the server
- Supports downloading/uploading of files
- Displays the retrieved resources on the client console

### HTTP Server (MyOwnHTTP)

- Listens for incoming connections from HTTP clients
- Supports handling of HTTP GET and PUT requests from clients
- Returns HTTP 200 OK responses for successful requests
- Returns HTTP 404 Not Found responses for non-existent resources
- Accepts files for upload

## How to Use

To use the HTTP client and server, follow the steps below:

### HTTP Client (MyOwnBrowser)

1. Compile the `MyOwnBrowser.c` file using the command `gcc MyOwnBrowser.c -o MyOwnBrowser`.
2. Run the compiled file using the command `./MyOwnBrowser <server-ip> <server-port>`.
   Replace `<server-ip>` with the IP address of the server and `<server-port>` with the port number on which the server is listening.
3. Use the following commands to interact with the HTTP server:
   - `GET <resource>` - retrieves the specified resource from the server
   - `PUT <resource>` - uploads the specified resource to the server
   - `EXIT` - closes the connection and exits the client

### HTTP Server (MyOwnHTTP)

1. Compile the `MyOwnHTTP.c` file using the command `gcc MyOwnHTTP.c -o MyOwnHTTP`.
2. Run the compiled file using the command `./MyOwnHTTP <port-number>`.
   Replace `<port-number>` with the port number on which the server should listen.
3. The server is now listening for incoming connections from HTTP clients and can handle HTTP GET and PUT requests.

## Submitted Files

The following files have been submitted as part of this assignment:

- `MyOwnBrowser.c`: contains the implementation of the HTTP client
- `MyOwnHTTP.c`: contains the implementation of the HTTP server
- `README.md`: contains the documentation for the repository

Please refer to the [Assgn-3.pdf](Assgn-3.pdf) file for the complete assignment description.