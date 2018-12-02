/**
 * handler.c
 *
 * @author Jon Zhang <16302010059@fudan.edu.cn>
 * @date 2018-11-21
 */
#include "handler.h"
#include "spiffy.h"
#include "packet.h"
#include <stdlib.h>
#include <assert.h>
#include <memory.h>

bt_config_t config;
int sock;
queue *down_chunks;//需要下载的chunk
queue *done_chunks;//下载完成的chunk

sender_pool_t *sender_pool;
receiver_pool_t *receiver_pool;

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

void set_configs(bt_config_t c, int s) {
    config = c;
    sock = s;
}

bt_peer_t *get_peer(struct sockaddr_in addr) {
    bt_peer_t *p;
    for (p = config.peers; p != NULL; p = p->next) {
        if (p->addr.sin_port == addr.sin_port) {
            return p;
        }
    }
    return NULL;
}

void output() {
    if (!is_empty(down_chunks) || is_empty(done_chunks)) {
        return;
    }
    chunk_t *data_chunk;
    while (done_chunks->n) {
        data_chunk = (chunk_t *) dequeue(done_chunks);
        if (check_chunk(data_chunk, data_chunk->sha1)) {//检查数据是否正确
            FILE *fp = fopen(config.output_file, "rw");
            fseek(fp, data_chunk->id * BT_CHUNK_SIZE, SEEK_SET);
            fwrite(data_chunk->data, sizeof(data_chunk->data), 1, fp);
        } else break;
        free_chunk(data_chunk);
    }
    free_queue(down_chunks);
    free_queue(done_chunks);
}

void look_at() {
    scan_chunk_done(down_chunks, done_chunks);//将下载完成的chunk从down_chunks移除，添加到done_chunks队列
    if (receiver_pool->cur_num >= receiver_pool->max_num) {
        return; //等待
    }
    if (!is_empty(down_chunks)) {
        chunk_t *down_chunk = choose_chunk_to_download(down_chunks);
        assert(down_chunk != NULL);
        //选择一个peer源下载
        bt_peer_t *peer = NULL;
        node *h_node = down_chunk->holders->head;
        while (h_node) {
            peer = (bt_peer_t *) h_node->data;
            if (get_receiver(receiver_pool, peer) == NULL)  //选择一个没建立连接的peer
                break;
            h_node = h_node->next;
        }
        if (peer == NULL)   //可能没接收到拥有这个chunk的peer的IHAVE数据包
            return;
        receiver *r = add_receiver(receiver_pool, peer, down_chunk);//建立连接，指定需要下载的chunk以及发送方
        update_state(down_chunks, down_chunk, DOWNLOADING);  //更新chunk状态为【下载中】
        uint8_t *bin_sha1 = (uint8_t *) malloc(SHA1_HASH_SIZE);
        ascii2hex(down_chunk->sha1, sizeof(down_chunk->sha1), bin_sha1);
        packet *pkt_get = make_GET(bin_sha1);
        send_PKT(sock, peer, pkt_get);
        start_timer(r->timer, 0);   //启动定时器
        free_packet(pkt_get);
    }
}

void send_PKT(int sock, bt_peer_t *peer, packet *pkt) {
    if (peer != NULL) {
        convert(pkt, NET);
        if (spiffy_sendto(sock, pkt, ntohs(pkt->header.total_pkt_len), 0, (struct sockaddr *) &peer->addr,
                          sizeof(peer->addr)) != -1)
            puts("成功发送数据包");
        convert(pkt, HOST);
    }
}

void send_PKTS(int sock, bt_peer_t *peer, queue *pkts) {
    node *cur_node = pkts->head;
    while (cur_node) {
        send_PKT(sock, peer, (packet *) cur_node->data);
        cur_node = cur_node->next;
    }
}

void process_sender(sender *sdr) {
    start_timer(sdr->timer, 0);
    struct timeval timer;
    timer.tv_sec = 1;
    timer.tv_usec = 0;
    while (1) {
        if (select(0, NULL, NULL, NULL, &timer) == 0) {
            if (sdr->last_sent < sdr->last_available && sdr->last_sent < sdr->pkt_num) {
                send_PKT(sock, sdr->p_receiver, sdr->pkts[++sdr->last_sent]);
            }
        }
        if (sdr->last_sent >= sdr->pkt_num)//发送完毕
            break;
    }
}

void process_download(bt_config_t c, int s) {
    if (c.chunk_file == NULL || c.output_file == NULL)
        return;
    set_configs(c, s);
    receiver_pool = malloc(sizeof(receiver_pool_t));
    sender_pool = malloc(sizeof(sender_pool_t));
    init_receiver_pool(receiver_pool, config.max_conn);
    init_sender_pool(sender_pool, config.max_conn);

    queue *queue_chunk = list_chunks(config.chunk_file);
    down_chunks = queue_chunk;

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
    struct timeval timer;
    timer.tv_sec = 2;
    timer.tv_usec = 0;
    while (!is_empty(down_chunks)) {    //每隔两秒检查一次下载任务是否完成，未完成则调用look_at()下载
        switch (select(0, NULL, NULL, NULL, &timer)) {
            case 0:
                look_at();
            default:
                break;
        }
    }

    output(); //将接收完成的数据输出到outputfile

}

void process_PKT(packet *pkt, struct sockaddr_in from) {
    puts("接收到数据包");
    convert(pkt, HOST);
    if (not_correct(pkt)) {
        puts("数据包损坏");
        return;
    }
    bt_peer_t *peer = get_peer(from);
    pkt_type type = pkt_parse_type(pkt->header.packet_type);
    switch (type) {
        case WHOHAS:
            puts("处理WHOHAS");
            process_WHOHAS(pkt, peer);
            break;
        case IHAVE:
            puts("处理IHAVE");
            process_IHAVE(pkt, peer);
            break;
        case GET:
            puts("处理GET");
            process_GET(pkt, peer);
            break;
        case ACK:
            puts("处理ACK");
            process_ACK(pkt, peer);
            break;
        case DATA:
            puts("处理DATA");
            process_DATA(pkt, peer);
            break;
        case DENIED:
            puts("处理DENIED");
            process_DENIED(pkt, peer);
            break;
        default:
            puts("没有对应类型");
            break;
    }
}

void process_WHOHAS(packet *pkt, bt_peer_t *peer) {
    queue *who_has_chunks = pkt2chunks(pkt, WHOHAS);
    queue *i_have_chunks = which_i_have(who_has_chunks, config.has_chunk_file);
    if (i_have_chunks != NULL && !is_empty(i_have_chunks)) {
        queue *i_have_pkts = chunks2pkts(i_have_chunks, IHAVE);
        send_PKTS(sock, peer, i_have_pkts);
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
    look_at();
}

void process_GET(packet *pkt, bt_peer_t *peer) {
    sender *sdr = get_sender(sender_pool, peer);
    //来自当前peer的上一个GET请求处理完毕，关闭连接
    if (sdr != NULL && sdr->last_sent == sdr->pkt_num)
        remove_sender(sender_pool, sdr);
    else if (sdr != NULL)
        return;
    if (sender_pool->cur_num >= sender_pool->max_num) {
        packet *pkt_denied = make_PKT(DENIED, 0, 0, NULL);
        send_PKT(sock, peer, pkt_denied);
        free_packet(pkt_denied);
    } else {
        chunk_t *chunk = get_data_chunk(config.chunk_file, pkt);
        packet **pkts = chunk2pkts(chunk);
        sdr = add_sender(sender_pool, peer, pkts);
        process_sender(sdr);
    }
}

void process_DATA(packet *pkt, bt_peer_t *peer) {
    receiver *rcvr = get_receiver(receiver_pool, peer);
    if (rcvr == NULL)
        return;
    if (pkt->header.seq_num == rcvr->next_expected) {
        int data_size = pkt->header.total_pkt_len - pkt->header.header_len;
        memcpy(rcvr->chunk->data + pkt->header.seq_num, pkt->data, (size_t) data_size);
        rcvr->last_rcvd = rcvr->next_expected;
        rcvr->next_expected += data_size;
    }
    packet *pkt_ack = make_ACK(rcvr->last_rcvd);
    send_PKT(sock, peer, pkt_ack);
    start_timer(rcvr->timer, rcvr->next_expected);
    if (rcvr->next_expected % PKT_SIZE != 0 || rcvr->next_expected > BT_CHUNK_SIZE) {//下载完成
        update_state(down_chunks, rcvr->chunk, FINISHED);   //更新下载状态为【完成】
        remove_receiver(receiver_pool, rcvr);
    }
}

void process_ACK(packet *pkt, bt_peer_t *peer) {
    sender *sdr = get_sender(sender_pool, peer);
    if (sdr == NULL)
        return;
    if (pkt->header.ack_num > (sdr->pkt_num - 1) * PKT_SIZE)//发送完成
        remove_sender(sender_pool, sdr);
    if (pkt->header.ack_num > sdr->last_acked) {
        sdr->last_acked = pkt->header.ack_num;
        sdr->last_available = sdr->last_acked + sdr->win_size;
        sdr->dup_ack_num = 1;   //第一次收到（有效）
    } else if (pkt->header.ack_num == sdr->last_acked) {
        sdr->dup_ack_num++;
        if (sdr->dup_ack_num >= DUP_ACK_NUM) {
            sdr->last_sent = pkt->header.ack_num;
            sdr->dup_ack_num = 1;
        }
    }
}

void process_DENIED(packet *pkt, bt_peer_t *peer) {
    //rejected
    //do nothing
}