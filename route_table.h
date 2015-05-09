#ifndef ROUTE_TABLE_H
#define ROUTE_TABLE_H

#include "list.h"

typedef struct route_table_s RouteTableNode;
struct route_table_s
{
	int destination;
	int next_hop;
	int port;

	// flags
	// N__: neighbour
	// L__: learned
	// _U_: link up
	// _D_: link down

	char flags[3];
	int metric;

	// reference is where does the entry learned from
	int reference;
	int TTL;

	RouteTableNode *next;
};

void initRoutingTable(SingleLinkedList list);
void insertIntoRoutingTable(int dest, int next, int port, char *flags, int metric, int ref, int TTL);
void destroyRoutingTable();
void printRoutingTable();
void updateTTL();

#endif
