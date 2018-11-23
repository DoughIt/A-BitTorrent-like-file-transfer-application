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

#define CHUNK_SIZE (512 * 1024)

#define MASTER_DATA_FILE "C.chunks"
typedef struct chunk_s {
    char sha1[SHA1_HASH_SIZE];
    char *data[CHUNK_SIZE];
    chunk_t *next;
} chunk_t;

#define define_queue(type) typdedef struct queue_##type{\
    type *head;\
    type *tail;\
    int n;\
} queue_##type;

define_queue(chunk_t);
define_queue(bt_peer_t);

/**
 * List the information about each chunk of the file.
 * @param chunkfile
 * @return chunk list
 */
struct queue_chunk_t *list_chunks(char *chunkfile);

/**
 * @param chunk
 * @return a list of peers that contain the chunk
 */
struct queue_bt_peer_t which_have(chunk_t *chunk);

/**
 * Determine whether the client obtained the right chunk or not.
 * @param chunk
 * @param sha1
 * @return 0 or 1
 */
int check_chunk(chunk_t *chunk, char *sha1);


#endif; //BITTORRENT_TRACKER_H
