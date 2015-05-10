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
	int metric_to_neighbour;
	int new_metric;


	// calculate metric to neighbour
	metric_to_neighbour = 16;
	for (RouteTableNode *it = route_table; it != NULL; it = it->next)
	{
		if (it->flags[0] == 'N')
		{
			if (it->destination == next)
			{
				// find the neighbour
				metric_to_neighbour = it->metric;
				break;
			}
		}

	}

	// create new metric for entry
	new_metric = metric + metric_to_neighbour;
	if (new_metric >= 16)
	{
		new_metric = 16;
	}

	while (curr != NULL)
	{

		// entry already exists
		if (curr->destination == dest)
		{
			// update the metric
			// first get metric from neighbour table, we assumed the neighbour is always
			// on at this stage
			// ignore the split horizon with posion reverse message
			if (metric == 16 && next == curr->next_hop)
			{
				if (curr->flags[1] == 'U')
				{
					curr->flags[1] = 'D';
					curr->TTL = 0;
					curr->metric = 16;
					curr->reference = ref;
				}
				else if (curr->flags[1] == 'D')
				{

				}
			}



			// update only if lower metric
			if (new_metric < curr->metric)
			{
				curr->metric = new_metric;
				curr->next_hop = next;
				curr->reference = ref;

				if (new_metric == 16)
				{
					curr->flags[1] = 'D';
					curr->TTL = 0;
				}
				else
				{
					curr->flags[1] = 'U';
					curr->TTL = 0;
				}

				// turn neighbour router to learned router
				if (curr->flags[0] == 'N')
				{
					curr->flags[0] = 'L';
					curr->port = 0;
				}
			}


			return;
		}
		prev = curr;
		curr = curr->next;
	}



	// insert at front
	// do not insert dead link
	if (curr == NULL && prev == NULL)
	{
		if (new_metric != 16)
		{
			route_table = create_new_entry(dest, next, port, flags, new_metric, ref, TTL);
		}
	}
	// insert at end
	else if (curr == NULL && prev != NULL)
	{
		if (new_metric != 16)
		{
			prev->next = create_new_entry(dest, next, port, flags, new_metric, ref, TTL);
		}

	}
}

void updateNeighbourRouter(int destination)
{
	for (RouteTableNode *it = route_table; it != NULL; it = it->next)
	{
		if (it->flags[0] == 'N')
		{
			if (it->destination == destination)
			{
				it->flags[1] = 'U'; // bring up the router and reset TTL
				it->TTL = 0;
				it->next_hop = destination;
				it->reference = destination;
				if (it->metric == 16) // restore metric to neighbour router
				{
					// look for backup entry to restore metric
					for (RouteTableNode *backup = neighbour_table; backup != NULL; backup = backup->next)
					{
						if (backup->destination == destination)
						{
							it->metric = backup->metric;
							break;
						}
					}

				}
				return;
			}
		}

	}

	// if neighbour router doesn't exist, then restore from backup

	for (RouteTableNode *backup = neighbour_table; backup != NULL; backup = backup->next)
	{
		if (backup->destination == destination)
		{
			RouteTableNode *prev = route_table;
			route_table = create_new_entry(
				backup->destination,
				backup->next_hop,
				backup->port,
				"NU_", // no link status in backup neighbour
				backup->metric,
				backup->reference,
				backup->TTL
				);
			printf("%c%c%c\n", backup->flags[0], backup->flags[1], backup->flags[2]);
			route_table->next = prev;
			break;
		}
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

void printRoutingTable()
{
	RouteTableNode *current = route_table;

	printf("Router[%d] Routing table\n", router_id);
	printf("Destination    Next_Hop    Port    Metric    Flags    Reference    TTL\n");
	while (current != NULL)
	{
		printf("%6d%14d%12d%7d%7c%c%c%11d%10d\n",
			current->destination,
			current->next_hop,
			current->port,
			current->metric,
			current->flags[0], current->flags[1], current->flags[2],
			current->reference,
			current->TTL
			);
		current = current->next;
	}
}

void destroyRoutingTable()
{
	free_nodes(route_table);
	free_nodes(neighbour_table);
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

int updateTTL(void)
{
	int trigger = 0;

	for (RouteTableNode *it = route_table;
		it != NULL;
		it = it->next)
	{
		if (it->flags[0] == 'N')
		{
			// neighbour router offline
			if (it->flags[1] == 'U' && it->TTL == DEACTIVE_TIME)
			{
				it->TTL = 0;
				it->flags[1] = 'D';
				it->metric = 16;
				trigger = 1;

				// set all metric with next_hop = current neighbour to 16
				for (RouteTableNode *entry = route_table; entry != NULL; entry = entry->next)
				{
					if (entry->next_hop == it->destination && entry->destination != it->destination)
					{
						entry->TTL = 0;
						entry->flags[1] = 'D';
						entry ->metric = 16;
					}
				}
			}
			else if (it->flags[1] == 'D' && it->TTL == GARBAGE_COLLECTION_TIME)
			{
				removeEntry(it->destination);
			}
			else
			{
				it->TTL++;
			}
		}

		// we only maintain the garbage collection timer for entries learned fron
		// neighbour router
		else if (it->flags[0] == 'L')
		{
			if (it->flags[1] == 'U')
			{

			}
			else if (it->flags[1] == 'D' && it->TTL == GARBAGE_COLLECTION_TIME)
			{
				removeEntry(it->destination);
			}
			else
			{
				it->TTL++;
			}
		}
	}

	return trigger;
}
