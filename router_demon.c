#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>
#include <signal.h>


#include "list.h"
#include "routing_table.h"
#include "rip_message.h"
#include "pidlock.h"

// global variable
extern unsigned int router_id;
extern char input_ports[512];
extern char output_dest[512];

#define BUF_SZ 504 // max size for rip header + 25 * entry

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
		remove_pid(router_id);
		exit(-1);
	}
	return sock;

}

int make_response(char *buf)
{
	// send routing tables to filedes (without split-horizon)
	RIPPacket p;

	int n = getNumEntry();
	int hops[n];

	// make rip packet
	p.command = RIP_RESPONSE;
	p.version = RIP_VERSION_2;
	p.n_entry = n;

	// prepare for send buffer
	int send_size = 4 + n * 20;

	// get router hops
	// without split horizon
	getAllHops(hops);
	for (int i = 0; i < n; i++)
	{
		p.entry[i].AFI = AF_INET;
		p.entry[i].address = router_id;
		p.entry[i].next_hop = hops[i];
		p.entry[i].metric = getValue(hops[i], METRIC);
	}

	// form raw string
	rip_packet_encode(buf, &p);

	return send_size;
}

int send_response(char *buf, int send_size, int sender_port)
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

int get_sender(char *buf)
{
	RIPPacket packet;
	rip_packet_decode(buf, &packet);


	return getValue(packet.entry[0].address, PORT);
}

int update_table(char *buffer)
{
	RIPPacket packet;
	rip_packet_decode(buffer, &packet);

	printf("Router%d\n", router_id);
	printf ("Before: \n");
	printRoutingTable();
	for (unsigned int i = 0; i < packet.n_entry; i++)
	{
		// split horizon enabled here
		if (packet.entry[i].next_hop == router_id)
		{
			continue;
		}
		updateTable(
			packet.entry[i].next_hop,
			0,
			packet.entry[i].address,
			packet.entry[i].metric + getValue(packet.entry[i].address, METRIC)
		);
	}
	printf ("After: \n");
	printRoutingTable();
	return 0;
}

int process_message(char *buffer)
{
	int ret;

	RIPPacket p;

	// if successfully read from input ports, then parse the
	// rip packet and add necessary information to routing
	// table
	if ((ret = rip_packet_decode(buffer, &p)) == 0)
	{
		return p.command;
	}
	else
	{
		printf("Unable to parse[%d]\n", ret);
	}

	return -1;
}



int router_demon_start(void)
{
	/* Initilize */
	int i;

	// read and load listening ports from config file
	SingleLinkedList inputs = split(input_ports, ',');

	// read and load adjacent hops from config file
	SingleLinkedList outputs = split(output_dest, ',');

	// get the number of input ports
	int n_inputs = getLength(inputs);

	// create a array to store these ports
	int input_port_numbers[n_inputs];
	i = 0;
	for (SingleLinkedList it = inputs; it != NULL; it = getNext(it))
	{
		input_port_numbers[i] = atoi(it->string);
		i++;
	}

	// index


	// fds: file descriptor for listening ports
	// server_fd: file descriptor that is ready to read
	// client_fd: accepted connection
	int fds[n_inputs], server_fd;

	// file descriptor set
	fd_set readset;

	// size of sockaddr_in struct
	socklen_t len;


	// client socket
	struct sockaddr_in clientname;

	int pid;
	int ret;
	char buffer[BUF_SZ]; // raw buffer

	/* Process */

	// create the initial routing with only adjacent routers
	createRoutingTableFromConfig(outputs);
	printRoutingTable();

	// free the memory allocated by input and output
	destroyList(inputs);
	destroyList(outputs);



	// create sockets for each of these input ports
	for (i = 0; i < n_inputs; i++)
	{
		// create and bind port number to local machine
		fds[i] = make_socket(input_port_numbers[i]);

	}

	// clear the read set
	FD_ZERO(&readset);


	// blocking
	// Another example
	// http://www.gnu.org/software/libc/manual/html_node/Server-Example.html
	while (1)
	{
		// rebuild readset everytime after select
		for (i = 0; i < n_inputs; i++)
		{
			FD_SET(fds[i], &readset);
		}

		// select with socket is ready to read
		if (select(FD_SETSIZE, &readset, NULL, NULL, NULL) < 0)
		{
			perror("Failed to select");
			remove_pid(router_id);
			exit(-1);
		}

		// we dont know which socket is ready so we need to poll
		for (int i = 0; i < n_inputs; i++)
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

		// if one connection take too long to process, we
		// shall fork a child process to deal with receiving messages
		if ((pid = fork()) < 0)
		{
			perror("Failed to fork");
			remove_pid(router_id);
			exit(-1);
		}
		else if (pid == 0)
		{

			ret = process_message(buffer);
			if (ret == RIP_REQUEST)
			{
				int send_size;
				int sender_port;

				if ((sender_port = get_sender(buffer)) < 0)
				{
					printf("Unable to find sender\n");
					exit(-1);
				}

				send_size = make_response(buffer);

				if ((ret = send_response(buffer, send_size, sender_port)) != 0)
				{
					perror("Failed to send");
					exit(-1);
				}


			}
			else if (ret == RIP_RESPONSE)
			{
				update_table(buffer);
			}


			// close(client_fd);
			exit(0);
		}
		else
		{
			// close(client_fd);
		}

	}


	destroyRoutingTable();

	return 0;
}
