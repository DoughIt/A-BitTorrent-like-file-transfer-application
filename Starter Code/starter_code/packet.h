//
// Created by Jian Zhang on 18-11-20.
//

#ifndef A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_PACKET_H
#define A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_PACKET_H

#define MAXPKTSIZE 1500 //The maximum packet size for UDP is 1500 bytes
#define HEADERSIZE 16   //The header length
#define MAXDATASIZE (MAXPKTSIZE - HEADERSIZE)
enum TYPE {
    WHOHAS = 0, IHAVE, GET, DATA, ACK, DENIED
};
/**
 * 1. Magic Number [2 bytes]
 * 2. Version Number [1 byte]
 * 3. Packet Type [1 byte]
 * 4. Header Length [2 bytes]
 * 5. Total Packet Length [2 bytes]
 * 6. Sequence Number [4 bytes]
 * 7. Acknowledgment Number [4 bytes]
 * All integers must be unsigned
 *
 * The magic number should be 15441, the version number should be 1.
 * Peers should drop packet that do not have these values.
 */
typedef struct header {
    unsigned short magic_num;   //The magic number should be 15441
    unsigned char version_num;  //The version number should be 1
    unsigned char packet_type;

    unsigned short header_len;
    unsigned short total_pkt_len;

    unsigned int seq_num;       //Used for congestion control and reliable transmission

    unsigned int ack_num;       //Used for reliable transmission

};

typedef struct packet {
    header pkt_header;
    char data[MAXDATASIZE];
} pkt;
#endif //A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_PACKET_H
