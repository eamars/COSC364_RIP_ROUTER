#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "rip_message.h"

static const char *RIP_PACKET_FORMAT_HEADER = "%01d%01d%02d";
static const char *RIP_PACKET_FORMAT_ENTRY = "%02d00%04d0000%04d%04d";

int rip_packet_decode(char *message, RIPPacket *packet)
{
	char buf[21];

	int len = strlen(message);

	if ((len - 4) % 20 != 0)
	{
		// unknown length
		return -1;
	}

	int n_entry = (len - 4) / 20;

	packet->n_entry = n_entry;

	memcpy(buf, message, 4);

	sscanf(buf, RIP_PACKET_FORMAT_HEADER,
		&packet->command,
		&packet->version,
		&packet->senderid
	);

	for (int i = 0; i < n_entry; i++)
	{
		memcpy(buf, message + 20 * i + 4, 20);
		sscanf(buf, RIP_PACKET_FORMAT_ENTRY,
			&packet->entry[i].AFI,
			&packet->entry[i].address,
			&packet->entry[i].next_hop,
			&packet->entry[i].metric
		);
		if (packet->entry[i].metric < 1 || packet->entry[i].metric > 16)
		{
			// bad metric
			return -2;
		}
	}

	return 0;
}


int rip_packet_encode(char *message, RIPPacket *packet)
{
	char buf[21];

	sprintf(message, RIP_PACKET_FORMAT_HEADER,
		packet->command,
		packet->version,
		packet->senderid
	);
	for (unsigned int i = 0; i < packet->n_entry; i++)
	{
		sprintf(buf, RIP_PACKET_FORMAT_ENTRY,
			packet->entry[i].AFI,
			packet->entry[i].address,
			packet->entry[i].next_hop,
			packet->entry[i].metric
		);
		strcat(message, buf);

	}

	return 0;
}

void debug_print_rip_packet(const RIPPacket *packet)
{
	printf("command = %d\nversion = %d\nsender = %d\n",
		packet->command,
		packet->version,
		packet->senderid
	);
	for (unsigned int i = 0; i < packet->n_entry; i++)
	{
		printf("Entry[%d]\n	AFI = %d\n	address = %d\n	next_hop = %d\n	metric = %d\n",
			i,
			packet->entry[i].AFI,
			packet->entry[i].address,
			packet->entry[i].next_hop,
			packet->entry[i].metric
		);
	}
}

