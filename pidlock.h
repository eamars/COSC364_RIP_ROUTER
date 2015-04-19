/**
 * Write and read the current pid of process to file
 */

#ifndef PID_LOCK_H
#define PID_LOCK_H

/**
 * Write process id to routerX.pid file
 * @param  pid process id
 * @param  rid router id
 * @return     negative if failed, 0 for succeed
 */
int write_pid(int pid, int rid);

/**
 * Read process from routerX.pid file
 * @param  rid router id
 * @return     negative if failed, positive for any process id
 */
int read_pid(int rid);

/**
 * Remove the routerX.pid file
 * @param rid router id
 */
void remove_pid(int rid);

#endif
