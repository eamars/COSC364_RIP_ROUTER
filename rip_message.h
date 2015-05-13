// Library for parsing RIP packet
// The packet format is as follow
//
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  command (1)  |  version (1)  |       must be zero (2)        |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | address family identifier (2) |            tag (2)            |
// +-------------------------------+-------------------------------+
// |                        IPv4 address (4)                       |
// +---------------------------------------------------------------+
// |                        Subnet Mask (4)                        |
// +---------------------------------------------------------------+
// |                          next hop (4)                         |
// +---------------------------------------------------------------+
// |                           metric (4)                          |
// +---------------------------------------------------------------+
//
// command: 1 for request, 2 for response
// version: 1 for RIP-1, 2 for RIP-2
// address family identifier: AF_INET
// IPv4 address: router_id
// next_hop: via

#ifndef RIP_MESSAGE_H_
#define RIP_MESSAGE_H_

#define RIP_REQUEST 1
#define RIP_RESPONSE 2
#define RIP_VERSION_1 1
#define RIP_VERSION_2 2

#define MAX_RIP_ENTRY 25

typedef struct rip_entry_s
{
	unsigned int AFI;
	unsigned int address;
	unsigned int next_hop;
	unsigned int metric;
} RIPEntry;

typedef struct rip_packet_s
{
	unsigned int command;
	unsigned int version;
	unsigned int senderid;
	unsigned int n_entry;
	RIPEntry entry[MAX_RIP_ENTRY];

} RIPPacket;

/**
 * decode the raw rip packet
 * @param  message raw rip packet as string
 * @param  packet  output rip packet
 * @return         negative for unsuccessful, 0 for successful
 */
int rip_packet_decode(char *message, RIPPacket *packet);

/**
 * encode a rip packet
 * @param  message output raw rip packet as string
 * @param  packet  input rip packet
 * @return         0 for successful
 */
int rip_packet_encode(char *message, RIPPacket *packet);

/**
 * debug print rip packet
 * @param packet input rip packet
 */
void debug_print_rip_packet(const RIPPacket *packet);


#endif
