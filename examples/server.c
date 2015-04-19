/**
 * Sample program using select
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <signal.h>

#define BUF_SZ 4096

int make_socket(int port)
{
	int sock;
	struct sockaddr_in name;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		perror("Unable to create socket");
		exit(-1);
	}

	name.sin_family = AF_INET;
	name.sin_port = htons(port);
	name.sin_addr.s_addr = htonl (INADDR_ANY);
	if (bind (sock, (struct sockaddr *) &name, sizeof (name)) < 0)
	{
		perror("Failed to bind socket");
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
		printf("Server: %s\n", buffer);
		return 0;
	}
}

int main(void)
{
	// ignore all return value from child process
	signal(SIGCHLD, SIG_IGN);

	// fds: file descriptor for listening ports
	// server_fd: file descriptor that is ready to read
	// client_fd: accepted connection
	int fds[3], server_fd, client_fd;

	// file descriptor set
	fd_set readset;
	int c = sizeof(struct sockaddr_in);

	// client socket
	struct sockaddr_in clientname;

	int pid;


	// create sockets
	for (int i = 0, p = 10000; i < 3; i++, p++)
	{
		// create and bind port number to local machine
		fds[i] = make_socket(p);

		// listen to ports
		if (listen(fds[i], 1) < 0)
		{
			perror("Failed to listen");
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
		for (int i = 0, p = 10000; i < 3; i++, p++)
		{
			FD_SET(fds[i], &readset);
		}

		// select with socket is ready to read
		if (select(FD_SETSIZE, &readset, NULL, NULL, NULL) < 0)
		{
			perror("Failed to select");
			exit(-1);
		}

		// we dont know which socket is ready so we need to poll
		for (int i = 0; i < 3; i++)
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
			continue;
		}

		// if one connection take too long to process, we
		// shall fork a child process to deal with receiving messages
		if ((pid = fork()) < 0)
		{
			perror("Failed to fork");
			exit(-1);
		}
		else if (pid == 0)
		{
			// read from remove client
			read_from_client(client_fd);
			fprintf (stderr,
		                     "Server: connect from host %s, port %hd.\n",
		                     inet_ntoa (clientname.sin_addr),
		                     ntohs (clientname.sin_port));

			// close(client_fd);
			exit(0);
		}
		else
		{
			close(client_fd);
		}

	}
	return 0;

}
