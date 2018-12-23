# README

16302010059 张健

标签（空格分隔）： 计算机网络 PROJECT

* cp1: passed
* cp2: passed
* cp3: passed
* concurrenttest: passed

---

## 架构（新增）

* `tracker.c` 管理对等方的chunks信息，提供查询接口，以及`chunk`数据结构和相关接口
* `rcv_send.c` 管理接收方和发送方信息，处理重发（GBN），计时
* `handler.c` 处理接收到的各种数据包，发送数据包，拥塞控制
* `packet.c` 数据包结构，生成各种数据包
* `queue.c` 队列，方便存储各种链式数据

---

## 处理过程

* `peer.c`接收配置信息，初始化config
* 不断监听输入
    * 客户端输入`GET XXX XXX`指令：更新config信息，调用`process_download`函数处理下载任务
    * 另一对等方请求：接收数据包，调用`process_PKT`函数处理数据包

### 客户端下载过程

* 客户端调用`process_download`函数
* 初始化连接池、下载队列`down_chunks`（需下载的chunks）
* 向其他（除自己）的对等方发送`WHOHAS`数据包
* 新建一个线程`process_receiver`下载所有down_chunks

#### 客户端接收`IHAVE`数据包

* 根据数据包包含的hash值
* 更新下载队列中对应的chunk的拥有者信息
* `process_receiver`线程函数每隔1秒检查`down_chunks`队列是否有可下载的chunk
> 根据最稀缺块优先下载原则：
> 该块至少有一个来源且来源最稀少，下载状态为`READY`，

#### 客户端接收`DATA`数据包

判断该数据包的`seq_num`是否是当前连接期待接收到的`next_expected`数据包

* 是，更新连接窗口信息，发送`ACK`数据包
* 否，发送`ACK`数据包（重复的ACK）
* 判断是否下载完成
* 释放完成下载的连接
* 将数据写入文件

#### 判断是否下载完成
为了下载的健壮性，如果得到的数据不正确，需要从其他对等方处下载：
```java
    if (rcvr->last_rcvd * DATA_SIZE >= BT_CHUNK_SIZE) {//下载完成
        if (check_chunk(rcvr->chunk, rcvr->chunk->sha1)) {
            //更新下载状态为【完成】
            update_state(down_chunks, rcvr->chunk, FINISHED);
        } else {
            //数据不正确，将下载状态设为【准备下载】，重新获取
            update_state(down_chunks, rcvr->chunk, READY);
        }
        //断开该连接
        remove_receiver(receiver_pool, rcvr);
    }
```
    
---

### 上传过程 

#### 对等方接收`WHOHAS`数据包 

* 查询拥有的`chunk`信息 
* 将满足`WHOHAS`数据包要求的`chunk`哈希值打包到一个`IHAVE`数据包 

> 因为满足的chunk数必定小于等于需要的chunk数
> 因此只需要一个IHAVE数据包

#### **对等方接收`GET`数据包** 

* 当请求的`GET`数达到最大值，拒绝连接，发送`DENIED`数据包
* 否则根据哈希值得到对应`chunk`数据
    * 检查是否已经发送最后一个数据包，若是则释放完成上传的连接
    * 将数据打包成`DATA`数据包数组
    * 建立连接，新建一个线程`process_sender`发送数据包数组

#### **对等方接收`ACK`数据包**

使用"累计确认"原则

* 当`ack_num`大于上次确认的ack_num，则更新发送窗口信息
* `process_sender`线程函数捕捉到空闲窗口则发送新的数据包

#### **拥塞控制**

**接收到合法ACK：**

a. 拥塞窗口小于窗口阈值，即慢启动阶段。每收到一个ACK拥塞窗口加一，即指数增长

* 初始时，cwnd = 1
* 接收到一个ACK，cwnd = 2
* 接收到两个ACK，cwnd = 4
* 接收到四个ACK，cwnd = 8
* ……

b. 拥塞窗口大于等于窗口阈值，即线性增长阶段。
   每收到一个ACK，拥塞窗口增加`1 / 拥塞窗口大小`

```java
if (pkt->header.ack_num > sdr->last_acked) {
        //使用累计确认原则，当ack_num大于上次确认的ack_num
        //则更新发送窗口信息，process_sender线程捕捉到空闲窗口则发送新的数据包
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
            //接收到重复的ack_num，将last_sent指向最早未确认的包
            //process_sender线程根据新的last_sent重发所有未确认数据
            retransmit(sdr, pkt->header.ack_num);
            cwnd2log(sdr->p_receiver->id, (int) sdr->cwnd);
        }
    }
```
   
**重传**
两种情况：

* 接收到重复ACK数超过`DUP_ACK_NUM`
* 超时

调用`retransmit`函数执行超时重发（GBN）

* 在慢启动阶段超时或收到3个重复ACK，则重新开始慢启动
* 其他情况，使用`TCP Reno`策略快速恢复

```java
void retransmit(sender *sdr, uint32_t last_acked) {
    sdr->last_sent = last_acked;
    sdr->last_acked = last_acked;
    sdr->dup_ack_num = 1;
    if (sdr->cwnd <= sdr->ssthresh) { // 重新进入慢启动阶段
        sdr->ssthresh = (uint32_t) sdr->cwnd;
        sdr->cwnd = 1;
    } else {    //TCP Reno策略，快速恢复
        sdr->ssthresh = (uint32_t) (sdr->cwnd / 2);
        sdr->cwnd = sdr->ssthresh + DUP_ACK_NUM;
    }
    sdr->last_available = (uint32_t) (sdr->last_sent + sdr->cwnd);
}
```

* process_sender线程会不断检测是否有空闲窗口
* 当last_sent被重置时，将重发所有未被确认的DATA数据包

```java
void *process_sender(sender *sdr){ //一个子线程
//……
//……
    while (1) {
        timer.tv_sec = 0;
        timer.tv_usec = 100;
        if (select(0, NULL, NULL, NULL, &timer) >= 0) {
            if (sdr->last_sent < sdr->last_available && sdr->last_sent < sdr->pkt_num) {
            //不断（每隔0.1毫秒）检测是否有空闲窗口，若有则发送数据包
                if (sdr->last_sent == sdr->last_acked) { 
                //对发送窗口的第一个数据包设置计时器
                    init_timer(sdr->timer, sdr->last_acked);
                    start_timer(sdr);
                }
                send_PKT(sock, sdr->p_receiver, sdr->pkts[sdr->last_sent++]);
            }
        }
        if (sdr->last_acked >= sdr->pkt_num) {//发送完毕
            break;
        }
//……
//……
```

* 拥塞窗口变化信息输出到`problem2-peer.txt`文件
* 使用多线程处理的缘故，拥塞窗口变化太快
* `fprintf`函数的花费较高，因此不能及时输出所有拥塞窗口变化的信息

对连续输出相同拥塞窗口大小的解释：

* 当超时或接收到3个重复ACK时，重发
* 若`cwnd == cwnd / 2 + 3`，则变化前后，输出相同拥塞窗口状态
```
……
peer2	time = 4001	cwnd = 5
peer2	time = 4027	cwnd = 5
peer2	time = 4036	cwnd = 5
peer2	time = 4041	cwnd = 5
peer2	time = 4066	cwnd = 2
peer2	time = 4071	cwnd = 3
peer2	time = 4076	cwnd = 4
peer2	time = 4080	cwnd = 5
peer2	time = 4085	cwnd = 6
peer2	time = 4089	cwnd = 7
peer2	time = 4093	cwnd = 8
peer2	time = 4097	cwnd = 9
peer2	time = 4101	cwnd = 10
……

```
---




