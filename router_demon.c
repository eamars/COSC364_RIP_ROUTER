#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>

#include "list.h"
#include "route_table.h"
#include "rip_message.h"
#include "pidlock.h"

#define BUF_SZ 504 // max size for rip header + 25 * entry

const int DEACTIVE_TIME = 10;
const int GARBAGE_COLLECTION_TIME = 60;
const int UPDATE_TIME = GARBAGE_COLLECTION_TIME / DEACTIVE_TIME;

// global variable
extern unsigned int router_id;
extern char input_ports[512];
extern char output_dest[512];
extern RouteTableNode *route_table;

static unsigned int second_tick;

int make_socket(int port)
{
	int sock;
	struct sockaddr_in name;

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // use udp
	if (sock < 0)
	{
		perror("Unable to create socket");
		remove_pid(router_id);
		exit(-1);
	}

	name.sin_family = AF_INET;
	name.sin_port = htons(port);
	name.sin_addr.s_addr = htonl (INADDR_ANY);
	if (bind (sock, (struct sockaddr *) &name, sizeof (name)) < 0)
	{
		perror("Failed to bind socket");
		printf("port = %d\n", port);
		remove_pid(router_id);
		exit(-1);
	}
	return sock;

}

int send_message(char *buf, int send_size, int sender_port)
{
	struct sockaddr_in sock;
	int sock_fd;


	sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock_fd < 0)
	{
		return -1;
	}

	sock.sin_addr.s_addr = htonl (INADDR_ANY);
	sock.sin_family = AF_INET;
	sock.sin_port = htons(sender_port);

	if (sendto(sock_fd, buf, send_size, 0, (struct sockaddr *)&sock, sizeof(sock)) < 0)
	{
		return -3;
	}
	close(sock_fd);

	return 0;
}

void make_response(void)
{
	char buffer[BUF_SZ];
	int i;

	// create different message for different receiver

	for (RouteTableNode *receiver = route_table;
		receiver != NULL;
		receiver = receiver->next)
	{
		// only send to neighbour and status is up
		if (receiver->flags[0] == 'N' && receiver->flags[1] == 'U')
		{
			RIPPacket packet;
			i = 0;

			packet.command = RIP_RESPONSE;
			packet.version = RIP_VERSION_2;

			for (RouteTableNode *table_entry = route_table;
				table_entry != NULL;
				table_entry = table_entry->next)
			{
				// do not send the information about sender
				packet.entry[i].AFI = AF_INET;
				packet.entry[i].address = table_entry->destination;
				packet.entry[i].next_hop = router_id;


				// split horizon with poison reverse
				if (receiver->destination == table_entry->reference)
				{
					packet.entry[i].metric = 16;
				}
				else
				{
					packet.entry[i].metric = table_entry->metric;
				}
				i++;
			}

			packet.n_entry = i;

			memset(buffer, 0, BUF_SZ);

			rip_packet_encode(buffer, &packet);

			printf("send: [%s] to %d\n", buffer, receiver->destination);

			send_message(buffer, strlen(buffer), receiver->port);
		}
	}
}

int update_table(RIPPacket *packet)
{
	int i;

	if (packet->n_entry == 0)
	{
		return -1;
	}

	// update neighbour router
	updateNeighbourRouter(packet->entry[0].next_hop);

	// add to routing table
	for (i = 0; i < (int)packet->n_entry; i++)
	{
		if (packet->entry[i].address != router_id)
		{
			insertIntoRoutingTable(
				packet->entry[i].address,
				packet->entry[i].next_hop,
				0,
				"LU_",
				packet->entry[i].metric,
				packet->entry[i].next_hop,
				0
				);
		}
	}
	return 0;
}

static void timer_handler()
{
	// atomic process, so disable the alarm at present
	signal(SIGALRM, SIG_IGN);

	// if detected one router offline, then start transmit
	if (updateTTL())
	{
		make_response();
	}
	//printf("----------------\n");
	printRoutingTable();
	//printf("----------------\n");

	if (second_tick % UPDATE_TIME == 0)
	{
		make_response();
	}


	second_tick++;
	// enable alarm again
	signal(SIGALRM, timer_handler);
}


int decode_message(char *buffer, RIPPacket *p)
{
	int ret;


	// if successfully read from input ports, then parse the
	// rip packet and add necessary information to routing
	// table
	if ((ret = rip_packet_decode(buffer, p)) == 0)
	{
		return p->command;
	}
	else
	{
		printf("Unable to parse[%d]\n", ret);
		return ret;
	}
}



int router_demon_start(void)
{
	/* Initilize */
	int i;
	int ret;
	char buffer[BUF_SZ]; // raw buffer
	struct itimerval timer;

	// read and load listening ports from config file
	SingleLinkedList inputs = split(input_ports, ',');

	// read and load adjacent hops from config file
	SingleLinkedList outputs = split(output_dest, ',');

	// get the number of input ports
	int n_inputs = 0;
	n_inputs = getLength(inputs);

	// create a array to store these ports
	int input_port_numbers[n_inputs];
	i = 0;
	for (SingleLinkedList it = inputs; it != NULL; it = getNext(it))
	{
		input_port_numbers[i] = atoi(it->string);
		i++;
	}

	// fds: file descriptor for listening ports
	// server_fd: file descriptor that is ready to read
	// client_fd: accepted connection
	int fds[n_inputs];
	int server_fd = 0;

	// file descriptor set
	fd_set readset;

	// size of sockaddr_in struct
	socklen_t len;


	// client socket
	struct sockaddr_in clientname;


	/* Process */


	// set timer
	timer.it_value.tv_sec = 1;
	timer.it_value.tv_usec = 0; // starts almost immediately
	timer.it_interval.tv_sec = 1; // period = 1Hz
	timer.it_interval.tv_usec = 0;

	// register timer
	signal(SIGALRM, timer_handler);
	// start timer
	setitimer(ITIMER_REAL, &timer, NULL);

	// create the initial routing with only adjacent routers
	initRoutingTable(outputs);

	// free the memory allocated by input and output
	destroyList(inputs);
	destroyList(outputs);


	// create sockets for each of these input ports
	for (i = 0; i < n_inputs; i++)
	{
		// create and bind port number to local machine
		fds[i] = make_socket(input_port_numbers[i]);

	}
	// blocking
	// Another example
	// http://www.gnu.org/software/libc/manual/html_node/Server-Example.html
	while (1)
	{
		// rebuild readset everytime after select
		FD_ZERO(&readset);
		for (i = 0; i < n_inputs; i++)
		{
			FD_SET(fds[i], &readset);
		}

		// select with socket is ready to read
		if ((ret = select(FD_SETSIZE, &readset, NULL, NULL, NULL)) < 0)
		{
			// select can catch the signal but we want to ignore it
			if (errno == EINTR)
			{
				continue;
			}
			else
			{
				perror("Failed to select");
				remove_pid(router_id);
				exit(-1);
			}

		}

		// we dont know which socket is ready so we need to poll
		for (i = 0; i < n_inputs; i++)
		{
			if (FD_ISSET(fds[i], &readset))
			{
				server_fd = fds[i];
				break;
			}
		}

		// udp receive
		memset(buffer, 0, sizeof(buffer));
		if (recvfrom(server_fd, buffer, BUF_SZ, 0, (struct sockaddr *)&clientname, &len) < 0)
		{
			perror("Failed to receive");
			remove_pid(router_id);
			exit(-1);
		}
		printf("recv: [%s]\n", buffer);

		// start decoding
		RIPPacket p;

		ret = decode_message(buffer, &p);

		if (ret == RIP_RESPONSE)
		{
			update_table(&p);
		}

	}


	destroyRoutingTable();

	return 0;
}
