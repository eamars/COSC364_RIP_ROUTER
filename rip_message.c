#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "rip_message.h"

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
		&packet->port,
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
		packet->port,
		packet->next_hop,
		packet->metric
	);

	return 0;
}

void debug_print_rip_packet(RIPPacket *packet)
{
	printf("command = %d\nversion = %d\nAFI = %d\nport = %d\nnext_hop = %d\nmetric = %d\n",
		packet->command,
		packet->version,
		packet->AFI,
		packet->port,
		packet->next_hop,
		packet->metric
	);
}

int main(void)
{
	char buf[25];
	RIPPacket p;
	RIPPacket q;

	p.command = RIP_REQUEST;
	p.version = RIP_VERSION_2;
	p.AFI = AF_INET;
	p.port = 10000;
	p.next_hop = 1;
	p.metric = 16;

	rip_packet_encode(buf, &p);
	printf("%s\n", buf);
	printf("%d\n", rip_packet_decode(buf, &q));

	debug_print_rip_packet(&q);
	printf("AF_INET = %d\n", AF_INET);
	return 0;
}
