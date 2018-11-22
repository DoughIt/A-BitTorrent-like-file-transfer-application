/**
 * queue.h
 *
 * @author Jon Zhang <16302010059@fudan.edu.cn>
 * @date 2018-11-22
 */
#ifndef A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_QUEUE_H
#define A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_QUEUE_H

#include "sha.h"

typedef enum state {
    DONE, UNDONE, VALID, INVALID
} state;

typedef struct node {
    state flag;
    uint32_t seq;
    SHA1Context sha1;
    node *next;
} node;

typedef struct queue {
    node *head;
    node *tail;
} queue;

void init_queue(queue *);

void enqueue(queue *, node *);

node *dequeue(queue *);

#endif; //A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_QUEUE_H
