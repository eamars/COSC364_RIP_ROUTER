/**
 * Routing table implementation of RIP
 * Active routes
 * --------------------------------------------------------
 * | Destination		  Port		   via		   Metric |
 * |------------------------------------------------------|
 * |     10				 10011			1	         8	  |
 * |      8				 10012			2	         1    |
 * --------------------------------------------------------
 *
 * If via = 0, then it's directly connected
 */

#ifndef ROUTING_TABLE_H
#define ROUTING_TABLE_H

#include "list.h"

/**
 * insert or update the existing routing table
 * @param dest   destinated router
 * @param port   port of destinated router
 * @param metric cost to router
 */
void updateTable(int dest, int port, int via, int metric);

/**
 * returns the cost to router
 * @param  dest destinated router
 * @return      cost
 */
int getRouterMetric(int dest);

/**
 * returns the port of destinated router
 * @param  dest destinated router
 * @return      port number
 */
int getRouterPort(int dest);

int getRouterVia(int dest);


/**
 * pretty print the routing table
 */
void printRoutingTable(void);

/**
 * Create a new routing table from configure file
 * @param list [description]
 */
void createRoutingTable(SingleLinkedList list);

/**
 * Free the memory taken by routing table
 */
void destroyRoutingTable(void);

#endif
