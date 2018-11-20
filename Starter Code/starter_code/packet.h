//
// Created by Jian Zhang on 18-11-20.
//

#ifndef A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_PACKET_H
#define A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_PACKET_H

#define WHOHAS 0
#define IHAVE  1
#define GET    2
#define DATA   3
#define ACK    4
#define DENIED 5
/**
 * 1. Magic Number [2 bytes]
 * 2. Version Number [1 byte]
 * 3. Packet Type [1 byte]
 * 4. Header Length [2 bytes]
 * 5. Total Packet Length [2 bytes]
 * 6. Sequence Number [4 bytes]
 * 7. Acknowledgment Number [4 bytes]
 * All integers must be unsigned
 */
typedef struct header{
    unsigned short magic_num;
    unsigned char version_num;
    unsigned char packet_type;

    unsigned short header_len;
    unsigned short total_pkt_len;

    unsigned int seq_num;

    unsigned int ack_num;

};

typedef struct packet{
    header pkt_header;
    char data[1000];
} pkt;
#endif //A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_PACKET_H
