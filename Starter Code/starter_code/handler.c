/**
 * handler.c
 *
 * @author Jon Zhang <16302010059@fudan.edu.cn>
 * @date 2018-11-21
 */
#include "handler.h"
#include "spiffy.h"
#include "packet.h"
#include <sys/time.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include <pthread.h>

extern bt_config_t config;
extern int sock;
queue *down_chunks;//需要下载的chunk
queue *done_chunks;//下载完成的chunk

sender_pool_t *sender_pool;
receiver_pool_t *receiver_pool;

FILE *log_file;
struct timeval starter;
struct timeval now;
#define LOG(id, ms, cwnd) fprintf(log_file, "peer%d\ttime = %d\tcwnd = %d\n", id, ms, cwnd)

void send_PKT(int sock, bt_peer_t *peer, packet *pkt) {
    if (peer != NULL) {
        convert(pkt, NET);
        spiffy_sendto(sock, pkt, ntohs(pkt->header.total_pkt_len), 0, (struct sockaddr *) &peer->addr,
                      sizeof(peer->addr));
        convert(pkt, HOST);
    }
}

void process_download() {
    receiver_pool = malloc(sizeof(receiver_pool_t));
    init_receiver_pool(receiver_pool, config.max_conn);

    queue *queue_chunk = list_chunks(config.chunk_file);
    down_chunks = queue_chunk;

    done_chunks = (queue *) malloc(sizeof(queue));
    init_queue(done_chunks);

    queue *queue_pkt = chunks2pkts(queue_chunk, WHOHAS);
    packet *pkt;
    while ((pkt = (packet *) dequeue(queue_pkt)) != NULL) {
        bt_peer_t *peers = config.peers;
        while (peers != NULL) {
            if (peers->id != config.identity)
                send_PKT(sock, peers, pkt);
            peers = peers->next;
        }
    }
    //新建一个线程下载所有chunks
    pthread_t t_receiver;
    if (pthread_create(&t_receiver, NULL, pthread_receiver, NULL) == -1) {//创建线程下载chunks
        puts("Fail to download");
        exit(-1);
    }
//    void *res; //等待线程结束
//    if (pthread_join(t_receiver, &res) == -1) {
//        exit(-1);
//    }
}

void process_PKT(packet *pkt, struct sockaddr_in *from) {
    convert(pkt, HOST);
    if (not_correct(pkt)) {
        puts("Bad packet!");
        return;
    }
    bt_peer_t *peer = get_peer(from);
    pkt_type type = pkt_parse_type(pkt->header.packet_type);
    switch (type) {
        case WHOHAS:
            puts("Received WHOHAS");
            process_WHOHAS(pkt, peer);
            break;
        case IHAVE:
            puts("Received IHAVE");
            process_IHAVE(pkt, peer);
            break;
        case GET:
            puts("Received GET");
            process_GET(pkt, peer);
            break;
        case ACK:
//            puts("Received ACK");
            process_ACK(pkt, peer);
            break;
        case DATA:
            puts("Received DATA");
            process_DATA(pkt, peer);
            break;
        case DENIED:
            puts("Received DENIED");
            process_DENIED(pkt, peer);
            break;
        default:
            puts("Have no such type!");
            break;
    }
}

void process_WHOHAS(packet *pkt, bt_peer_t *peer) {
    queue *who_has_chunks = pkt2chunks(pkt, WHOHAS);
    queue *i_have_chunks = which_i_have(who_has_chunks, config.has_chunk_file);
    if (i_have_chunks != NULL && !is_empty(i_have_chunks)) {
//        node *cur_node = i_have_chunks->head;
//        while (cur_node) {
//            printf("id = %d\tsha1 = %s\n", ((chunk_t *) cur_node->data)->id, ((chunk_t *) cur_node->data)->sha1);
//            cur_node = cur_node->next;
//        }
        queue *i_have_pkts = chunks2pkts(i_have_chunks, IHAVE);
        send_PKT(sock, peer, (packet *) dequeue(i_have_pkts));
        free_queue(i_have_pkts);
        free_queue(i_have_chunks);
    }
    free_queue(who_has_chunks);
}

void process_IHAVE(packet *pkt, bt_peer_t *peer) {
    queue *i_have_chunks = pkt2chunks(pkt, IHAVE);
    node *cur_node = i_have_chunks->head;
    while (cur_node) {
        add_holder(down_chunks, (chunk_t *) cur_node->data, peer);
        cur_node = cur_node->next;
    }

}

void process_GET(packet *pkt, bt_peer_t *peer) {
    if (sender_pool == NULL) {
        sender_pool = malloc(sizeof(sender_pool_t));
        init_sender_pool(sender_pool, config.max_conn);
    }
    sender *sdr = get_sender(sender_pool, peer);
    //来自当前peer的上一个GET请求处理完毕，关闭连接
    if (sdr != NULL && sdr->last_sent == sdr->pkt_num)
        remove_sender(sender_pool, sdr);
    else if (sdr != NULL) {
//        puts("Busy connection");
        return;
    }
    if (sender_pool->cur_num >= sender_pool->max_num) {
//        puts("Full connection pool");
        packet *pkt_denied = make_PKT(DENIED, 0, 0, NULL);
        send_PKT(sock, peer, pkt_denied);
        free(pkt_denied);
    } else {
        chunk_t *chunk = get_data_chunk(config.chunk_file, pkt);
        int num = 0;
        packet **pkts = chunk2pkts(chunk, &num);
        sdr = add_sender(sender_pool, peer, pkts, num);
        //建立连接，调用process_sender处理数据包
        //新建一个线程发送该chunk对应的DATA数据包
        pthread_t t_sender;
        if (pthread_create(&t_sender, NULL, (void *(*)(void *)) process_sender, sdr) == -1) {
            puts("Fail to upload！");
            exit(-1);
        }
//       void *res;
//        if (pthread_join(t_sender, &res) == -1) {
//            exit(-1);
//        }
    }
}

void process_DATA(packet *pkt, bt_peer_t *peer) {
    receiver *rcvr = get_receiver(receiver_pool, peer);
    if (rcvr == NULL)
        return;
    if (pkt->header.seq_num == rcvr->next_expected) {
        int data_size = pkt->header.total_pkt_len - pkt->header.header_len;
        memcpy(rcvr->chunk->data + rcvr->last_rcvd * DATA_SIZE, pkt->data, (size_t) data_size);
        rcvr->last_rcvd = rcvr->next_expected;
        rcvr->next_expected++;
    }
    packet *pkt_ack = make_ACK(rcvr->last_rcvd);
    send_PKT(sock, peer, pkt_ack);
    if (rcvr->last_rcvd * DATA_SIZE >= BT_CHUNK_SIZE) {//下载完成
        update_state(down_chunks, rcvr->chunk, FINISHED);   //更新下载状态为【完成】
        remove_receiver(receiver_pool, rcvr);
    }
    free(pkt_ack);
}

void process_ACK(packet *pkt, bt_peer_t *peer) {
    sender *sdr = get_sender(sender_pool, peer);
    if (sdr == NULL)
        return;
    if (pkt->header.ack_num < 0)
        return;
//    printf("Last Ack Number: %d, Ack Number: %d\n", sdr->last_acked, pkt->header.ack_num);
    if (pkt->header.ack_num >= sdr->pkt_num) {//发送完成
        if (sdr->last_sent < sdr->pkt_num)
            return;
        puts("Finished");
        stop_timer(sdr);
        remove_sender(sender_pool, sdr);
        return;
    }
    if (pkt->header.ack_num > sdr->last_acked) {
        //使用累计确认原则，当ack_num大于上次确认的ack_num，则更新发送窗口信息，process_sender捕捉到空闲窗口则发送新的数据包
        sdr->last_acked = pkt->header.ack_num;
        sdr->last_available = (uint32_t) (sdr->last_acked + sdr->cwnd);
        sdr->dup_ack_num = 1;   //第一次收到（有效）
        if (sdr->cwnd <= sdr->ssthresh) {
            sdr->cwnd++;    //慢启动指数增长
            cwnd2log(sdr->p_receiver->id, (int) sdr->cwnd);
        } else {
            if ((int) sdr->cwnd + 1 < sdr->cwnd + 1 / sdr->cwnd) {
                sdr->cwnd += 1 / sdr->cwnd;
                cwnd2log(sdr->p_receiver->id, (int) sdr->cwnd);
            } else sdr->cwnd += 1 / sdr->cwnd;
        }
    } else if (pkt->header.ack_num == sdr->last_acked) {
        sdr->dup_ack_num++;
        if (sdr->dup_ack_num >= DUP_ACK_NUM) {
            //接收到重复的ack_num，将last_sent指向最早未确认的包，process_sender根据新的last_sent重发所有未确认数据
            retransmit(sdr, pkt->header.ack_num);
            cwnd2log(sdr->p_receiver->id, (int) sdr->cwnd);
        }
    }

    stop_timer(sdr);
}

void process_DENIED(packet *pkt, bt_peer_t *peer) {
    //rejected
    //do nothing
}

void *process_sender(sender *sdr) {
    puts("Uploading");
    if (log_file == NULL) {
        log_file = fopen("problem2-peer.txt", "a+");
        gettimeofday(&starter, NULL);
    }
    init_timer(sdr->timer, sdr->last_acked);
    start_timer(sdr);
    struct timeval timer;
    while (1) {
        timer.tv_sec = 0;
        timer.tv_usec = 100;
        if (select(0, NULL, NULL, NULL, &timer) >= 0) {
            if (sdr->last_sent < sdr->last_available && sdr->last_sent < sdr->pkt_num) {//不断（每隔0.1毫秒）检测是否有空闲窗口，若有则发送数据包
                if (sdr->last_sent == sdr->last_acked) { //对发送窗口的第一个数据包设置计时器
                    init_timer(sdr->timer, sdr->last_acked);
                    start_timer(sdr);
                }
                send_PKT(sock, sdr->p_receiver, sdr->pkts[sdr->last_sent++]);
            }
        }
        if (sdr->last_acked >= sdr->pkt_num) {//发送完毕
            break;
        }
    }
    if (log_file != NULL) {
        fflush(log_file);
        fclose(log_file);
    }
    return NULL;
}

void *pthread_receiver(void *arg) {
    struct timeval timeout;
    puts("Downloading");
    while (!is_empty(down_chunks)) {    //每隔1秒检查一次下载任务是否完成，未完成则调用look_at()下载
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        if (select(0, NULL, NULL, NULL, &timeout) >= 0) {
            look_at();
        }
    }
    chunks2file(); //将接收完成的数据输出到outputfile
    puts("Finished");
    return NULL;
}

void chunks2file() {
    if (!is_empty(down_chunks) || is_empty(done_chunks)) {
        return;
    }
    chunk_t *data_chunk;
    FILE *fp;
    if ((fp = fopen(config.output_file, "w+")) == NULL) {
        printf("Fail to create %s\n", config.output_file);
        exit(0);
    }
    while (done_chunks->n) {
        data_chunk = (chunk_t *) dequeue(done_chunks);
//        if (check_chunk(data_chunk, data_chunk->sha1)) {//检查数据是否正确
        fseek(fp, data_chunk->id * BT_CHUNK_SIZE, SEEK_SET);
        fwrite(data_chunk->data, BT_CHUNK_SIZE, 1, fp);
//        } else break;
        free_chunk(data_chunk);
    }
    fclose(fp);
    free(receiver_pool);
    free_queue(down_chunks);
    free_queue(done_chunks);
}

void look_at() {
    scan_chunk_done(down_chunks, done_chunks);//将下载完成的chunk从down_chunks移除，添加到done_chunks队列
    if (receiver_pool->cur_num >= receiver_pool->max_num) {
        return; //等待
    }
    if (!is_empty(down_chunks)) {
        chunk_t *down_chunk = choose_chunk_to_download(down_chunks);//选择可下载块
        assert(down_chunk != NULL);
        //选择一个peer源下载
        bt_peer_t *peer = NULL;
        node *h_node = down_chunk->holders->head;
        while (h_node) {
            peer = (bt_peer_t *) h_node->data;
            if (get_receiver(receiver_pool, peer) == NULL) {//选择一个没建立连接的peer
                break;
            }
            h_node = h_node->next;
        }
        if (get_receiver(receiver_pool, peer) != NULL)  //该链接正在使用
            return;
        if (peer == NULL) {//可能没接收到拥有这个chunk的peer的IHAVE数据包
            return;
        }
        add_receiver(receiver_pool, peer, down_chunk);//建立连接，指定需要下载的chunk以及发送方
        update_state(down_chunks, down_chunk, DOWNLOADING);  //更新chunk状态为【下载中】
        uint8_t *bin_sha1 = (uint8_t *) malloc(SHA1_HASH_SIZE);
        ascii2hex(down_chunk->sha1, sizeof(down_chunk->sha1), bin_sha1);
        packet *pkt_get = make_GET(bin_sha1);
        send_PKT(sock, peer, pkt_get);
        free(bin_sha1);
        free(pkt_get);
    }
}

void cwnd2log(int id, int cwnd) {
    gettimeofday(&now, NULL);
    int ms = (int) ((now.tv_sec - starter.tv_sec) * 1000 + (now.tv_usec - starter.tv_usec) / 1000);
    LOG(id, ms, cwnd);
}

int correct(packet *pkt) {
    pkt_header *header = &pkt->header;
    if (header->magic_num != MAGIC || header->version_num != VERSION || header->packet_type > DENIED ||
        header->packet_type < WHOHAS)
        return 0;
    pkt_type type = pkt_parse(pkt);
    if ((type == DATA && (header->seq_num <= 0 || header->ack_num > 0))
        || (type == ACK && (header->seq_num > 0 || header->ack_num < 0))
        || (type != DATA && type != ACK && (header->seq_num > 0 || header->ack_num > 0)))
        return 0;
    return 1;
}

int not_correct(packet *pkt) {
    return !correct(pkt);
}

bt_peer_t *get_peer(struct sockaddr_in *addr) {
    bt_peer_t *p;
    for (p = config.peers; p != NULL; p = p->next) {
        if (p->addr.sin_port == addr->sin_port) {
            return p;
        }
    }
    return NULL;
}