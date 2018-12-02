/**
 * handler.h
 *
 * @author Jon Zhang <16302010059@fudan.edu.cn>
 * @date 2018-11-21
 */

#ifndef A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_HANDLER_H
#define A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_HANDLER_H

#include "timer.h"
#include "tracker.h"
#include <sys/socket.h>

int correct(packet *pkt);

int not_correct(packet *pkt);

int is_ack(packet *pkt, uint32_t ack_num);

void send_PKT(int sock, bt_peer_t *peers, packet *pkt);

void send_PKTS(int sock, bt_peer_t *peer, queue *pkts);

void look_at();

bt_peer_t *get_peer(struct sockaddr_in addr);

void process_download(bt_config_t config, int sock);

void process_PKT(packet *pkt, struct sockaddr_in from);

void process_WHOHAS(packet *pkt, bt_peer_t *peer);

void process_IHAVE(packet *pkt, bt_peer_t *peer);

void process_GET(packet *pkt, bt_peer_t *peer);

void process_DATA(packet *pkt, bt_peer_t *peer);

void process_ACK(packet *pkt, bt_peer_t *peer);

void process_DENIED(packet *pkt, bt_peer_t *peer);

#endif //A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_HANDLER_H
