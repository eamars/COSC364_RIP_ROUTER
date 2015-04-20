#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>

#define RIP_REQUEST 1
#define RIP_RESPONSE 2
#define RIP_VERSION_1 1
#define RIP_VERSION_2 2

typedef struct rip_packet_s
{
	unsigned int command;
	unsigned int version;
	unsigned int AFI;
	unsigned int address;
	unsigned int next_hop;
	unsigned int metric;
} RIPPacket;



static const char *RIP_PACKET_FORMAT = "%01x%01x00%02x00%04x0000%04x%04x";

int rip_packet_decode(char *message, RIPPacket *packet)
{
	if (strlen(message) != 24)
	{
		return -1;
	}

	return sscanf(message, RIP_PACKET_FORMAT,
		&packet->command,
		&packet->version,
		&packet->AFI,
		&packet->address,
		&packet->next_hop,
		&packet->metric
	) != 6;
}


int rip_packet_encode(char *message, RIPPacket *packet)
{
	sprintf(message, RIP_PACKET_FORMAT,
		packet->command,
		packet->version,
		packet->AFI,
		packet->address,
		packet->next_hop,
		packet->metric
	);

	return 0;
}

void debug_print_rip_packet(RIPPacket *packet)
{
	printf("command = %d\nversion = %d\nAFI = %d\naddress = %d\nnext_hop = %d\nmetric = %d\n",
		packet->command,
		packet->version,
		packet->AFI,
		packet->address,
		packet->next_hop,
		packet->metric
	);
}

int main(int argc, char **argv)
{
	char buf[25];
	RIPPacket p;
	p.command = RIP_REQUEST;
	p.version = RIP_VERSION_1;
	p.AFI = AF_INET;
	p.address = atoi(argv[1]);
	p.next_hop = atoi(argv[2]);
	p.metric = atoi(argv[3]);

	rip_packet_encode(buf, &p);
	printf("raw: %s\n", buf);
	debug_print_rip_packet(&p);
	return 0;
}
