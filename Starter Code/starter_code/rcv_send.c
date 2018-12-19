/**
 * rcv_send.h
 *
 * @author Jon Zhang <16302010059@fudan.edu.cn>
 * @date 2018-11-30
 */

#include "rcv_send.h"
#include <malloc.h>
#include <math.h>
#include <pthread.h>

double timeout_interval = 1000;

void init_sender_pool(sender_pool_t *sender_pool, int max) {
    sender_pool->max_num = max;
    sender_pool->cur_num = 0;
    sender_pool->workers = malloc(max * sizeof(sender));
    init_queue(sender_pool->workers);
}

void init_receiver_pool(receiver_pool_t *receiver_pool, int max) {
    receiver_pool->max_num = max;
    receiver_pool->cur_num = 0;
    receiver_pool->workers = malloc(max * sizeof(receiver));
    init_queue(receiver_pool->workers);
}

sender *add_sender(sender_pool_t *sender_pool, bt_peer_t *p_rcvr, packet **pkts, int num) {
    if (sender_pool->cur_num >= sender_pool->max_num)
        return NULL;
    sender *sdr = malloc(sizeof(sender));
    sdr->p_receiver = p_rcvr;
    sdr->ssthresh = SSTHRESH;
    sdr->cwnd = CWND;
    sdr->last_sent = 0;
    sdr->last_acked = 0;
    sdr->last_available = CWND;
    sdr->dup_ack_num = 0;
    sdr->pkts = pkts;
    sdr->pkt_num = (uint32_t) num;
    sdr->timer = (my_timer_t *) malloc(sizeof(my_timer_t));
    init_timer(sdr->timer, sdr->last_acked);
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
        if (((receiver *) cur_node->data)->p_sender == p_sdr) {
            return (receiver *) cur_node->data;
        }
        cur_node = cur_node->next;
    }
    return NULL;
}

void remove_sender(sender_pool_t *sender_pool, sender *sdr) {
    if (sender_pool == NULL || sdr == NULL)
        return;
    sender *cur_sender;
    int n = sender_pool->cur_num;
    for (int i = 0; i < n; ++i) {
        cur_sender = dequeue(sender_pool->workers);
        if (cur_sender->p_receiver == sdr->p_receiver) {
            sdr->timer->running = 0;
            sender_pool->cur_num--;
            return;
        }
        enqueue(sender_pool->workers, cur_sender);
    }
}

void remove_receiver(receiver_pool_t *receiver_pool, receiver *rcvr) {
    if (receiver_pool == NULL || rcvr == NULL)
        return;
    receiver *cur_receiver;
    int n = receiver_pool->cur_num;
    for (int i = 0; i < n; ++i) {
        cur_receiver = dequeue(receiver_pool->workers);
        if (cur_receiver->p_sender == rcvr->p_sender) {
            receiver_pool->cur_num--;
            return;
        }
        enqueue(receiver_pool->workers, cur_receiver);
    }
}

void retransmit(sender *sdr, uint32_t last_acked) {
    sdr->last_sent = last_acked;
    sdr->last_acked = last_acked;
    sdr->ssthresh = (uint32_t) (sdr->cwnd / 2);
    sdr->cwnd = sdr->ssthresh + DUP_ACK_NUM;
    sdr->dup_ack_num = 1;
    sdr->last_available = (uint32_t) (sdr->last_sent + sdr->cwnd);
}


int is_running(my_timer_t *timer) {
    return timer->running == 1;
}

void init_timer(my_timer_t *timer, uint32_t id) {
    timer->running = 0;
    timer->id = id;
    timer->timeout.tv_sec = (__time_t) (timeout_interval / 1000);
    timer->timeout.tv_usec = 0;
}

void *t_start(sender *sdr) {
    uint32_t id = sdr->timer->id;
    if (is_running(sdr->timer) && select(0, NULL, NULL, NULL, &sdr->timer->timeout) == 0) {
        if (sdr->timer->id == id) {//判断当前计时段内计时器是否被重启过，如重启过则该次计时失效
            retransmit(sdr, id);
        }
    }
    return NULL;
}

void start_timer(sender *sdr) {
    if (sdr == NULL)
        return;
    sdr->timer->running = 1;
    pthread_t t_timeout;
    pthread_create(&t_timeout, NULL, (void *(*)(void *)) t_start, sdr);
}

void stop_timer(sender *sdr) {
    if (sdr == NULL)
        return;
    sdr->timer->running = 0;
}