/**
 * rcv_send.h
 *
 * @author Jon Zhang <16302010059@fudan.edu.cn>
 * @date 2018-11-30
 */

#include "rcv_send.h"
#include <malloc.h>

void init_sender_pool(sender_pool_t *sender_pool, int max) {
    sender_pool->max_num = max;
    sender_pool->cur_num = 0;
    sender_pool->workers = malloc(max * sizeof(sender));
}

void init_receiver_pool(receiver_pool_t *receiver_pool, int max) {
    receiver_pool->max_num = max;
    receiver_pool->cur_num = 0;
    receiver_pool->workers = malloc(max * sizeof(receiver));
}

void add_sender(sender_pool_t *sender_pool, bt_peer_t *p_receiver, queue *pkts) {
    if (sender_pool->cur_num >= sender_pool->max_num)
        return;
    sender *sdr = malloc(sizeof(sender));
    sdr->p_receiver = p_receiver;
    sdr->win_size = WINDOW_SIZE;
    sdr->last_sent = 0;
    sdr->last_acked = 0;
    sdr->last_available = WINDOW_SIZE;
    sdr->dup_ack_num = 0;
    sdr->pkts = pkts;
    sdr->timer = malloc(sizeof(timer_t));
    enqueue(sender_pool->workers, sdr);
}

void add_receiver(receiver_pool_t *receiver_pool, bt_peer_t *p_sender, chunk_t *chunk) {
    if (receiver_pool->cur_num >= receiver_pool->max_num)
        return;
    receiver *rcvr = malloc(sizeof(receiver));
    rcvr->last_rcvd = 0;
    rcvr->last_read = 0;
    rcvr->next_expected = 1;
    rcvr->chunk = chunk;
    rcvr->p_sender = p_sender;
    enqueue(receiver_pool->workers, rcvr);
    receiver_pool->cur_num++;
}