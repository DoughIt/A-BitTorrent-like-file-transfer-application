/**
 * handler.c
 *
 * @author Jon Zhang <16302010059@fudan.edu.cn>
 * @date 2018-11-21
 */
#include "handler.h"
#include "spiffy.h"
#include "bt_parse.h"
#include <stdlib.h>

int correct(packet *pkt) {
    pkt_header header = pkt->header;
    if (header.magic_num != MAGIC || header.version_num != VERSION || header.packet_type > DENIED ||
        header.packet_type < WHOHAS)
        return 0;
    pkt_type type = pkt_parse(pkt);
    if ((type == DATA && (header.seq_num <= 0 || header.ack_num > 0))
        || (type == ACK && (header.seq_num > 0 || header.ack_num < 0))
        || (type != DATA && type != ACK && (header.seq_num > 0 || header.ack_num > 0)))
        return 0;
    return 1;
}

int not_correct(packet *pkt) {
    return !correct(pkt);
}

int is_ack(packet *pkt, uint32_t ack_num) {
    return pkt->header->ack_num == ack_num;
}


void send_PKT(int sock, bt_peer_t *peers, uint8_t type, uint32_t seq_ack, uint32_t data_size, uint8_t *data) {
    packet *pkt = make_PKT(NET, type, seq_ack, data_size, data);
    while (peers != NULL) {
        spiffy_sendto(sock, pkt, ntohs(pkt->header->total_pkt_len), 0, (struct sockaddr *) &peers->addr,
                      sizeof(peers->addr));
        peers = peers->next;
    }
}

void process_PKT(packet *pkt, bt_peer_t *peer) {
    if (not_correct(pkt))
        return;
    pkt_type type = pkt_parse(pkt);
    PROCESS_PKT(type, pkt, peer);
}

void process_WHOHAS(packet *pkt, bt_peer_t *peer) {
    //TODO
}

void process_IHAVE(packet *pkt, bt_peer_t *peer) {
    //TODO
}

void process_GET(packet *pkt, bt_peer_t *peer) {
    //TODO
}

void process_DATA(packet *pkt, bt_peer_t *peer) {
    //TODO
}

void process_ACK(packet *pkt, bt_peer_t *peer) {
    //TODO
}

void process_DENIED(packet *pkt, bt_peer_t *peer) {
    //rejected
    return;
}

void process_NULL(packet *pkt, bt_peer_t *peer) {
    //do nothing
    return;
}