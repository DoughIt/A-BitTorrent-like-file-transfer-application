/**
 * handler.h
 *
 * @author Jon Zhang <16302010059@fudan.edu.cn>
 * @date 2018-11-21
 */

#ifndef A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_HANDLER_H
#define A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_HANDLER_H

#include "tracker.h"
#include "rcv_send.h"
#include <sys/socket.h>

int correct(packet *pkt);

int not_correct(packet *pkt);

void output();

void look_at();

void send_PKT(int sock, bt_peer_t *peers, packet *pkt);

void send_PKTS(int sock, bt_peer_t *peer, queue *pkts);

bt_peer_t *get_peer(struct sockaddr_in *addr);

void *process_sender(sender *sdr);

void process_download();

void process_PKT(packet *pkt, struct sockaddr_in *from);

void process_WHOHAS(packet *pkt, bt_peer_t *peer);

void process_IHAVE(packet *pkt, bt_peer_t *peer);

void process_GET(packet *pkt, bt_peer_t *peer);

void process_DATA(packet *pkt, bt_peer_t *peer);

void process_ACK(packet *pkt, bt_peer_t *peer);

void process_DENIED(packet *pkt, bt_peer_t *peer);

#endif //A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_HANDLER_H
