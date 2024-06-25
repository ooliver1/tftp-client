#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

const char *SERVER = "127.0.0.1";
const unsigned int BUFFER_LENGTH = 516;
const unsigned int PORT = 69;

typedef struct {
    unsigned short opcode;
    const char *filename;
    const char *mode;
} Request;

void kys(char const *s) {
    perror(s);
    exit(1);
}

int main(void) {
    struct sockaddr_in server;
    unsigned int sock;
    socklen_t slen = sizeof(server);
    char buf[BUFFER_LENGTH];
    char message[BUFFER_LENGTH];

    Request request = {1, "/home/oliver/h.sh", "netascii"};

    // Construct message.
    sprintf(
        message, "%c%c%s%c%s%c", (request.opcode >> 8) & 0xFF,
        request.opcode & 0xFF, request.filename, '\0', request.mode, '\0'
    );
    unsigned int message_length =
        strlen(request.filename) + strlen(request.mode) + 4;

    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        kys("socket");
    }

    // Clear `server` struct.
    memset((char *)&server, 0, sizeof(server));
    server.sin_family = AF_INET; // IPv4
    server.sin_port = htons(PORT);

    // Decode IP address dot notation into binary form.
    if (inet_aton(SERVER, &server.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    if (sendto(
            sock, message, message_length, 0, (struct sockaddr *)&server, slen
        ) == -1) {
        kys("sendto()");
    }

    memset(buf, '\0', BUFFER_LENGTH);
    if (recvfrom(
            sock, buf, BUFFER_LENGTH, 0, (struct sockaddr *)&server, &slen
        ) == -1) {
        kys("recvfrom()");
    }

    puts(buf);

    close(sock);
    return 0;
}
