/**
 * handler.h
 *
 * @author Jon Zhang <16302010059@fudan.edu.cn>
 * @date 2018-11-21
 */

#ifndef A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_HANDLER_H
#define A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_HANDLER_H

#include "tracker.h"
#include "rcv_send.h"
#include <sys/socket.h>

void send_PKT(int sock, bt_peer_t *peers, packet *pkt);

void process_download();

void process_PKT(packet *pkt, struct sockaddr_in *from);

void process_WHOHAS(packet *pkt, bt_peer_t *peer);

void process_IHAVE(packet *pkt, bt_peer_t *peer);

void process_GET(packet *pkt, bt_peer_t *peer);

void process_DATA(packet *pkt, bt_peer_t *peer);

void process_ACK(packet *pkt, bt_peer_t *peer);

void process_DENIED(packet *pkt, bt_peer_t *peer);

/**
 * 线程函数
 * 当接收到GET请求时，新建一个线程负责发送该请求需要的DATA包
 * 监听发送窗口，当有多余窗口位置时发送数据包
 * @param sdr
 * @return
 */
void *process_sender(sender *sdr);

/**
 * 线程函数
 * 当接收到download请求时，新建一个线程负责下载该请求所需要的所有chunks
 * @param arg
 * @return
 */
void *pthread_receiver(void *arg);

/**
 * 将下载完成的chunk按序（id）写入目标文件
 */
void chunks2file();

void look_at();

bt_peer_t *get_peer(struct sockaddr_in *addr);

void *cwnd2log(sender *sdr);

int correct(packet *pkt);

int not_correct(packet *pkt);

#endif //A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_HANDLER_H

