/**
 * timer.h
 *
 * @author Jon Zhang <16302010059@fudan.edu.cn>
 * @date 2018-11-22
 */

#ifndef A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_TIMER_H
#define A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_TIMER_H

int running;

int is_running();

void start_timer(uint32_t seq);

void stop_timer(uint32_t seq);

#endif //A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_TIMER_H
