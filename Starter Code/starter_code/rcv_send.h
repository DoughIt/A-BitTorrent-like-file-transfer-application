/**
 * rcv_send.h
 *
 * @author Jon Zhang <16302010059@fudan.edu.cn>
 * @date 2018-11-30
 */

#ifndef BITTORRECT_LIKE_RCV_SEND_H
#define BITTORRECT_LIKE_RCV_SEND_H

#include <inttypes.h>
#include "timer.h"
#include "tracker.h"

#define GET_NUM 6          /**< The maximum number of GET requests that a peer can tackle **/
#define WINDOW_SIZE 8      /**< Sliding window size **/
#define DUP_ACK_NUM 3      /**< To avoid confusion from re-ordering, a sender counts a packet lost only after 3 duplicate ACKs in a row **/

typedef enum state {
    DONE, UNDONE, VALID, INVALID
} state;

typedef struct sender_s {
    uint32_t win_size;      /**< last_sent - last_acked <= win_size **/
    uint32_t last_sent;
    uint32_t last_acked;
    uint32_t last_available;
    uint32_t dup_ack_num;
    timer_t timer;
    queue *pkts;
    bt_peer_t *p_receiver;
} sender;

typedef struct receiver_s {
    uint32_t last_read;
    uint32_t last_rcvd;
    uint32_t next_expected;
    chunk_t *chunk;
    bt_peer_t *p_sender;
} receiver;

#define define_pool(type)\
typedef struct type##_pool_s{\
    int max_num;\
    int cur_num;\
    queue *workers;\
} type##_pool_t;


define_pool(sender);
define_pool(receiver);

void init_sender_pool(sender_pool_t *sender_pool, int max);

void init_receiver_pool(receiver_pool_t *receiver_pool, int max);

void add_sender(sender_pool_t *sender_pool, bt_peer_t *p_receiver, queue *pkts);

void add_receiver(receiver_pool_t *receiver_pool, bt_peer_t *p_sender, chunk_t *chunk);


#endif //BITTORRECT_LIKE_RCV_SEND_H
