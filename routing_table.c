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

#include <stdio.h>
#include <stdlib.h>
#include "routing_table.h"
#include "list.h"

// sort with respect to destination
typedef struct route_table_s RTableNode;
struct route_table_s
{
	int row[4];
	RTableNode *left;
	RTableNode *right;
};

enum RTABLE {DEST = 0, PORT, VIA, METRIC};

// global variable
RTableNode *routing_table;



static RTableNode *create_new_node(int dest, int port, int via, int metric)
{
	// allocate memory for node
	RTableNode *node = (RTableNode *) malloc (sizeof(RTableNode));

	// copy value
	node->row[DEST] = dest;
	node->row[PORT] = port;
	node->row[VIA] = via;
	node->row[METRIC] = metric;

	// set left and right to NULL
	node->left = NULL;
	node->right = NULL;

	return node;
}


static RTableNode *insert(RTableNode *head, int dest, int port, int via, int metric)
{
	// base case: empty list
	if (head == NULL)
	{
		head = create_new_node(dest, port, via, metric);
	}
	else
	{
		// all destination must be unique
		// if not, then update the metric and via
		if (head->row[DEST] == dest)
		{
			if (head->row[METRIC] >= metric)
			{
				head->row[VIA] = via;
				head->row[METRIC] = metric;
			}
		}
		else if (head->row[DEST] > dest)
		{
			// insert at left
			head->left = insert(head->left, dest, port, via, metric);
		}
		else
		{
			// insert at right
			head->right = insert(head->right, dest, port, via, metric);
		}
	}
	return head;
}

static void free_nodes(RTableNode *head)
{
	if (head != NULL)
	{
		free_nodes(head->left);
		free_nodes(head->right);
		free(head);
	}
}

static void print_table(RTableNode *head)
{
	if (head != NULL)
	{
		print_table(head->left);
		printf("Dest = %d, Port = %d, Via = %d, Metric = %d\n", head->row[DEST],
			head->row[PORT], head->row[VIA], head->row[METRIC]);
		print_table(head->right);
	}
}

static int getValue(RTableNode *head, int dest, int selection)
{
	if (head == NULL)
	{
		return -1;
	}
	else
	{
		if (head->row[DEST] == dest)
		{
			return head->row[selection];
		}
		if (head->row[DEST] > dest)
		{
			return getValue(head->left, dest, selection);
		}
		else
		{
			return getValue(head->right, dest, selection);
		}
	}
}


void updateTable(int dest, int port, int via, int metric)
{
	routing_table = insert(routing_table, dest, port, via, metric);
}

int getRouterMetric(int dest)
{
	return getValue(routing_table, dest, METRIC);
}

int getRouterPort(int dest)
{
	return getValue(routing_table, dest, PORT);
}

int getRouterVia(int dest)
{
	return getValue(routing_table, dest, VIA);
}

void printRoutingTable(void)
{
	print_table(routing_table);
}

void createRoutingTable(SingleLinkedList list)
{
	int dest, port, via, metric;

	for (SingleLinkedList it = list; it != NULL; it = getNext(it))
	{
		SingleLinkedList table, temp;
		table = split(it->string, '-');

		temp = table;

		dest = atoi(temp->string);

		temp = getNext(temp);

		metric = atoi(temp->string);

		temp = getNext(temp);

		port = atoi(temp->string);

		via = 0;

		destroyList(table);

		updateTable(dest, port, via, metric);
	}
}

void destroyRoutingTable(void)
{
	free_nodes(routing_table);
}

int get_routers_dest(RTableNode *head, int *routers)
{
	static int index = 0;
	int count = 0;

	if (head != NULL)
	{
		count += get_routers_dest(head->left, routers);
		routers[index] = head->row[0];
		routers[++index] = -1;
		count++;

		count += get_routers_dest(head->right, routers);
	}

	return count;
}

int getAllRouters(int *routers)
{
	return get_routers_dest(routing_table, routers);
}

int get_num_of_entry_in_rtable(RTableNode *head)
{
	int count = 0;

	if (head != NULL)
	{
		count += get_num_of_entry_in_rtable(head->left);
		count++;
		count += get_num_of_entry_in_rtable(head->right);
	}

	return count;
}

int getNumEntry(void)
{
	return get_num_of_entry_in_rtable(routing_table);
}
