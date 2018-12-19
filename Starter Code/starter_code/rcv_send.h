/**
 * rcv_send.h
 *
 * @author Jon Zhang <16302010059@fudan.edu.cn>
 * @date 2018-11-30
 */

#ifndef BITTORRECT_LIKE_RCV_SEND_H
#define BITTORRECT_LIKE_RCV_SEND_H

#include <inttypes.h>
#include "tracker.h"

#define GET_NUM 6           /** The maximum number of GET requests that a peer can tackle **/
#define SSTHRESH 20          /** Default ssthresh value **/
#define CWND 1              /** Default cwnd value **/
#define DUP_ACK_NUM 3       /** To avoid confusion from re-ordering, a sender counts a packet lost only after 3 duplicate ACKs in a row **/

#define ALPHA 0.125
#define BETA 0.25

typedef enum state {
    DONE, UNDONE, VALID, INVALID
} state;


typedef struct my_timer_s {
    int running;
    uint32_t id;
    struct timeval timeout;
} my_timer_t;


typedef struct sender_s {
    uint32_t ssthresh;
    float cwnd;        /** last_sent - last_acked <= cwnd **/
    uint32_t last_sent;
    uint32_t last_acked;
    uint32_t last_available;
    uint32_t dup_ack_num;
    uint32_t pkt_num;
    my_timer_t *timer;
    packet **pkts;
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

sender *add_sender(sender_pool_t *sender_pool, bt_peer_t *p_rcvr, packet **pkts, int num);

receiver *add_receiver(receiver_pool_t *receiver_pool, bt_peer_t *p_sdr, chunk_t *chunk);

sender *get_sender(sender_pool_t *sdr_pool, bt_peer_t *p_rcvr);

receiver *get_receiver(receiver_pool_t *rcvr_pool, bt_peer_t *p_sdr);

void remove_sender(sender_pool_t *sender_pool, sender *sdr);

void remove_receiver(receiver_pool_t *receiver_pool, receiver *rcvr);

void retransmit(sender *sdr, uint32_t last_acked);

int is_running(my_timer_t *);

void init_timer(my_timer_t *, uint32_t);

void start_timer(sender *);

void stop_timer(sender *);

#endif //BITTORRECT_LIKE_RCV_SEND_H
