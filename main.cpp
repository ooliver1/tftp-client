#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

const char *SERVER = "127.0.0.1";
const unsigned int BUFFER_LENGTH = 516;
const unsigned int PORT = 69;

enum class Opcode : unsigned short { RRQ = 1, WRQ = 2, DATA = 3, ACK = 4, ERROR = 5 };

enum Mode { NETASCII, OCTET };

void kys(const char *s) {
    perror(s);
    exit(1);
}

class Packet {
  public:
    virtual std::string serialize() = 0;
};

class Request : public Packet {
  protected:
    Mode mode;

    Request(Mode mode) : mode(mode) {}

    const char *getMode() {
        if (mode == NETASCII) {
            return "netascii";
        } else if (mode == OCTET) {
            return "octet";
        } else {
            fprintf(stderr, "Invalid mode\n");
            exit(1);
        }
    }
};

class RRQ : public Request {
  public:
    RRQ(const char *filename, Mode mode) : filename(filename), Request(mode) {}

    std::string serialize() {
        const char *modec = getMode();
        std::string message = {0x00};
        message += static_cast<char>(Opcode::RRQ);
        message += filename;
        message += '\0';
        message += modec;
        message += '\0';
        return message;
    }

  private:
    const char *filename;
};

class WRQ : public Request {
  public:
    WRQ(const char *filename, Mode mode) : filename(filename), Request(mode) {}

    std::string serialize() {
        const char *modec = getMode();
        std::string message = {0x00};
        message += static_cast<char>(Opcode::WRQ);
        message += filename;
        message += '\0';
        message += modec;
        message += '\0';
        return message;
    }

  private:
    const char *filename;
};

class TFTP {
  public:
    TFTP(char *host, unsigned short port);
    ~TFTP();
    void send(Packet &request);
    void receive();

  private:
    unsigned int sock;
    struct sockaddr_in server;
};

TFTP::TFTP(char *host, unsigned short port) {
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

void TFTP::send(Packet &request) {
    std::string message = request.serialize();
    unsigned int message_length = message.length();

    if (sendto(
            sock, message.c_str(), message_length, 0, (struct sockaddr *)&server,
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
    TFTP tftp((char *)SERVER, PORT);
    RRQ request("/home/oliver/h.sh", NETASCII);

    tftp.send(request);
    tftp.receive();

    return 0;
}
