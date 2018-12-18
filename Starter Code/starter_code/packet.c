/**
 * packet.c
 *
 * Structure for packet.
 *
 * @author Jon Zhang <16302010059@fudan.edu.cn>
 * @date 2018-11-21
 */
#include "packet.h"
#include "sha.h"
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

void init(packet *pkt, uint8_t type, uint16_t tot_len,
          uint32_t seq, uint32_t ack,
          uint8_t *data) {
    pkt_header *header = &pkt->header;
    header->magic_num = MAGIC;
    header->version_num = VERSION;
    header->packet_type = type;
    header->header_len = HDR_SIZE;
    header->total_pkt_len = tot_len;
    header->ack_num = ack;
    header->seq_num = seq;
    memcpy(pkt->data, data, (size_t) (tot_len - HDR_SIZE));
}

void convert(packet *pkt, value_type vt) {
    pkt_header *header = &pkt->header;
    switch (vt) {
        case HOST:
            header->magic_num = ntohs(header->magic_num);
            header->header_len = ntohs(header->header_len);
            header->total_pkt_len = ntohs(header->total_pkt_len);
            header->seq_num = ntohl(header->seq_num);
            header->ack_num = ntohl(header->ack_num);
            break;
        case NET:
            header->magic_num = htons(header->magic_num);
            header->header_len = htons(header->header_len);
            header->total_pkt_len = htons(header->total_pkt_len);
            header->seq_num = htonl(header->seq_num);
            header->ack_num = htonl(header->ack_num);
            break;
        default:
            break;
    }
}


packet *make_PKT(uint8_t type, uint32_t seq_ack, uint32_t data_size, uint8_t *data) {
    packet *pkt = (packet *) malloc(HDR_SIZE + data_size);
    init(pkt, type, (uint16_t) (HDR_SIZE + data_size), type == DATA ? seq_ack : INIT_SEQ,
         type == ACK ? seq_ack : INIT_ACK, data);
    return pkt;
}

packet *make_WHOHAS(uint32_t data_size, uint8_t *data) {
    return make_PKT(WHOHAS, 0, data_size, data);
}

packet *make_IHAVE(uint32_t data_size, uint8_t *data) {
    return make_PKT(IHAVE, 0, data_size, data);
}

packet *make_GET(uint8_t *data) {
    return make_PKT(GET, 0, SHA1_HASH_SIZE, data);
}

packet *make_DATA(uint32_t seq_ack, uint32_t data_size, uint8_t *data) {
    return make_PKT(DATA, seq_ack, data_size, data);
}

packet *make_ACK(uint32_t seq_ack) {
    return make_PKT(ACK, seq_ack, 0, NULL);
}

pkt_type pkt_parse_type(uint8_t type) {
    switch (type) {
        case 0:
            return WHOHAS;
        case 1:
            return IHAVE;
        case 2:
            return GET;
        case 3:
            return DATA;
        case 4:
            return ACK;
        case 5:
            return DENIED;
        default:
            break;
    }
}
