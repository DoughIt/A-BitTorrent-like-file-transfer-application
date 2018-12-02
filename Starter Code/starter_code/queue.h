/**
 * queue.h
 *
 * @author Jon Zhang <16302010059@fudan.edu.cn>
 * @date 2018-11-22
 */
#ifndef A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_QUEUE_H
#define A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_QUEUE_H
typedef struct node {
    void *data;
    struct node *next;
} node;

typedef struct queue {
    node *head;
    node *tail;
    int n;
} queue;

void init_queue(queue *);

void enqueue(queue *, void *);

void *dequeue(queue *);

int is_empty(queue *);

int enqueue_all(queue *, queue *);

void free_queue(queue *);

#endif //A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_QUEUE_H
