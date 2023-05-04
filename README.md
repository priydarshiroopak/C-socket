# Computer Networks (CS31204/ CS39006)

This repository contains solutions to six assignments related to C socket programming. Each assignment is contained within its respective folder, and each folder contains a README file that describes the assignment and its solution.

## [TCP Time Server and Calculator](TCP%20Calculator%20and%20Timeserver)

This folder contains the completed solution for Assignment 1 of TCP Client-Server Programming. The assignment requires implementing two problems using TCP iterative servers and clients: Get System Date and Time, and Evaluate Arithmetic Expressions.

## [TCP concurrent SHELL and UDP timeserver](TCP%20concurrent%20SHELL%20and%20UDP%20timeserver)

This repository contains the solutions for Assignment 2 of TCP and UDP Client-Server Programming. The assignment consists of two parts, each requiring the implementation of a client-server system: UDP Time Client-Server, and TCP Shell Client-Server.

## [Load Balancer over TCP](Load%20Balancer%20over%20TCP)

This Assigment implements a simplified version of a load balancer, which balances the load between two computation servers (S1 and S2) based on their current load. It also provides a service for clients to request the date and time.

## [Simple HTTP Client-Server](HTTP%20client-server)

This folder contains the submission for Assignment 4 - Simple HTTP Client-Server. In this assignment, we have implemented a simplified TCP concurrent Hyper Text Transfer Protocol (HTTP) client and server based on applicable RFCs.

## [MyTCP -A Message-Oriented TCP Protocol](MyTCP%20-Message%20oriented%20TCP)

This is a C library that implements a message-oriented TCP protocol called `MyTCP`. The `MyTCP` protocol guarantees that anything sent with a single `my_send` call will be received by a single `my_recv` call, similar to UDP but with reliability and in-order delivery like TCP.

## [PingnetInfo -Ping and Traceroute](Ping%20and%20TraceRoute)

`PingNetInfo` is a program that takes a site address and finds the route and estimate the latency and bandwidth of each link in the path. It probes by sending ICMP packets and takes three arguments: a site to probe, the number of times a probe will be sent per link, and the time difference between any two probes.

Each folder contains a README file with more detailed information on the specific assignment and how to run the code.