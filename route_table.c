#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "route_table.h"


// route table
RouteTableNode *route_table;
RouteTableNode *neighbour_table; // constant, never changed

extern const int DEACTIVE_TIME;
extern const int GARBAGE_COLLECTION_TIME;
extern unsigned router_id;


static RouteTableNode *create_new_entry(int dest, int next, int port, char *flags, int metric, int ref, int TTL)
{
	RouteTableNode *table = (RouteTableNode *) malloc (sizeof(RouteTableNode));

	table->destination = dest;
	table->next_hop = next;
	table->port = port;
	strncpy(table->flags, flags, 3);
	table->metric = metric;
	table->reference = ref;
	table->TTL = TTL;

	table->next = NULL;

	return table;
}

void initRoutingTable(SingleLinkedList list)
{
	int addr, port, next_hop, metric;

	for (SingleLinkedList it = list; it != NULL; it = getNext(it))
	{

		SingleLinkedList splited_string, temp;
		splited_string = split(it->string, '-');

		// addr
		temp = splited_string;
		addr = atoi(temp->string);
		temp = getNext(temp);

		// metric
		metric = atoi(temp->string);
		temp = getNext(temp);

		// port
		port = atoi(temp->string);
		temp = getNext(temp);

		// next_hop
		next_hop = addr;

		destroyList(splited_string);

		// insert into route table
		RouteTableNode *prev = route_table;
		route_table = create_new_entry(addr, next_hop, port, "NU_", metric, router_id, 0);
		route_table->next = prev;

		// insert into neighbour table for backup
		prev = neighbour_table;
		neighbour_table = create_new_entry(addr, next_hop, port, "N__", metric, router_id, 0);
		neighbour_table->next = prev;
	}
}

void insertIntoRoutingTable(int dest, int next, int port, char *flags, int metric, int ref, int TTL)
{

	RouteTableNode *prev = NULL;
	RouteTableNode *curr = route_table;

	while (curr != NULL)
	{
		// entry already exists
		if (curr->destination == dest)
		{
			// update the metric
			// first get metric from neighbour table, we assumed the neighbour is always
			// on at this stage
			int metric_to_neighbour = 16;

			for (RouteTableNode *it = route_table; it != NULL && it->flags[1] == 'N'; it = it->next)
			{
				if (it->destination == next)
				{
					// find the neighbour
					metric_to_neighbour = it->metric;
					break;
				}
			}

			curr->metric += metric_to_neighbour;
			if (curr->metric > 16)
			{
				curr->metric = 16;
				curr->flags[1] = 'D';
			}

			// update reference
			curr->reference = ref;

			return;
		}
		prev = curr;
		curr = curr->next;
	}

	// insert at front
	if (curr == NULL && prev == NULL)
	{
		route_table = create_new_entry(dest, next, port, flags, metric, ref, TTL);
	}
	// insert at end
	else if (curr == NULL && prev != NULL)
	{
		prev->next = create_new_entry(dest, next, port, flags, metric, ref, TTL);
	}
}

static void free_nodes(RouteTableNode *table)
{
	if (table != NULL)
	{
		free_nodes(table->next);
		free(table);
	}
}

static void print_nodes(RouteTableNode *table)
{
	if (table != NULL)
	{
		printf("DEST = %d, NEXT_HOP = %d, PORT = %d, METRIC = %d, FLAGS = %c%c%c, REF = %d, TTL = %d\n",
			table->destination,
			table->next_hop,
			table->port,
			table->metric,
			table->flags[0], table->flags[1], table->flags[2],
			table->reference,
			table->TTL
			);
		print_nodes(table->next);
	}
}

void destroyRoutingTable()
{
	free_nodes(route_table);
	free_nodes(neighbour_table);
}
void printRoutingTable()
{
	printf("RIP routing table\n");
	print_nodes(route_table);
	printf("RIP internal neighbour backup table\n");
	print_nodes(neighbour_table);
}

void removeEntry(int dest)
{
	RouteTableNode *prev = NULL;
	RouteTableNode *curr = route_table;


	while (curr != NULL)
	{
		if (curr->destination == dest)
		{
			break;
		}
		prev = curr;
		curr = curr->next;
	}

	// remove at head
	if (prev == NULL && curr != NULL)
	{
		route_table = curr->next;
		free(curr);
	}
	// remove at middle
	else if (prev != NULL && curr != NULL)
	{
		prev->next = curr->next;
		free(curr);
	}
	else if (prev != NULL && curr == NULL)
	{
		prev->next = NULL;
		free(curr);
	}
}

void updateTTL(void)
{
	for (RouteTableNode *it = route_table;
		it != NULL;
		it = it->next)
	{
		it->TTL++;
		if (it->flags[1] == 'U' && it->TTL == DEACTIVE_TIME)
		{
			it->TTL = 0;
			it->flags[1] = 'D';
		}
		else if (it->flags[1] == 'D' && it->TTL == GARBAGE_COLLECTION_TIME)
		{
			removeEntry(it->destination);
		}
	}
}
