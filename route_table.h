/**
 * Route table module
 *
 * Module maintains two route tables
 * route_table: store the router information directly set by configure and learned
 * 				from neighbour routers
 * neigbhour_table: stores the initial information read from configure and restore
 * 					settings if necessary
 *
 * Route table format:
 * Destination    Next_Hop    Port    Metric    Flags    TTL
 *
 * Destination: destination router, stored as router id
 * Next_Hop: neighbour used to reach destinated router
 * Port: only valid for neighbour router. Used to send UDP packet
 * Metric: total cost to reach the destinated router
 * Flags: contains three columns
 *   flags[0]: N for neighbour router and L for remote router learned from neigbhour
 *   flags[1]: U for link up and D for link down
 *   flags[2]: unused
 * TTL: Time To aLive counter
 *
 * Author: Ran Bao, Liang Ma
 *
 */

#ifndef ROUTE_TABLE_H
#define ROUTE_TABLE_H

#include "list.h"

typedef struct route_table_s RouteTableNode;
struct route_table_s
{
	int destination;
	int next_hop;
	int port;
	char flags[3];
	int metric;
	int TTL;

	RouteTableNode *next;
};

/**
 * Initilize the routing table, includes adding entry from configure file to
 * route_table and neighbour_table
 * @param list [description]
 */
void initRoutingTable(SingleLinkedList list);

/**
 * Insert new entry to route_table
 * @param dest   destinated router
 * @param next   next_hop or via
 * @param port   port number
 * @param flags  link status of router
 * @param metric total cost to reach router
 * @param ref    the entry that learned from
 * @param TTL    counter
 */
void addEntryToRoutingTable(int dest, int next, int metric);

/**
 * Free memory that allocated for route_table and neighbour_table
 * @return [description]
 */
void destroyTable(RouteTableNode *table);

/**
 * pretty print the route_table
 */
void printTable(RouteTableNode *table);

/**
 * Maintain the counter of entry in route_table
 * @return [description]
 */
int updateTTL(void);

/**
 * Bring the neighbour router online if receive any message from this router
 * @param destination [description]
 */
void updateNeighbourRouter(int destination);

RouteTableNode *createForwardingTable();

#endif
