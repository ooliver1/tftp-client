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

enum class Opcode : unsigned short {
    RRQ = 1,
    WRQ = 2,
    DATA = 3,
    ACK = 4,
    ERROR = 5
};

enum Mode { NETASCII, OCTET };

void kys(const char *s) {
    perror(s);
    exit(1);
}

class Packet {
  public:
    virtual std::string serialize() = 0;
};

class Data : public Packet {
  public:
    Data(unsigned short block_number, char *data)
        : block_number(block_number), data(data) {}
    Data(char *raw) {
        block_number = raw[1];
        data = raw + 4;
    }

    std::string serialize() {
        std::string message = {0x00};
        message += static_cast<char>(Opcode::DATA);
        message += static_cast<char>(block_number);
        message += data;
        return message;
    }

    unsigned short getBlockNumber() { return block_number; }
    char *getData() {
        char *data_copy = new char[strlen(data) + 1];
        snprintf(data_copy, strlen(data) + 1, "%s", data);
        return data_copy;
    }

  private:
    unsigned short block_number;
    char *data;
};

class Ack : public Packet {
  public:
    Ack(unsigned short block_number) : block_number(block_number) {}
    Ack(char *raw) { block_number = raw[1]; }

    std::string serialize() {
        std::string message = {0x00};
        message += static_cast<char>(Opcode::ACK);
        message += static_cast<char>(block_number);
        return message;
    }

    unsigned short getBlockNumber() { return block_number; }

  private:
    unsigned short block_number;
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
    void read(char *filename, Mode mode);

  private:
    unsigned int sock;
    struct sockaddr_in server;
    void send(Packet &request);
    char *receive();
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
            sock, message.c_str(), message_length, 0,
            (struct sockaddr *)&server, sizeof(server)
        ) == -1) {
        kys("sendto()");
    }
}

char *TFTP::receive() {
    char buf[BUFFER_LENGTH];
    socklen_t slen = sizeof(server);

    memset(buf, '\0', BUFFER_LENGTH);
    if (recvfrom(
            sock, buf, BUFFER_LENGTH, 0, (struct sockaddr *)&server, &slen
        ) == -1) {
        kys("recvfrom()");
    }

    int opcode = buf[1];
    if (opcode == static_cast<int>(Opcode::DATA)) {
        Data data(buf);
        Ack ack(data.getBlockNumber());
        send(ack);
        return data.getData();
    } else if (opcode == static_cast<int>(Opcode::ACK)) {
        Ack ack(buf);
    } else if (opcode == static_cast<int>(Opcode::ERROR)) {
        fprintf(stderr, "Error: %s\n", buf + 4);
        exit(1);
    } else {
        fprintf(stderr, "Invalid opcode\n");
        exit(1);
    }

    return nullptr;
}

void TFTP::read(char *filename, Mode mode) {
    RRQ request(filename, mode);
    send(request);
    char *data = receive();
    printf("%s\n", data);
    delete[] data;
}

int main() {
    TFTP tftp((char *)SERVER, PORT);
    tftp.read((char *)"/home/oliver/Documents/pwogwamming/cpp/tftp/.vscode/settings.json", Mode::NETASCII);

    return 0;
}
