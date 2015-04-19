/**
 * Routing table implementation of RIP
 * Active routes
 * -------------------------------------------------
 * | Destination		  Port				Metric |
 * |-----------------------------------------------|
 * |     10				 10011				  8	   |
 * |      8				 10012				  1    |
 * -------------------------------------------------
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
void updateTable(int dest, int port, int metric);

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
