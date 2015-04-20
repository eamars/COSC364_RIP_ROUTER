/**
 * Sample program to send message to local ports
 */

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	struct sockaddr_in server;
	int socket_fd;
	char *message;

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0)
	{
		printf("Failed to create socket\n");
		return -1;
	}

	server.sin_addr.s_addr = htonl (INADDR_ANY);
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[1]));

	// connect to remote server
	if (connect(socket_fd, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		printf("Failed to connect to %s\n", argv[1]);
		return -2;
	}

	// send http header to remote host

	message = argv[2];
	if (send(socket_fd, message, strlen(message), 0) < 0)
	{
		printf("Failed to send\n");
		return -3;
	}
	printf("send: [%s]\n", message);

	// recv from remote
	char reply[10240];
	if (recv(socket_fd, reply, 10240, 0) < 0)
	{
		printf("Failed to receive\n");
		return -4;
	}

	printf("recv: [%s]\n", reply);

	close(socket_fd);

	return 0;
}

