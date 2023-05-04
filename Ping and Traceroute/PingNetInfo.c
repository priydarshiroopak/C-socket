#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define MAX_BUF_SIZE 5000
#define MSG1_SIZE 1024
#define MSG2_SIZE 4096
#define MAX_TRY 3

clock_t start_time, end_time;

unsigned short in_cksum(unsigned short *ptr, int nbytes);
char *receive_packet(int sockfd, char *finalIP);
void send_packet(int sockfd, char *ip_addr, char *data, int packSize, int ttl);

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Invalid arguments. Usage: sudo %s <host> <n> <T>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[2]);    // n = no. of times a probe will be sent
    int T = atoi(argv[3]);    // T = timedifference b/w two probes
    struct hostent *hent = gethostbyname(argv[1]);
    char *hostIP = (char *)malloc(sizeof(char) * INET_ADDRSTRLEN);
    strcpy(hostIP, inet_ntoa(*(struct in_addr *)hent->h_addr_list[0]));

    char *nodeIP = (char *)malloc(sizeof(char) * INET_ADDRSTRLEN);
    char *finalIP = (char *)malloc(sizeof(char) * INET_ADDRSTRLEN);

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        // perror("socket");
        return 1;
    }

    int optval = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &optval, sizeof(optval)) < 0) {
        // perror("setsockopt() error");
        exit(EXIT_FAILURE);
    }

    char *msg = (char *)malloc(sizeof(char) * MSG1_SIZE);
    memset(msg, 0, MSG1_SIZE);
    double prevLat = 0;
    int tryCount;

    for (int i = 1; i < 255; i++) {
        printf("  %d", i);
        fflush(stdout);
        send_packet(sockfd, hostIP, NULL, 0, i);
        tryCount = -1;
        while (!receive_packet(sockfd, finalIP) && ++tryCount < MAX_TRY) {
            printf("  *");
            fflush(stdout);
        }    // will try to receive packet for 3 times
        if (tryCount == MAX_TRY) {
            perror("\nrecv error");
            continue;
        }
        for (int j = 0; j < 4; j++) {
            send_packet(sockfd, hostIP, NULL, 0, i);
            // printf("Packet Sent!\n");
            tryCount = -1;
            while (!receive_packet(sockfd, nodeIP) && ++tryCount < MAX_TRY) {
                printf("  *");
                fflush(stdout);
            }    // will try to receive packet for 3 times
            if (tryCount == MAX_TRY) {
                perror("\nrecv error");
                break;
            }
            if (strcmp(nodeIP, finalIP) != 0)    // repeat if the IP comes out to be different
            {
                send_packet(sockfd, hostIP, NULL, 0, i);
                // printf("Packet Sent!\n");
                tryCount = -1;
                while (!receive_packet(sockfd, finalIP) && ++tryCount < MAX_TRY) {
                    printf("  *");
                    fflush(stdout);
                }    // will try to receive packet for 3 times
                if (tryCount == MAX_TRY) {
                    perror("\nrecv error");
                    break;
                }
                j = -1;
            }
        }
        if (tryCount == MAX_TRY)
            continue;

        printf("  %s", finalIP);

        // sending n empty packets i.e msg = NULL
        double avgLat = 0, lat;
        double band;
        for (int j = 0; j < n; j++) {
            send_packet(sockfd, hostIP, NULL, 0, i);
            tryCount = -1;
            while (!receive_packet(sockfd, finalIP) && ++tryCount < MAX_TRY) {
                printf("  *");
                fflush(stdout);
            }    // will try to receive packet for 3 times
            if (tryCount == MAX_TRY) {
                perror("\nrecv error");
                break;
            }
            lat = ((end_time - start_time) / (double)CLOCKS_PER_SEC) * 1000;
            avgLat += lat;
            // sending packets with data
            send_packet(sockfd, hostIP, msg, MSG1_SIZE, i);
            tryCount = -1;
            while (!receive_packet(sockfd, finalIP) && ++tryCount < MAX_TRY) {
                printf("  *");
                fflush(stdout);
            }    // will try to receive packet for 3 times
            if (tryCount == MAX_TRY) {
                perror("\nrecv error");
                break;
            }
            band = ((end_time - start_time) / (double)CLOCKS_PER_SEC) * 1000;
            band -= lat;      // subtracting the latency to find bandwidth
            if (band == 0)    // if RTT is 0, then set it to 0.0001
                band = 0.0001;
            band = MSG1_SIZE * 1000 / band / 1024 / 1024;    // converting to bits per second

            lat -= prevLat;    // subtracting the latency till previous node to find link latency
            printf("\t%.4lfms\t%.4lfMBps\t", lat, band);
            // sleep(T);
        }
        printf("\n");
        if (tryCount == MAX_TRY)
            continue;
        avgLat /= n;
        prevLat = avgLat;

        if (strcmp(nodeIP, hostIP) == 0)
            break;
    }
}

void setICMP(char *buf, char *data, int dataSize, int size) {
    struct icmphdr *icmp_hdr = (struct icmphdr *)buf;
    icmp_hdr->type = ICMP_ECHO;
    icmp_hdr->code = 0;
    icmp_hdr->un.echo.id = getpid();
    icmp_hdr->un.echo.sequence = 1;
    icmp_hdr->checksum = 0;
    memcpy(buf + sizeof(struct icmphdr), data, dataSize);

    icmp_hdr->checksum = in_cksum((unsigned short *)icmp_hdr, size);    // 20 is ip header size
}

void setIP(struct iphdr *ip_hdr, int packSize, struct sockaddr_in *dest_addr, int ttl) {
    ip_hdr->ihl = sizeof(struct iphdr) / 4;
    ip_hdr->version = 4;
    ip_hdr->tos = 0;
    ip_hdr->tot_len = htons(packSize);
    ip_hdr->id = htons(0);
    ip_hdr->frag_off = htons(0);
    ip_hdr->ttl = ttl;
    ip_hdr->protocol = IPPROTO_ICMP;
    ip_hdr->check = 0;
    ip_hdr->saddr = INADDR_ANY;
    ip_hdr->daddr = dest_addr->sin_addr.s_addr;

    ip_hdr->check = in_cksum((unsigned short *)ip_hdr, 4 * ip_hdr->ihl);
}

void send_packet(int sockfd, char *ip_addr, char *data, int size, int ttl) {
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    inet_aton(ip_addr, &dest_addr.sin_addr);

    int packSize = sizeof(struct iphdr) + sizeof(struct icmphdr) + size;

    char packet[packSize];
    struct iphdr *ip_hdr = (struct iphdr *)packet;
    setIP(ip_hdr, packSize, &dest_addr, ttl);

    setICMP(packet + 4 * ip_hdr->ihl, data, size, packSize - 4 * ip_hdr->ihl);

    if (sendto(sockfd, packet, packSize, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        // perror("sendto error");
        return;
    }

    // record start time
    start_time = clock();
    return;
}

char *receive_packet(int sockfd, char *finalIP) {
    char buf[MAX_BUF_SIZE];
    struct sockaddr_in src_addr;
    socklen_t src_addr_len = sizeof(src_addr);

    struct timeval time;
    time.tv_sec = 3;
    time.tv_usec = 0;

    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&time, sizeof(time));
    int bytes = recvfrom(sockfd, buf, MAX_BUF_SIZE, 0, (struct sockaddr *)&src_addr, &src_addr_len);
    if (bytes < 0) {
        // perror("recvfrom error");
        return NULL;
    }

    // record end time
    end_time = clock();

    struct iphdr *ip_hdr = (struct iphdr *)buf;
    struct icmphdr *icmp_hdr = (struct icmphdr *)(buf + (ip_hdr->ihl * 4));

    // char *data = buf + (ip_hdr->ihl * 4) + sizeof(struct icmphdr);
    // printf("data: %s", data);
    strcpy(finalIP, inet_ntoa(src_addr.sin_addr));

    return finalIP;
}

// function to calculate checksum
uint16_t in_cksum(uint16_t *addr, int len) {
    int nleft = len;
    uint32_t sum = 0;
    uint16_t *w = addr;
    uint16_t answer = 0;

    while (nleft > 1) {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft == 1) {
        *(unsigned char *)(&answer) = *(unsigned char *)w;
        sum += answer;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    return answer;
}