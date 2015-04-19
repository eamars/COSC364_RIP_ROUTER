/**
 * Write and read the current pid of process to file
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int write_pid(int pid, int rid)
{
	char pid_lock_file[80];
	char pid_file[10];

	// create pid file name
	sprintf(pid_lock_file, "router%d.pid", rid);

	if (access(pid_lock_file, F_OK) != -1)
	{
		// file exist
		printf("\x1B[31m%s already exists. \nIf router%d is not currently running, please remove %s manually\033[0m\n",
			pid_lock_file, rid, pid_lock_file);
		return -3;
	}

	FILE *fp;
	if ((fp = fopen(pid_lock_file, "w")) == NULL)
	{
		printf("\x1B[31mFailed to write %s\033[0m\n", pid_lock_file);
		return -3;
	}

	sprintf(pid_file, "%d\n", pid);

	fwrite(pid_file, strlen(pid_file), 1, fp);

	fclose(fp);

	return 0;
}

int read_pid(int rid)
{
	char pid_lock_file[80];
	char pid_file[10];

	// create pid file name
	sprintf(pid_lock_file, "router%d.pid", rid);

	if (access(pid_lock_file, F_OK) == -1)
	{
		// file doesnt exist
		printf("\x1B[31m%s doesn't exists.\nPlease make sure that router%d is currently running\033[0m\n",
			pid_lock_file, rid);
		return -3;
	}

	FILE *fp;
	if ((fp = fopen(pid_lock_file, "r")) == NULL)
	{
		printf("\x1B[31mFailed to open %s\033[0m\n", pid_lock_file);
		return -3;
	}

	fread(pid_file, sizeof(pid_file), 1, fp);

	return atoi(pid_file);
}

void remove_pid(int rid)
{
	char cmd[512];

	// remove the pid lock
	sprintf(cmd, "rm -f router%d.pid", rid);
	system(cmd);
}
