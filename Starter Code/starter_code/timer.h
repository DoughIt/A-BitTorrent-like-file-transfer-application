/**
 * timer.h
 *
 * @author Jon Zhang <16302010059@fudan.edu.cn>
 * @date 2018-11-22
 */

#ifndef A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_TIMER_H
#define A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_TIMER_H

#include <sys/time.h>
#include <inttypes.h>

#define ALPHA 0.125
#define BETA 0.25

typedef struct my_timer_s {
    int running;
    struct timeval timestamp;
} my_timer_t;

my_timer_t old;

int is_running(my_timer_t *);

void init_timer(my_timer_t *);

void start_timer(my_timer_t *, uint32_t);

void stop_timer(my_timer_t *, uint32_t);

void update(float);

#endif //A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_TIMER_H
