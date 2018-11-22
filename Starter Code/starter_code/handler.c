/**
 * handler.c
 *
 * @author Jon Zhang <16302010059@fudan.edu.cn>
 * @date 2018-11-21
 */
#include "handler.h"
#include "spiffy.h"
#include "chunk.h"
#include "bt_parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>


void send_packet(int sock, bt_peer_t *peers, uint8_t type, uint32_t seq_ack, uint32_t data_size, uint8_t *data) {
    packet *pkt = (packet *) malloc(sizeof(packet) + data_size);
    init(NET, pkt, type, HDR_SIZE + data_size,
         type == DATA ? seq_ack : 0,
         type == DATA ? seq_ack : 0,
         data);
    while (peers != NULL) {
        spiffy_sendto(sock, pkt, ntohs(pkt->header->total_pkt_len), 0, (struct sockaddr *) &peers->addr,
                      sizeof(peers->addr));
        peers = peers->next;
    }
}