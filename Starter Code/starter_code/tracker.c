/**
 * tracker.h
 *
 * @author Jian Zhang <16302010059@fudan.edu.cn>
 * @date 2018-11-23
 */
#include "packet.h"
#include "tracker.h"
#include <malloc.h>
#include <string.h>

void init_chunk(chunk_t *chunk, int id, char *sha1) {
    chunk->id = id;
    memcpy(chunk->sha1, sha1, 255);
    chunk->holders = malloc(sizeof(queue));
    init_queue(chunk->holders);
    chunk->c_state = READY;
}

void free_chunk(chunk_t *chunk) {
    free_queue(chunk->holders);
    free(chunk);
}

int is_ready(chunk_t *chunk) {
    return chunk->c_state == READY;
}

void update_state(queue *down_chunks, chunk_t *chunk, chunk_state new_state) {
    chunk_t *down_chunk = contains_chunk(down_chunks, chunk);
    if (down_chunk != NULL && new_state > down_chunk->c_state && new_state <= FINISHED)
        down_chunk->c_state = new_state;
}

void add_holder(queue *down_chunks, chunk_t *chunk, bt_peer_t *holder) {
    chunk_t *down_chunk = contains_chunk(down_chunks, chunk);
    if (down_chunk != NULL) {
        if (down_chunk->holders == NULL) {
            down_chunk->holders = (queue *) malloc(sizeof(queue));
            init_queue(down_chunk->holders);
        }
        enqueue(down_chunk->holders, holder);
    }
}

chunk_t *get_ready_to_download(queue *down_chunks, chunk_t *chunk) {
    if (down_chunks == NULL || chunk == NULL)
        return NULL;
    chunk_t *down_chunk = NULL;
    if ((down_chunk = contains_chunk(down_chunks, chunk))->c_state == READY) {
        return down_chunk;
    }
    return NULL;
}

chunk_t *choose_chunk_to_download(queue *down_chunks) {
    if (down_chunks == NULL)
        return NULL;
    node *cur_node = down_chunks->head;
    chunk_t *down_chunk = (chunk_t *) cur_node->data;
    while (cur_node) {
        if (((chunk_t *) cur_node->data)->holders->n < down_chunk->holders->n &&
            !is_empty(((chunk_t *) cur_node->data)->holders))
            down_chunk = (chunk_t *) cur_node->data;
        cur_node = cur_node->next;
    }
    return down_chunk;
}

void scan_chunk_done(queue *down_chunks, queue *done_chunks) {
    if (down_chunks == NULL || done_chunks == NULL)
        return;
    int n = down_chunks->n;
    for (int i = 0; i < n; ++i) {
        chunk_t *chunk = (chunk_t *) dequeue(down_chunks);
        if (chunk->c_state == FINISHED) {
            enqueue(done_chunks, chunk);
        } else enqueue(down_chunks, chunk);
    }
}

queue *list_chunks(char *chunkfile) {
    FILE *fp;
    if ((fp = fopen(chunkfile, "r")) == NULL)
        return NULL;
    queue *chunks = (queue *) malloc(sizeof(queue));
    init_queue(chunks);
    char line[TRACKER_LINE_LEN] = {0};
    while (!feof(fp) && fgets(line, TRACKER_LINE_LEN, fp) != NULL) {
        chunk_t *chunk = malloc(sizeof(chunk_t));
        memset(chunk, 0, sizeof(chunk_t));
        sscanf(line, "%d %s", &chunk->id, chunk->sha1);
        init_chunk(chunk, chunk->id, chunk->sha1);
        enqueue(chunks, chunk);
    }
    fclose(fp);
    return chunks;
}

chunk_t *contains_chunk(queue *chunks, chunk_t *chunk) {
    if (chunks == NULL || chunk == NULL)
        return NULL;
    node *cur_node = chunks->head;
    while (cur_node) {
        chunk_t *cur_chunk = (chunk_t *) cur_node->data;
        if (memcmp(chunk->sha1, cur_chunk->sha1, 2 * SHA1_HASH_SIZE) == 0)
            return cur_chunk;
        cur_node = cur_node->next;
    }
    return NULL;
}

queue *chunks2pkts(queue *chunks, pkt_type type) {
    if (chunks == NULL || (type != WHOHAS && type != IHAVE))
        return NULL;
    queue *pkts = (queue *) malloc(sizeof(queue));
    init_queue(pkts);

    node *cur_node = chunks->head;
    chunk_t *chunk;
    int num = chunks->n / SHA_MAX_NUM + (chunks->n % SHA_MAX_NUM > 0);
    char sha1_num;  //每个packet包含的hash数
    for (int i = 0; i < num; ++i) {
        if (i < num - 1)
            sha1_num = SHA_MAX_NUM;
        else sha1_num = (char) (chunks->n % SHA_MAX_NUM);
        int data_size = 4 + SHA1_HASH_SIZE * sha1_num;
        uint8_t *data = (uint8_t *) malloc((size_t) data_size);
        memset(data, 0, (size_t) data_size);
        // hash值数量（1字节），4 字节对齐
        memcpy(data, &sha1_num, 1);
        // hash值列表（每个20字节）
        uint8_t *bin_sha1 = malloc(SHA1_HASH_SIZE);
        memset(bin_sha1, 0, SHA1_HASH_SIZE);
        for (int idx = 0; idx < sha1_num && cur_node != NULL; idx++, cur_node = cur_node->next) {
            if ((chunk = (chunk_t *) cur_node->data) != NULL) {
                hex2binary(chunk->sha1, 2 * SHA1_HASH_SIZE, bin_sha1);//hex_sha1的有效部分占40字节
                memcpy(data + 4 + idx * SHA1_HASH_SIZE, bin_sha1, SHA1_HASH_SIZE);
            } else
                break;
        }
        packet *pkt = make_PKT(type, 0, (uint32_t) data_size, data);
        enqueue(pkts, pkt);
    }
    return pkts;
}

queue *pkt2chunks(packet *pkt, pkt_type type) {
    if (pkt == NULL || (type != WHOHAS && type != IHAVE))
        return NULL;
    queue *chunks = (queue *) malloc(sizeof(queue));
    init_queue(chunks);
    chunk_t *chunk;
    uint8_t *data = pkt->data;
    char sha1_num;
    memcpy(&sha1_num, data, 1);
    uint8_t bin_sha1[SHA1_HASH_SIZE];
    for (int i = 0; i < sha1_num; ++i) {
        chunk = (chunk_t *) malloc(sizeof(chunk_t));
        memset(chunk, 0, sizeof(chunk_t));
        memcpy(bin_sha1, data + 4 + i * SHA1_HASH_SIZE, SHA1_HASH_SIZE);
        binary2hex(bin_sha1, SHA1_HASH_SIZE, chunk->sha1);
        init_chunk(chunk, chunk->id, chunk->sha1);
        enqueue(chunks, chunk);
    }
    return chunks;
}

queue *which_i_have(queue *who_has_chunks, char *has_chunk_file) {
    queue *chunks = list_chunks(has_chunk_file);
    queue *i_have_chunks = (queue *) malloc(sizeof(queue));
    init_queue(i_have_chunks);
    node *cur_node = who_has_chunks->head;
    chunk_t *chunk;
    while (cur_node) {
        if ((chunk = contains_chunk(chunks, (chunk_t *) cur_node->data)) != NULL) {
            enqueue(i_have_chunks, chunk);
        }
        cur_node = cur_node->next;
    }
    return i_have_chunks;
}

packet **chunk2pkts(chunk_t *data_chunk, int *total) {
    char *data = data_chunk->data;
    *total = BT_CHUNK_SIZE / DATA_SIZE + (BT_CHUNK_SIZE % DATA_SIZE > 0);//一个数据块被分成num个DATA数据包
    packet *pkt;
    packet **pkts = malloc(*total * sizeof(packet *));
    int i;
    for (i = 0; i < *total - 1; ++i) {
        uint8_t pkt_data[DATA_SIZE];
        memcpy(pkt_data, data + i * DATA_SIZE, DATA_SIZE);
        pkt = make_DATA((uint32_t) (i + 1), DATA_SIZE, pkt_data);
        pkts[i] = pkt;
    }
    pkt = make_DATA((uint32_t) (i + 1), (uint32_t) (BT_CHUNK_SIZE % DATA_SIZE),
                    (uint8_t *) (data + i * DATA_SIZE));
    pkts[i] = pkt;
    return pkts;
}

chunk_t *get_data_chunk(char *chunkfile, packet *pkt) {
    FILE *fp = fopen(chunkfile, "r");
    if (fp == NULL)
        return NULL;
    char sha1[255] = {0};
    uint8_t *data = malloc(SHA1_HASH_SIZE);
    memcpy(data, pkt->data, SHA1_HASH_SIZE);
    binary2hex(data, SHA1_HASH_SIZE, sha1);

    char file_line[TRACKER_LINE_LEN] = {0};
    char file_name[BT_FILENAME_LEN];
    if (!feof(fp)) {
        fgets(file_line, TRACKER_LINE_LEN, fp);
    } else {
        return NULL;
    }
    sscanf(file_line, "File: %s\n", file_name);
    char line[TRACKER_LINE_LEN] = {0};
    fgets(line, TRACKER_LINE_LEN, fp);
    chunk_t *data_chunk = malloc(sizeof(chunk_t));
    init_chunk(data_chunk, 0, "");
    while (!feof(fp) && fgets(line, TRACKER_LINE_LEN, fp) != NULL) {
        sscanf(line, "%d %s", &data_chunk->id, data_chunk->sha1);
        printf("%s\n", data_chunk->sha1);
        if (memcmp(sha1, data_chunk->sha1, SHA1_HASH_SIZE) == 0) {
            puts("Found one chunk!");
            break;
        }
    }
    fclose(fp);
    fp = fopen(file_name, "r");
    fseek(fp, data_chunk->id * BT_CHUNK_SIZE, SEEK_SET);    //定位读指针起始位置
    fread(data_chunk->data, BT_CHUNK_SIZE, 1, fp);
    fclose(fp);
    return data_chunk;
}

int check_chunk(chunk_t *chunk, char *sha1) {
    uint8_t target[2 * SHA1_HASH_SIZE];
    shahash((uint8_t *) chunk->data, BT_CHUNK_SIZE, target);
    char target_hex[SHA1_HASH_SIZE] = {0};
    binary2hex(target, SHA1_HASH_SIZE, target_hex);
    return memcmp(target_hex, sha1, 2 * SHA1_HASH_SIZE) == 0;
}