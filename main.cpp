#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

const char *SERVER = "127.0.0.1";
const unsigned int BUFFER_LENGTH = 516;
const unsigned int PORT = 69;

enum Opcode : unsigned short { RRQ = 1, WRQ = 2, DATA = 3, ACK = 4, ERROR = 5 };

enum Mode { NETASCII, OCTET };

typedef struct {
    Opcode opcode;
    const char *filename;
    Mode mode;
} Request;

void kys(const char *s) {
    perror(s);
    exit(1);
}

class TFTP {
  public:
    TFTP(const char *server, unsigned short port);
    ~TFTP();
    void send(const Request &request);
    void receive();

  private:
    unsigned int sock;
    struct sockaddr_in server;
};

TFTP::TFTP(const char *host, unsigned short port) {
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        kys("socket");
    }

    // Clear `server` struct.
    memset((char *)&server, 0, sizeof(server));
    server.sin_family = AF_INET; // IPv4
    server.sin_port = htons(port);

    // Decode IP address dot notation into binary form.
    if (inet_aton(host, &server.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
}

TFTP::~TFTP() { close(sock); }

void TFTP::send(const Request &request) {
    char message[BUFFER_LENGTH];

    if (request.opcode != RRQ && request.opcode != WRQ) {
        fprintf(stderr, "Invalid opcode\n");
        exit(1);
    }

    const char *mode;
    if (request.mode == NETASCII) {
        mode = "netascii";
    } else if (request.mode == OCTET) {
        mode = "octet";
    } else {
        fprintf(stderr, "Invalid mode\n");
        exit(1);
    }

    // Construct message.
    sprintf(
        message, "%c%c%s%c%s%c", (request.opcode >> 8) & 0xFF,
        request.opcode & 0xFF, request.filename, '\0', mode, '\0'
    );
    unsigned int message_length = strlen(request.filename) + strlen(mode) + 4;

    if (sendto(
            sock, message, message_length, 0, (struct sockaddr *)&server,
            sizeof(server)
        ) == -1) {
        kys("sendto()");
    }
}

void TFTP::receive() {
    char buf[BUFFER_LENGTH];
    socklen_t slen = sizeof(server);

    memset(buf, '\0', BUFFER_LENGTH);
    if (recvfrom(
            sock, buf, BUFFER_LENGTH, 0, (struct sockaddr *)&server, &slen
        ) == -1) {
        kys("recvfrom()");
    }

    puts(buf);
}

int main() {
    TFTP tftp(SERVER, PORT);
    Request request = {Opcode::RRQ, "/home/oliver/h.sh", Mode::NETASCII};

    tftp.send(request);
    tftp.receive();

    return 0;
}
