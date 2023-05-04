# PingNetInfo
PingNetInfo is a program that takes a site address and finds the route and estimate the latency and bandwidth of each link in the path. It probes by sending ICMP packets and takes three arguments: a site to probe (can be either a name like cse.iitkgp.ac.in or an IP address like 10.3.45.6), the number of times a probe will be sent per link (n), and the time difference between any two probes (T).

## How it works

- If the site is given as a name, it first calls gethostbyname() to get an IP address corresponding to it.
- It then creates a raw socket to send and receive ICMP packets.
- It then goes ahead to find the path to the site given. This is similar to how traceroute tool works.
- To discover the intermediate nodes, it uses the same concept as in traceroute.
- It sends at least 5 ICMP packets with proper headers, each 1 second apart, per intermediate node before finalizing it.
- To estimate the latency and bandwidth of an intermediate link X-Y, it sends ping ICMP packets with different amounts of data in them to Y. For each size of data, send n pings, each T seconds apart. It uses the RTT measurements and the data sizes to estimate the latency and bandwidth.
- The program handles various possibilities, like any packet may be dropped, both requests and may reach their respective destinations out of order, more than one request may be on its way before response to any one of them comes back, and some intermediate servers will not respond to pings.
- For every ICMP packet sent and received, it prints out the header fields in a nice format. Additionally, for any ICMP packet received that is not a Echo Request/reply or Time Exceeded, it checks the data part to see if there is any data, and if so, prints out the IP header and the next level protocol header fields.

## Usage
Compile the pingnetinfo.c file using the following command:
```
gcc pingnetinfo.c -o pingnetinfo
```
Run the program using the following command:
```
./pingnetinfo <site-to-probe> <number-of-probes> <time-difference-between-probes>
```
For example:
```
./pingnetinfo www.google.com 5 1
```
This will probe the site www.google.com, send 5 probes per link, and with a time difference of 1 second between each probe.

The complete assignment description can be found in the [Assgn-3.pdf](Assgn-3.pdf) file.