/**
 * packet.h
 *
 * Structure for packet.
 *
 * @author Jon Zhang <16302010059@fudan.edu.cn>
 * @date 2018-11-20
 */

#ifndef A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_PACKET_H
#define A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_PACKET_H

#include <inttypes.h>

#define PKT_SIZE 1500      /**< The maximum packet size for UDP is 1500 bytes. **/
#define HDR_SIZE 16        /**< The header length. **/
#define DATA_SIZE 1400   /**< The maximum data size. **/

#define MAGIC 15441        /**< The magic number should be 15441 **/
#define VERSION 1          /**< The version number should be 1 **/
#define INIT_SEQ 0
#define INIT_ACK 0

#define pkt_parse(pkt) pkt_parse_type((pkt)->header.packet_type)

typedef enum {
    HOST, NET
} value_type;

typedef enum {
    IGNORE, QUEUE, NOTIFY
} handle_pkt_lost;

typedef enum {
    WHOHAS = 0, IHAVE, GET, DATA, ACK, DENIED
} pkt_type;

/**
 * 1. Magic Number [2 bytes]
 * 2. Version Number [1 byte]
 * 3. Packet Type [1 byte]
 * 4. Header Length [2 bytes]
 * 5. Total Packet Length [2 bytes]
 * 6. Sequence Number [4 bytes]
 * 7. Acknowledgment Number [4 bytes]
 * All integers must be unsigned
 * All multi_byte integer fields must be transmitted in network byte order.
 *
 * > The magic number should be 15441, the version number should be 1.
 * > Peers should drop packet that do not have these values.
 */
typedef struct {
    uint16_t magic_num;       /**< MAGIC **/
    uint8_t version_num;      /**< VERSION **/
    uint8_t packet_type;      /**< pkt_type **/
    uint16_t header_len;
    uint16_t total_pkt_len;
    uint32_t seq_num;         /**< Used for congestion control and reliable transmission **/
    uint32_t ack_num;         /**< Used for reliable transmission **/
} pkt_header;

typedef struct {
    pkt_header header;
    uint8_t data[0];
} packet;

/**
 * magic_num = MAGIC, version_num = VERSION, header_len = HDR_SIZE
 */
void init(packet *, uint8_t, uint16_t, uint32_t, uint32_t, uint8_t *);

packet *make_PKT(uint8_t type, uint32_t seq_ack, uint32_t data_size, uint8_t *data);

void convert(packet *, value_type);

/**
 * WHOHAS contains:
 * the number of chunk hashes(1 byte)
 * 3 bytes of empty padding space to keep the chunk 32-bit aligned
 * Both seq num and ack num have no meaning, that is, they are invalid.
 */
packet *make_WHOHAS(uint32_t data_size, uint8_t *data);

/**
 * IHAVE is familiar with WHOHAS.
 */
packet *make_IHAVE(uint32_t data_size, uint8_t *data);

/**
 * The payload of GET contains only the chunk hash for the chunk the client wants to fetch(20 bytes)
 * Both seq num and ack num are invalid.
 */
packet *make_GET(uint8_t *data);

/**
 * DATA packets do not have any payload format defined
 * normally they should just contain file data.
 * The ack num should be invalid. (Do not use a DATA packet to acknowledge a previous packet)
 */
packet *make_DATA(uint32_t seq_ack, uint32_t data_size, uint8_t *data);

/**
 * ACK packet does not contain any data.
 * The seq num should be invalid. (Do not send data in a ACK packet)
 */
packet *make_ACK(uint32_t seq_ack);

pkt_type pkt_parse_type(uint8_t type);


#endif //A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_PACKET_H
