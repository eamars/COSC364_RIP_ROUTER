#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "rip_message.h"

static const char *RIP_PACKET_FORMAT = "%01x%01x00%02x00%04x0000%04x%04x";

int rip_packet_decode(char *message, RIPPacket *packet)
{
	int recv_count;

	if (strlen(message) != 24)
	{
		return -1;
	}

	recv_count = sscanf(message, RIP_PACKET_FORMAT,
		&packet->command,
		&packet->version,
		&packet->AFI,
		&packet->address,
		&packet->next_hop,
		&packet->metric
	);

	// check if metric <= 16
	if (packet->metric > 16)
	{
		return -2;
	}

	return recv_count != 6;
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

