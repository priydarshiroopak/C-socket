# Load Balancer over TCP

This Assigment implements a simplified version of a load balancer, which balances the load between two computation servers (S1 and S2) based on their current load. It also provides a service for clients to request the date and time. 

## Functionality

- S1 and S2: Wait for a request from the load balancer (L). When a request to send the current load is received, the server generates a random integer between 1 and 100 as a dummy load and sends it to L. If it receives a client service request, it fulfils the service and sends the result back to L.

- L: Periodically (every 5 seconds) asks both S1 and S2 for their load by sending a message "Send Load". It stores the received load as the last known load of the servers in a table, and closes the connection. When it receives a load value, it prints "Load received from <server IP> <x>", where <server IP> is the IP address of S1 or S2 whose load is being asked, and <x> is the load value received. If a client requests a service, L finds the server with the lower load and sends the string "Send Time" to that server, and prints a message saying "Sending client request to <server IP>", where <server IP> is the IP of the server the request is sent to. The server sends the date and time back to L, which sends it to the client. L acts as a server to the clients and as a client to S1 and S2.

- Clients: Connect to L to request the date and time. When the connection is made, it waits to receive the date/time from L, after which it closes the connection.

## Usage

1. Compile the code using the following command:
```
gcc -o server server.c
gcc -o lb lb.c
gcc -o client client.c
```

2. Start the computation servers (S1 and S2) by running the following command:
```
./server <port_number>
```

3. Start the load balancer (L) by running the following command:
```
./lb <load_balancer_port_number> <server_1_port_number> <server_2_port_number>
```

4. Start the clients by running the following command:
```
./client <load_balancer_port_number> <port_number>
```

Note: The port number must be different for each instance of the servers/ clients. Pass the correct port numbers as command line arguments.

The complete assignment description can be found in the [Assgn-3.pdf](Assgn-3.pdf) file. The relevant submitted files are:
- server.c
- lb.c
- client.c