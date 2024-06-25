#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

const unsigned int BUFFER_LENGTH = 512;
const unsigned int PORT = 8888;

void kys(char const *s) {
    perror(s);
    exit(1);
}

int main(void) {
    struct sockaddr_in server, client;
    unsigned int sock, recv_len;
    socklen_t slen = sizeof(client);
    char buf[BUFFER_LENGTH];

    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        kys("socket");
    }

    memset((char *)&server, 0, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) == -1) {
        kys("bind");
    }

    while (1) {
        printf("Waiting for data...");
        fflush(stdout);

        if ((recv_len = recvfrom(
                 sock, buf, BUFFER_LENGTH, 0, (struct sockaddr *)&client, &slen
             )) == -1) {
            kys("recvfrom()");
        }

        printf(
            "Received packet from %s:%d\n", inet_ntoa(client.sin_addr),
            ntohs(client.sin_port)
        );
        printf("Data: %s\n", buf);

        if (sendto(sock, buf, recv_len, 0, (struct sockaddr *)&client, slen) ==
            -1) {
            kys("sendto()");
        }
    }

    close(sock);
    return 0;
}