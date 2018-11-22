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

void init(value_type vt, packet *pkt, uint8_t type, uint16_t tot_len,
          uint32_t seq, uint32_t ack,
          uint8_t *data) {
    switch (vt) {
        case HOST:
            init_host(pkt, MAGIC, VERSION, type, HDR_SIZE, tot_len, seq, ack, data);
            break;
        case NET:
            init_net(pkt, MAGIC, VERSION, type, HDR_SIZE, tot_len, seq, ack, data);
            break;
        default:
            break;
    }
}

void init_host(packet *pkt, uint16_t magic, uint8_t version, uint8_t type,
               uint16_t hdr_len, uint16_t tot_len,
               uint32_t seq, uint32_t ack,
               uint8_t *data) {
    pkt->header->magic_num = ntohs(magic);
    pkt->header->version_num = version & 0xff;
    pkt->header->packet_type = type & 0xff;
    pkt->header->header_len = ntohs(hdr_len);
    pkt->header->total_pkt_len = ntohs(tot_len);
    pkt->header->seq_num = ntohl(seq);
    pkt->header->ack_num = ntohl(ack);
    if (data != NULL) {
        memcpy(pkt->data, data, ntohs(tot_len) - ntohs(hdr_len));
    }
}

void init_net(packet *pkt, uint16_t magic, uint8_t version, uint8_t type,
              uint16_t hdr_len, uint16_t tot_len,
              uint32_t seq, uint32_t ack,
              uint8_t *data) {
    pkt->header->magic_num = htons(magic);
    pkt->header->version_num = version & 0xff;
    pkt->header->packet_type = type & 0xff;
    pkt->header->header_len = htons(hdr_len);
    pkt->header->total_pkt_len = htons(tot_len);
    pkt->header->seq_num = htonl(seq);
    pkt->header->ack_num = htonl(ack);
    if (data != NULL) {
        memcpy(pkt->data, data, tot_len - hdr_len);
    }
}

packet *make_PKT(value_type host_net, uint8_t type, uint32_t seq_ack, uint32_t data_size, uint8_t *data) {
    packet *pkt = (packet *) malloc(sizeof(packet) + data_size);
    init(host_net, pkt, type, sizeof(pkt), type == DATA ? seq_ack : 0, type == ACK ? seq_ack : 0, data);
    return pkt;
}

packet *make_WHOHAS(uint32_t data_size, uint8_t *data) {
    return make_PKT(NET, WHOHAS, 0, data_size, data);
}

packet *make_IHAVE(uint32_t data_size, uint8_t *data) {
    return make_PKT(NET, IHAVE, 0, data_size, data);
}

packet *make_GET(uint8_t *data) {
    return make_PKT(NET, GET, 0, SHA1_HASH_SIZE, data);
}

packet *make_DATA(uint32_t seq_ack, uint32_t data_size, uint8_t *data) {
    return make_PKT(NET, DATA, seq_ack, data_size, data);
}

packet *make_ACK(uint32_t seq_ack) {
    return make_PKT(NET, ACK, seq_ack, 0, NULL);
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
            return NULL;
    }
}