/**
 *  Router simulator
 *  The program simulate the real router that communicate with each other
 *  using RIP (Router Information Protocol)
 *
 * 	Author: Ran Bao, Liang Ma
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "config.h"
#include "list.h"
#include "pidlock.h"


// global variables
static const char *usage = "Usage: router.out [\x1B[32mstart\033[0m|\x1B[32mstop\033[0m] router.cfg";
unsigned int router_id;
char input_ports[512];
char output_dest[512];

// external functions
int router_demon_start(void);

int load_config(char *config)
{
	char buffer[512];
	int ret;

	if ((ret = config_open(config)) < 0)
	{
		// failed to load
		printf("\x1B[31mFailed to read config %s [%d]\033[0m\n", config, ret);
		return -2;
	}

	// read Router id
	config_get_value("router-id", buffer);
	router_id = atoi(buffer);

	// read Input ports
	config_get_value("input-ports", input_ports);

	// read outputs
	config_get_value("outputs", output_dest);

	// close config file and destroy memory
	config_close();

	return 0;

}

int start(void)
{
	int pid;
	int ret;


	// fork a subprocess to execute the job
	pid = fork();
	if (pid < 0) // failed to fork
	{
		printf("\x1B[31mFailed to fork subprocess\033[0m\n");
		return -2;
	}
	else if (pid == 0) // child process
	{
		pid = getpid();

		// write pid to file to make sure that the router is not allowed to be
		// executed twice at the same time
		ret = write_pid(pid, router_id);
		if (ret)
		{
			return ret;
		}

		printf("\x1B[32mRouter%d\033[0m starts at pid=\x1B[32m%d\033[0m\n", router_id, pid);

		// start the router demon
		ret = router_demon_start();
		if (ret)
		{
			return ret;
		}
		remove_pid(router_id);

	}
	else // parent process
	{

	}
	return 0;
}

int stop(void)
{
	int pid;
	char cmd[512];

	// read pid from pid lock
	pid = read_pid(router_id);
	if (pid < 0)
	{
		return pid;
	}

	// terminate the task
	// kill both child and parent task
	sprintf(cmd, "pkill -TERM -P %d", pid);
	system(cmd);
	sprintf(cmd, "kill -TERM %d", pid);
	system(cmd);
	// kill(pid, SIG_TERM);

	printf("\x1B[32mRouter%d\033[0m Terminated\n", router_id);

	remove_pid(router_id);

	return 0;
}

int main(int argc, char **argv)
{
	int ret;

	// ignore all return value from child process
	signal(SIGCHLD, SIG_IGN);

	// exit when input argument not equal to 3
	if (argc != 3)
	{
		printf("%s\n", usage);
		return -1;
	}

	// exit when nether start or stop
	if (!strcmp(argv[1], "start"))
	{
		ret = load_config(argv[2]);
		if (ret)
		{
			return ret;
		}
		return start();
	}
	else if (!strcmp(argv[1], "stop"))
	{
		ret = load_config(argv[2]);
		if (ret)
		{
			return ret;
		}
		return stop();
	}
	else
	{
		printf("%s\n", usage);
		return -1;
	}





	return 0;
}
