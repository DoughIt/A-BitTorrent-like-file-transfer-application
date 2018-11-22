/**
 * handler.h
 *
 * @author Jon Zhang <16302010059@fudan.edu.cn>
 * @date 2018-11-21
 */

#ifndef A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_HANDLER_H
#define A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_HANDLER_H

#include "packet.h"
#include "queue.h"
#include "timer.h"
#include <sys/socket.h>

#define GET_NUM 6          /**< The maximum number of GET requests that a peer can tackle **/
#define WINDOW_SIZE 8      /**< Sliding window size **/
#define DUP_ACK_NUM 3      /**< To avoid confusion from re-ordering, a sender counts a packet lost only after 3 duplicate ACKs in a row **/
#define PROCESS_PKT(type, pkt, peer) process_##type(pkt, peer)  /**< process_WHOHAS, process_GET, and process_IHAVE etc. **/

typedef struct sender_s {
    uint32_t win_size;      /**< last_sent - last_acked <= win_size **/
    uint32_t last_sent;
    uint32_t last_acked;
    uint32_t last_available;
    uint32_t dup_ack_num;
    timer_t timer;
    queue *window;
} sender_t;

typedef struct receiver_s {
    uint32_t last_read;
    uint32_t last_rcvd;
    uint32_t next_expected;
} receiver_t;

//TODO

int correct(packet *pkt);

int not_correct(packet *pkt);

int is_ack(packet *pkt, uint32_t ack_num);

void send_PKT(uint8_t type, uint32_t seq_ack, uint32_t data_size, uint8_t *data);

//void send_WHOHAS(packet *pkt, bt_peer_t *peer);
//
//void send_IHAVE(packet *pkt, bt_peer_t *peer);
//
//void send_GET(packet *pkt, bt_peer_t *peer);
//
//void send_DATA(packet *pkt, bt_peer_t *peer);
//
//void send_ACK(packet *pkt, bt_peer_t *peer);

void process_PKT(packet *pkt, bt_peer_t *peer);

void process_WHOHAS(packet *pkt, bt_peer_t *peer);

void process_IHAVE(packet *pkt, bt_peer_t *peer);

void process_GET(packet *pkt, bt_peer_t *peer);

void process_DATA(packet *pkt, bt_peer_t *peer);

void process_ACK(packet *pkt, bt_peer_t *peer);

void process_DENIED(packet *pkt, bt_peer_t *peer);
/**
 * This method is used for NULL type.
 * pkt_parse(pkt) == NULL
 */
void process_NULL(packet *pkt, bt_peer_t *peer);

#endif; //A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_HANDLER_H
