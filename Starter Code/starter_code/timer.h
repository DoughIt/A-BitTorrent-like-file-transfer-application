/**
 * timer.h
 *
 * @author Jon Zhang <16302010059@fudan.edu.cn>
 * @date 2018-11-22
 */

#ifndef A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_TIMER_H
#define A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_TIMER_H

#include <sys/time.h>

#define ALPHA 0.125
#define BETA 0.25

typedef unsigned char bool;

typedef struct timer_s {
    bool running;
    struct timeval timestamp;
} timer_t;

bool is_running(timer_t *);

void start_timer(timer_t *, uint32_t);

void stop_timer(timer_t *, uint32_t);

void update(float);
#endif //A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_TIMER_H
