/**
 * queue.c
 *
 * @author Jon Zhang <16302010059@fudan.edu.cn>
 * @date 2018-11-22
 */

#include "queue.h"
#include <malloc.h>

void init_queue(queue *queue1) {
    queue1->head = queue1->tail = NULL;
    queue1->n = 0;
}

void enqueue(queue *queue1, void *data) {
    node *node1 = malloc(sizeof(node));
    node1->data = data;
    node1->next = NULL;
    if (queue1->tail == NULL)
        queue1->head = queue1->tail = node1;
    else {
        queue1->tail->next = node1;
        queue1->tail = node1;
    }
    queue1->n++;
}

void *dequeue(queue *queue1) {
    if (queue1->n <= 0)
        return NULL;
    node *head = queue1->head;
    void *data = head->data;
    queue1->head = queue1->head->next;
    if (queue1->head == NULL)
        queue1->tail = queue1->head;
    queue1->n--;
    free(head);
    return data;
}

int is_empty(queue *queue1) {
    return queue1 == NULL || queue1->n <= 0;
}


int enqueue_all(queue *dest, queue *src) {
    if (dest == NULL || src == NULL || is_empty(src))
        return 0;
    node *cur_node = src->head;
    int size = src->n, i;
    for (i = 0; i < size && cur_node != NULL; ++i, cur_node = cur_node->next) {
        enqueue(dest, cur_node->data);
    }
    return i;
}

void free_queue(queue *queue1) {
    while (queue1->n) {
        free(dequeue(queue1));
    }
    free(queue1);
}