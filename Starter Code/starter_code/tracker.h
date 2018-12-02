/**
 * tracker.h
 *
 * @author Jian Zhang <16302010059@fudan.edu.cn>
 * @date 2018-11-23
 */
#ifndef BITTORRENT_TRACKER_H
#define BITTORRENT_TRACKER_H

#include "chunk.h"
#include "sha.h"
#include "bt_parse.h"
#include "queue.h"
#include "packet.h"

#define TRACKER_LINE_LEN 64
#define SHA_MAX_NUM ((PKT_SIZE - HDR_SIZE - 4)/SHA1_HASH_SIZE)    // 74

typedef enum chunk_state {
    READY, DOWNLOADING, FINISHED
} chunk_state;

typedef struct chunk_s {
    int id;
    char sha1[255];
    chunk_state c_state;
    queue *holders;
    char data[BT_CHUNK_SIZE];
} chunk_t;

void init_chunk(chunk_t *chunk, char *sha1);

void free_chunk(chunk_t *chunk);

int is_ready(chunk_t *chunk);

void update_state(queue *down_chunks, chunk_t *chunk, chunk_state new_state);

void add_holder(queue *down_chunks, chunk_t *chunk, bt_peer_t *holder);

/**
 * @return the chunk that its state is READY & its reference in chunks is ready too,
 * that is it has not been downloading or downloaded
 */
chunk_t *get_ready_to_download(queue *down_chunks, chunk_t *chunk);

/**
 * 最稀缺（来源holders最少）的chunk优先下载
 * @param down_chunks
 * @return down_chunk
 */
chunk_t *choose_chunk_to_download(queue *down_chunks);

/**
 * 将下载完成的chunk从down_chunks移除，添加到done_chunks队列
 * @param down_chunks
 * @param done_chunks
 */
void scan_chunk_done(queue *down_chunks, queue *done_chunks);

/**
 * List the information about each chunk of the file.
 * @param chunkfile
 * @return chunk list
 */
queue *list_chunks(char *chunkfile);

/**
 * @return chunk' if the chunk is belong to chunks, else NULL
 */
chunk_t *contains_chunk(queue *chunks, chunk_t *chunk);

/**
 * Convert chunks to packets
 */
queue *chunks2pkts(queue *chunks, pkt_type type);

/**
 * Convert packet to chunks
 */
queue *pkt2chunks(packet *pkt, pkt_type type);


queue *which_i_have(queue *who_has_chunks, char *has_chunk_file);

/**
 * Convert chunk to DATA packets
 * @param data_chunk
 * @return
 */
packet **chunk2pkts(chunk_t *data_chunk);

chunk_t *get_data_chunk(char *chunkfile, packet *pkt);

/**
 * Determine whether the client obtained the right chunk or not.
 * @param chunk
 * @param sha1
 * @return 0 or 1
 */
int check_chunk(chunk_t *chunk, char *sha1);

#endif //BITTORRENT_TRACKER_H
