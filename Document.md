# Document 

标签（空格分隔）： 计算机网络

---

## 架构（新增）

* `tracker.c` 记录每个对等方有某一文件的哪些chunks，提供查询接口，以及`chunk`数据结构和相关接口
* `rcv_send.c` 管理接收方和发送方信息
* `handler.c` 处理接收到的各种数据包，发送数据包
* `packet.c` 数据包结构，生成各种数据包
* `queue.c` 队列，方便存储各种链式数据
* `timer.c` 定时器，提供启动、关闭计时器的接口。

## 处理过程

* `peer.c`接收配置信息，初始化config
* 不断监听输入
    * 客户端输入`GET XXX XXX`指令，更新config信息，调用`process_download`函数处理下载任务
    * 另一对等方请求，接收数据包，调用`process_PKT`函数处理数据包

### 客户端下载过程

* 客户端`process_download`函数
* 初始化连接池、下载队列`down_chunks`（需下载的chunks）
* 向其他（除自己）的对等方发送`WHOHAS`数据包
* 客户端接收`IHAVE`数据包
* 根据数据包包含的hash值
* 更新下载队列中对应的chunk的拥有者信息
> 即添加发送者到拥有此chunk的对等方队列 

* `process_download`函数每隔1~2秒
* 检查`down_chunks`队列是否有可下载的chunk
> 最稀缺优先下载、至少有一个拥有者、该chunk的状态为`READY`

* 客户端接收`DATA`数据包
* 判断该数据包的`seq_num`是否是当前连接期待接收到的`next_expected`数据包
    * 是，更新连接窗口信息，发送`ACK`数据包
    * 否，发送`ACK`数据包（重复的ACK）
    * 判断是否下载完成
    * 即接收到的数据包是最后一个（数据少于1500-16字节）
    * 或者下一个期待收到的超过`chunk`的大小（512kB)
    * 释放完成下载的连接

### 其他对等方上传过程 

#### 对等方接收`WHOHAS`数据包 

* 查询拥有的`chunk`信息 
* 将满足`WHOHAS`数据包要求的`chunk`哈希值打包到一个`IHAVE`数据包 

> 因为满足的chunk数必定小于等于需要的chunk数
> 因此只需要一个数据包

#### **对等方接收`GET`数据包** 

* 当请求的`GET`数达到最大值，拒绝连接，发送`DENIED`数据包
* 否则根据哈希值得到对应`chunk`数据
    * 检查是否已经发送最后一个数据包，若是则释放完成上传的连接
    * 将数据打包成`DATA`数据包数组
    * 建立连接，调用`process_sender`处理数据包
    * 当发送窗口有空闲时，发送`last_sent`指向的下一个数据包
        *  根据`ACK`数据包更新窗口信息
        *  接收到重复`ACK`达三次，则重发所有未确认的数据包
        > 将last_sent指向last_acked 


#### **对等方接收`ACK`数据包**

**使用"累计确认"原则**

* 当`ack_num`大于上次确认的ack_num，则更新发送窗口信息
* `process_sender`捕捉到空闲窗口则发送新的数据包

**接收到重复的`ack_num`**

* 将`last_sent`指向最早未确认的包，`process_sender`根据新的`last_sent`重发所有未确认数据包
* 检查是否是最后一个`ACK`，若是则释放完成上传的连接

## 疑问

### **cp2无法通过**

命令行输出：
```
成功发送数据包
Peer 1 received WHO_HAS with 2 hashes 
id = -1, hash = 6acfce07d222d400ce900d918306c6664501b33f 
id = -1, hash = af91dbd47aac60e980e0369df68271ca52de11a8 
found 2 matches: 
id = 0, hash = 6acfce07d222d400ce900d918306c6664501b33f 
id = 1, hash = af91dbd47aac60e980e0369df68271ca52de11a8 
creating Control packet of type 1 with 2 chunks 
Peer 1 Sending IHAVE of size 60 to 0.0.0.0:2222
```
分析: 

* 直接调用`spiffy_sendto`
* 向`127.0.0.1:2222`所在的peer发送IHAVE数据包 
* 命令行显示，没有获取到正确的地址
* 而是向`0.0.0.0:2222`对应的peer发送数据包 
* 测试cp1时也有向`127.0.0.1:2222`发送数据包
* 但cp1能正确发送数据 
* 在调用`spiffy_sendto`前输出peer的地址，得到正确的地址（peer->addr.sin_addr.s_addr）
* 也就是说，发送方式应该是没问题的。。。 


```
void send_PKT(int sock, bt_peer_t *peer, packet *pkt) {
    if (peer != NULL) {
        convert(pkt, NET);
        if (spiffy_sendto(sock, pkt, ntohs(pkt->header.total_pkt_len), 0, (struct sockaddr *) &peer->addr,
                          sizeof(peer->addr)) != -1)
            puts("成功发送数据包");
        convert(pkt, HOST);
    }
}
```

* 感觉project1的实现没什么问题，上面的测试结果还是不太懂。。。



