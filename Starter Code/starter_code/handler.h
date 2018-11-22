/**
 * handler.h
 *
 * @author Jon Zhang <16302010059@fudan.edu.cn>
 * @date 2018-11-21
 */

#ifndef A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_HANDLER_H
#define A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_HANDLER_H

#include "packet.h"

#define GET_NUM 6          /**< The maximum number of GET requests that a peer can tackle **/
#define WINDOW_SIZE 8      /**< Sliding window size **/
#define DUP_ACK_NUM 3      /**< To avoid confusion from re-ordering, a sender counts a packet lost only after 3 duplicate ACKs in a row **/
#define TIMEOUT 5

void send_packet(uint8_t type, uint32_t seq_ack, uint32_t data_size, uint8_t *data);

void send_whohas();

void send_ihave();

void send_get();

void send_data();

void send_ack();

void process_packet();

void process_whohas();

void process_ihave();

void process_get();

void process_data();

void process_ack();

#endif //A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_HANDLER_H
