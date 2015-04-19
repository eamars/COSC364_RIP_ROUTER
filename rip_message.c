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

