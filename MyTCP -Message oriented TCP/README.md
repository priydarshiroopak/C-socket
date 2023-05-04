# Message Oriented TCP

This is a submission for CS 39006: Assignment 5 - Message Oriented TCP. The project implements a message-oriented TCP protocol using the standard TCP connection below it. The project implements a new socket type, called MyTCP (SOCK_MyTCP) socket, that guarantees that anything sent with a single send call will be received by a single receive call.

## Project Overview

The project consists of a set of function calls that implement MyTCP sockets. The MyTCP socket is implemented underneath by a TCP socket through which all communication will actually happen. The functions are implemented as a library. Any user wishing to use MyTCP sockets will write a C program that will call these functions in the same sequence as when using TCP sockets.

The following are the functions that have been implemented:

* my_socket – This function opens a standard TCP socket with the socket call. It also creates two threads R and S, allocates and initializes space for two tables Send_Message and Received_Message, and any additional space that may be needed. The parameters to these are the same as the normal socket( ) call, except that it will take only SOCK_MyTCP as the socket type.
* my_bind – binds the socket with some address-port using the bind call.
* my_listen – makes a listen call.
* my_accept – accepts a connection on the MyTCP socket by making the accept call on the TCP socket (only on server side)
* my_connect – opens a connection through the MyTCP socket by making the connect call on the TCP socket
* my_send – sends a message. One message is defined as what is sent in one my_send call.
* my_recv – receives a message. This is a blocking call and will return only if a message, sent using the my_send call, is received. If the buffer length specified is less than the message length, the extra bytes in the message are discarded (similar to that in UDP).
* my_close – closes the socket and cleans up everything. If there are any messages to be sent/received, they are discarded.

## Implementation Details

To ensure that a message is received as a whole on the other side, a standard TCP connection is opened to actually communicate. This ensures the reliability, exactly once, and in-order delivery. However, this does not guarantee that whatever is sent using a my_send call on the MyTCP socket is sent and received in a single send/recv call on the underlying TCP socket. Therefore, it was necessary to remember and send message boundaries and reconstruct the message on the receiving side even if it is not received in one go.

Each MyTCP socket requires the following:

1. One TCP socket through which all actual communication actually happen.
2. Two threads R and S. Thread R handles all messages received from the TCP socket, and thread S, handles all sends done through the TCP socket.
3. Buffers, Send_Message and Received_Message, to store messages to send (given by the my_send call) and messages received (to be read by the my_recv call.

The threads are killed, and the data structures freed when the MyTCP socket is closed. For simplicity, in this assignment, it was assumed that a program will create only one MyTCP socket, and the server is iterative. It was also assumed that the maximum length of a message can be 5000 bytes.

The thread R waits on a recv call on the TCP socket, receives data that comes in, and interprets the data to form the message (the complete data for which may come in multiple recv calls), and puts the message in the Received_Message table.

The thread S sleeps for some time (T) and wakes up periodically. On waking up, it sees if any message is waiting to be sent in the Send