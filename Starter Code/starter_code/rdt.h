/**
 * rdt.h
 *
 * @author Jon Zhang <16302010059@fudan.edu.cn>
 * @date 2018-11-22
 */
#ifndef A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_RDT_H
#define A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_RDT_H

#include "packet.h"
#include <inttypes.h>

int rdt_send(void *data);

int rdt_rcv(packet *pkt);

int correct(packet *pkt);

int not_correct(packet *pkt);

int is_ack(packet *pkt, uint32_t ack_num);


#endif //A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_RDT_H
