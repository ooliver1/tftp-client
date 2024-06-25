# Notes

## Overviewe

- netascii - ASCII where codes are interpreted in telnet - end lines in CRLF, CR -> CR NUL
- octet - raw 8-bit bytes

Block length 512 bytes over UDP. One packet = one block. If packet lost, recipient times out, sends last packet back, then continue. Acknowledgement exists however so UDP is marginally useless?

Errors cause termination after error packet - packet not resent, malformed packet, cannot execute (file not found), access lost (disk full).

Header contains 2 byte opcode (DATA, ERROR, ...).

## Connection

Send write or read request (WRQ, RRQ) to server. Receive ACK for write, first DATA for read. ACK is of a block number - sequential of each data block beginning with 1. It is 0 for a WRQ ACK.

Choose a TID for the connection, send request to decimal 69 port, server sends back its TID (port) and uses TID used previously.

## Packets

1. RRQ
2. WRQ
3. DATA
4. ACK
5. ERROR

### RRQ/WRQ

| 2 bytes | string   | 1 byte | string | 1 byte |
| ------- | -------- | ------ | ------ | ------ |
| Opcode  | Filename | 0      | Mode   | 0      |

Mode is either "netascii" or "octet".

### DATA

| 2 bytes | 2 bytes | n bytes |
| ------- | ------- | ------- |
| Opcode  | Block # | Data    |

### ACK

| 2 bytes | 2 bytes |
| ------- | ------- |
| Opcode  | Block # |

### ERROR

| 2 bytes | 2 bytes | string | 1 byte |
| ------- | ------- | ------ | ------ |
| Opcode  | ErrCode | ErrMsg | 0      |

## Termination

A final packet with 0-512 bytes. If 512 bytes, another empty packet is sent. ACK is sent for this final packet with an ACK response too?

## Error Codes

| Error Code | Error Message                            |
| ---------- | ---------------------------------------- |
| 0          | Not defined, see error message (if any). |
| 1          | File not found.                          |
| 2          | Access violation.                        |
| 3          | Disk full or allocation exceeded.        |
| 4          | Illegal TFTP operation.                  |
| 5          | Unknown transfer ID.                     |
| 6          | File already exists.                     |
| 7          | No such user.                            |
