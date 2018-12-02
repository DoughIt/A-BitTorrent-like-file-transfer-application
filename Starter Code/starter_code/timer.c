/**
 * timer.c
 *
 * @author Jon Zhang <16302010059@fudan.edu.cn>
 * @date 2018-11-22
 */

#include "timer.h"
#include <math.h>
#include <stddef.h>

double timeout_interval = 1000, dev_rtt = 0, estimated_rtt = 0, sample_rtt = 0;

int is_running(my_timer_t *timer) {
    return timer->running == 1;
}

void init_timer(my_timer_t *timer) {
    timer->running = 0;
    timer->timestamp.tv_sec = (__time_t) (timeout_interval / 1000);
    timer->timestamp.tv_usec = 0;
}

void start_timer(my_timer_t *timer, uint32_t seq) {
    if (timer == NULL)
        return;
    timer->running = 1;
    //TODO
}

void stop_timer(my_timer_t *timer, uint32_t seq) {
    if (timer == NULL)
        return;
    timer->running = 0;
    //TODO
}

void update(float sample_rtt) {
    estimated_rtt = (1 - ALPHA) * estimated_rtt + ALPHA * sample_rtt;
    dev_rtt = (1 - BETA) * dev_rtt + BETA * fabs(sample_rtt - estimated_rtt);
    timeout_interval = estimated_rtt + 4 * dev_rtt;
}