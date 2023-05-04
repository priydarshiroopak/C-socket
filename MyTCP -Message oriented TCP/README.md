# MyTCP - A Message-Oriented TCP Protocol

This is a C library that implements a message-oriented TCP protocol called `MyTCP`. The `MyTCP` protocol guarantees that anything sent with a single `my_send` call will be received by a single `my_recv` call, similar to UDP but with reliability and in-order delivery like TCP.

The `MyTCP` protocol is implemented by putting a layer of software between the `MyTCP` calls and the TCP socket calls. The library takes the calls of `MyTCP` and converts them to calls of TCP, along with additional bookkeeping that will be needed to ensure the message-oriented behavior.

## Functions
The following functions are implemented in the library:

- `my_socket` - This function opens a standard TCP socket, creates two threads R and S, allocates and initializes space for two tables `Send_Message` and `Received_Message`, and any additional space that may be needed.

- `my_bind` - Binds the socket with some address-port using the `bind` call.

- `my_listen` - Makes a `listen` call.

- `my_accept` - Accepts a connection on the `MyTCP` socket by making the `accept` call on the TCP socket (only on the server-side).

- `my_connect` - Opens a connection through the `MyTCP` socket by making the `connect` call on the TCP socket.

- `my_send` - Sends a message (see description later). One message is defined as what is sent in one `my_send` call.

- `my_recv` - Receives a message. This is a blocking call and will return only if a message, sent using the `my_send` call, is received. If the buffer length specified is less than the message length, the extra bytes in the message are discarded (similar to that in UDP).

- `my_close` - Closes the socket and cleans up everything. If there are any messages to be sent/received, they are discarded.

## Implementation Details
To ensure that a message is received as a whole on the other side, we open a standard TCP connection to actually communicate. This ensures the reliability, exactly once, and in-order delivery we want. However, this does not guarantee that whatever is sent using a `my_send` call on the `MyTCP` socket is sent and received in a single `send`/`recv` call on the underlying TCP socket. 

To solve this problem, we remember and send message boundaries and reconstruct the message on the receiving side even if it is not received in one go.

For each `MyTCP` socket, we need the following:

1. One TCP socket through which all actual communication actually happens.

2. Two threads R and S. Thread R handles all messages received from the TCP socket, and thread S, handles all sends done through the TCP socket.

3. Buffers, `Send_Message` and `Received_Message`, to store messages to send (given by the `my_send` call) and messages received (to be read by the `my_recv` call).

The threads are killed and the data structures freed when the `MyTCP` socket is closed. For simplicity, in this assignment, we assume that a program will create only one `MyTCP` socket, and the server is iterative. We also assume that the maximum length of a message can be 5000 bytes.

In addition to the main implementation, we have also provided a detailed documentation file, "documentation.txt", which describes the implementation details and design choices in more depth.

## How to Use

To use our MyTCP library, include the "mytcp.h" header file and link the "libmytcp.a" static library to your project. Then, use the provided functions `my_socket`, `my_bind`, `my_listen`, `my_accept`, `my_connect`, `my_send`, `my_recv`, and `my_close` as you would use the corresponding functions of the standard TCP socket library.

For more information on the usage of these functions, please refer to the documentation file.

## Submission Contents

The submission for this assignment contains the following files:

- `mytcp.h`: header file containing the function declarations for the MyTCP library
- `mytcp.c`: source code for the MyTCP library implementation
- `documentation.txt`: a detailed documentation file that describes the implementation details and design choices

The complete assignment description can be found in the [Assgn-3.pdf](Assgn-3.pdf) file.