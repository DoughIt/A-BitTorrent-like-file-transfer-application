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

sender *add_sender(sender_pool_t *sender_pool, bt_peer_t *p_rcvr, queue *pkts) {
    if (sender_pool->cur_num >= sender_pool->max_num)
        return NULL;
    sender *sdr = malloc(sizeof(sender));
    sdr->p_receiver = p_rcvr;
    sdr->win_size = WINDOW_SIZE;
    sdr->last_sent = 0;
    sdr->last_acked = 0;
    sdr->last_available = WINDOW_SIZE;
    sdr->dup_ack_num = 0;
    sdr->pkts = pkts;
    sdr->timer = (my_timer_t *) malloc(sizeof(my_timer_t));
    init_timer(sdr->timer);
    enqueue(sender_pool->workers, sdr);
    sender_pool->cur_num++;
    return sdr;
}

receiver *add_receiver(receiver_pool_t *receiver_pool, bt_peer_t *p_sdr, chunk_t *chunk) {
    if (receiver_pool->cur_num >= receiver_pool->max_num)
        return NULL;
    receiver *rcvr = malloc(sizeof(receiver));
    rcvr->last_rcvd = 0;
    rcvr->last_read = 0;
    rcvr->next_expected = 1;
    rcvr->chunk = chunk;
    rcvr->p_sender = p_sdr;
    rcvr->timer = malloc(sizeof(my_timer_t));
    init_timer(rcvr->timer);
    enqueue(receiver_pool->workers, rcvr);
    receiver_pool->cur_num++;
    return rcvr;
}

sender *get_sender(sender_pool_t *sdr_pool, bt_peer_t *p_rcvr) {
    if (sdr_pool == NULL || p_rcvr == NULL)
        return NULL;
    queue *sdrs = sdr_pool->workers;
    node *cur_node = sdrs->head;
    while (cur_node) {
        if (((sender *) cur_node->data)->p_receiver == p_rcvr)
            return (sender *) cur_node->data;
        cur_node = cur_node->next;
    }
    return NULL;
}

receiver *get_receiver(receiver_pool_t *rcvr_pool, bt_peer_t *p_sdr) {
    if (rcvr_pool == NULL || p_sdr == NULL)
        return NULL;
    queue *rcvrs = rcvr_pool->workers;
    node *cur_node = rcvrs->head;
    while (cur_node) {
        if (((receiver *) cur_node->data)->p_sender == p_sdr)
            return (receiver *) cur_node->data;
        cur_node = cur_node->next;
    }
    return NULL;
}

void remove_sender(sender_pool_t *sender_pool, sender *sdr) {

}

void remove_receiver(receiver_pool_t *receiver_pool, receiver *rcvr) {

}