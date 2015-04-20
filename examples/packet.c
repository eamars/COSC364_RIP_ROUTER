#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>

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
	unsigned int n_entry;
	RIPEntry entry[MAX_RIP_ENTRY];

} RIPPacket;


static const char *RIP_PACKET_FORMAT_HEADER = "%01d%01d00";
static const char *RIP_PACKET_FORMAT_ENTRY = "%02d00%04d0000%04d%04d";

int rip_packet_decode(char *message, RIPPacket *packet)
{
	char buf[20];

	int len = strlen(message);
	printf("len = %d\n", len);
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
		&packet->version
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
	char buf[20];

	sprintf(message, RIP_PACKET_FORMAT_HEADER,
		packet->command,
		packet->version
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
	printf("command = %d\nversion = %d\n",
		packet->command,
		packet->version
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




int main(int argc, char **argv)
{
	if (!strcmp(argv[1], "encode"))
	{

		char buf[504];
		RIPPacket p;
		p.command = RIP_REQUEST;
		p.version = RIP_VERSION_2;
		p.n_entry = 1;

		p.entry[0].AFI = AF_INET;
		p.entry[0].address = 3;
		p.entry[0].next_hop = 0;
		p.entry[0].metric = 16;

		rip_packet_encode(buf, &p);
		printf("raw: %s\n", buf);
		debug_print_rip_packet(&p);

	}
	else if (!strcmp(argv[1], "decode"))
	{
		RIPPacket q;

		printf("raw: %s\n", argv[2]);
		rip_packet_decode(argv[2], &q);
		debug_print_rip_packet(&q);
	}

	return 0;
}
