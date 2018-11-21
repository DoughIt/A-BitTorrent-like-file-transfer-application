/**
 * # Packet
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
#define DATA_SIZE (PKT_SIZE - HDR_SIZE)   /**< The maximum data size. **/

#define MAGIC 15441        /**< The magic number should be 15441 **/
#define VERSION 1          /**< The version number should be 1 **/

#define GET_NUM 6          /**< The maximum number of GET requests that a peer can tackle **/
#define WINDOW_SIZE 8      /**< Sliding window size **/
#define DUP_ACK_NUM 3      /**< To avoid confusion from re-ordering, a sender counts a packet lost only after 3 duplicate ACKs in a row **/
#define TIMEOUT 5

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
    uint8_t data[PKT_SIZE];
} pkt;
/**
 * WHOHAS contains:
 * the number of chunk hashes(1 byte)
 * 3 bytes of empty padding space to keep the chunk 32-bit aligned
 * Both seq num and ack num have no meaning, that is, they are invalid.
 */

/**
 * IHAVE is familiar with WHOHAS.
 */

/**
 * The payload of GET contains only the chunk hash for the chunk the client wnats to fetch(20 bytes)
 * Both seq num and ack num are invalid.
 */

/**
 * DATA packets do not have any payload format defined
 * normally they should just contain file data.
 * The ack num should be invalid. (Do not use a DATA packet to acknowledge a previous packet)
 */

/**
 * ACK packet does not contain any data.
 * The seq num should be invalid. (Do not send data in a ACK packet)
 */

#endif //A_BITTORRENT_LIKE_FILE_TRANSFER_APPLICATION_PACKET_H
