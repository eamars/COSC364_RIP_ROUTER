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
extern int router_id;
extern char input_ports[512];
extern char output_dest[512];

#define BUF_SZ 64

int make_socket(int port)
{
	int sock;
	struct sockaddr_in name;

	sock = socket(AF_INET, SOCK_STREAM, 0);
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

int read_from_client (int filedes)
{
	char buffer[BUF_SZ];
	int nbytes;

	nbytes = read (filedes, buffer, BUF_SZ);
	if (nbytes < 0)
	{
		/* Read error. */
		perror ("Failed to read");
		exit (-1);
	}
	else if (nbytes == 0)
	/* End-of-file. */
	{
		return -1;
	}
	else
	{
		/* Data read. */
		RIPPacket p;
		printf("Router%d: [%s]\n", router_id, buffer);

		// if successfully read from input ports, then parse the
		// rip packet and add necessary information to routing
		// table
		if (!rip_packet_decode(buffer, &p))
		{
			debug_print_rip_packet(&p);
			printRoutingTable();
			if (p.command == RIP_REQUEST)
			{
				// request
				// send back routing table
			}
			else if (p.command == RIP_RESPONSE)
			{
				// response
				// update routing table
			}
			printRoutingTable();

		}
		return 0;
	}
}


int router_demon_start(void)
{
	SingleLinkedList inputs = split(input_ports, ',');
	SingleLinkedList outputs = split(output_dest, ',');
	int n_inputs = getLength(inputs);
	int input_port_numbers[n_inputs];
	int i;

	// fds: file descriptor for listening ports
	// server_fd: file descriptor that is ready to read
	// client_fd: accepted connection
	int fds[n_inputs], server_fd, client_fd;

	// file descriptor set
	fd_set readset;
	int c = sizeof(struct sockaddr_in);


	// client socket
	struct sockaddr_in clientname;

	int pid;

	// create and print routing tables
	createRoutingTable(outputs);
	printRoutingTable();

	// get input port numbers
	i = 0;
	for (SingleLinkedList it = inputs; it != NULL; it = getNext(it))
	{
		input_port_numbers[i] = atoi(it->string);
		i++;
	}



	// create sockets
	for (i = 0; i < n_inputs; i++)
	{
		// create and bind port number to local machine
		fds[i] = make_socket(input_port_numbers[i]);

		// listen to ports
		if (listen(fds[i], 1) < 0)
		{
			perror("Failed to listen");
			remove_pid(router_id);
			exit(-1);
		}
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

		// accept incoming connection
		client_fd = accept(server_fd, (struct sockaddr *)&clientname, (socklen_t *)&c);
		if (client_fd < 0)
		{
			perror("Failed to accept");
			remove_pid(router_id);
			continue;
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
			// read from remove client
			read_from_client(client_fd);

			// close(client_fd);
			exit(0);
		}
		else
		{
			close(client_fd);
		}

	}


	destroyRoutingTable();

	destroyList(inputs);
	destroyList(outputs);
	return 0;
}
