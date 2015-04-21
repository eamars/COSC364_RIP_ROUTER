/**
 * Routing table implementation of RIP
 * Active routes
 *
 * Relations:
 * E <-> A <-> B <-> C
 *
 * Routing table for A
 * +--------------------------------------------+
 * |	NEXT_HOP 	PORT 	ADDR 	METRIC 		|  <-- NEXT_HOP: destination, ADDR: via
 * +--------------------------------------------+
 * |		E		xx		E 		xx			|  <-- at time 0
 * |		B		xx		B 		xx			|  <-- at time 0
 * |		C 		0		B 		ADDR[b]+new |  <-- at time 1
 * +--------------------------------------------+
 *
 * if NEXT_HOP = 0 then it's directly connected
 * When A send it's routing table to E, send
 * 		ADDR = A, NEXT_HOP = E, METRIC = xxx <-- drop
 * 		ADDR = A, NEXT_HOP = B, METRIC = xxx
 * 		ADDR = A, NEXT_HOP = C, METRIC = ADDR[b]+new
 * 	When A send it's routing table to B, send
 * 		ADDR = A, NEXT_HOP = E, METRIC = xxx
 * 		ADDR = A, NEXT_HOP = B, METRIC = xxx
 * 		ADDR = A, NEXT_HOP = C, METRIC = ADDR[b]+new
 * 	ADDR is always the one that is directly reachable
 */

#ifndef ROUTING_TABLE_H
#define ROUTING_TABLE_H

#include "list.h"


enum RTABLE {NEXT_HOP = 0, PORT, ADDR, METRIC};

void createRoutingTableFromConfig(SingleLinkedList list);
void updateTable(int next_hop, int port, int addr, int metric);
void getAllHops(int *hops);
int getNumEntry(void);
int getValue(int next_hop, int selection);
void destroyRoutingTable(void);
void printRoutingTable(void);

int getTimeoutHops(int *hops);
void timeoutUpdate(void);

#endif
