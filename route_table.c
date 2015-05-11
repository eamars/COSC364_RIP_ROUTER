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


static RouteTableNode *create_new_entry(int dest, int next, int port, char *flags, int metric, int TTL)
{
	RouteTableNode *table = (RouteTableNode *) malloc (sizeof(RouteTableNode));

	table->destination = dest;
	table->next_hop = next;
	table->port = port;
	strncpy(table->flags, flags, 3);
	table->metric = metric;
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
		route_table = create_new_entry(addr, next_hop, port, "NU-", metric, 0);
		route_table->next = prev;

		// insert into neighbour table for backup
		prev = neighbour_table;
		neighbour_table = create_new_entry(addr, next_hop, port, "N--", metric, 0);
		neighbour_table->next = prev;
	}
}

RouteTableNode *createForwardingTable()
{
	// create a new table
	// *REMEMBER TO FREE*
	RouteTableNode *table = NULL;

	// iteriate through all records in route table and add to forwarding table
	for (RouteTableNode *entry = route_table; entry != NULL; entry = entry->next)
	{
		RouteTableNode *prev = NULL;
		RouteTableNode *curr = table;
		int find = 0;

		// find a suitable location to insert
		while (curr != NULL)
		{
			// read route information
			if (curr->destination == entry->destination)
			{
				// over write information
				if (entry->metric < curr->metric)
				{
					curr->port = entry->port;
					curr->next_hop = entry->next_hop;
					strncpy(curr->flags, entry->flags, 2);
					curr->metric = entry->metric;
					curr->TTL = entry->TTL;
				}
				find = 1;
				break;
			}

			prev = curr;
			curr = curr->next;
		}

		// couldn't find
		if (!find)
		{
			if (curr == NULL && prev == NULL) // insert at the front
			{
				table = create_new_entry(entry->destination, entry->next_hop, entry->port, entry->flags, entry->metric, entry->TTL);
			}

			else if (curr == NULL && prev != NULL) // insert at the end
			{
				prev->next = create_new_entry(entry->destination, entry->next_hop, entry->port, entry->flags, entry->metric, entry->TTL);
			}
		}
	}
	return table;
}

void insertDeadLinkIntoRoutingTable(int dest, int next, int metric)
{
	// since the incoming is a dead link, we
	// 1. The route exists and still alive, then we set flag to link down and reset TTL
	// 2. The route exists but already down. We keep settings unchanged
	// 3. The route doesn't exist, we still insert into the route table and set flag to link down and set TTL to 0
	RouteTableNode *prev = NULL;
	RouteTableNode *curr = route_table;

	while (curr != NULL)
	{
		if (curr->destination == dest && curr->next_hop == next)
		{
			// 1. The route exists and still alive, then we set flag to link down and reset TTL
			if (curr->flags[0] == 'L' && curr->flags[1] == 'U')
			{
				curr->flags[1] = 'D';
				curr->metric = metric;
				curr->TTL = 0;

				return;
			}

			// 2. The route exist and already down. We keep settings unchanged
			else if (curr->flags[0] == 'L' && curr->flags[1] == 'D')
			{
				return;
			}
		}

		prev = curr;
		curr = curr->next;
	}

	// 3. The route doesn't exist, we still insert into the route table and set flag to link down and set TTL to 0
	if (curr == NULL && prev == NULL) // insert at the front
	{
		route_table = create_new_entry(dest, next, 0, "LD-", metric, 0);
	}

	else if (curr == NULL && prev != NULL) // insert at the end
	{
		prev->next = create_new_entry(dest, next, 0, "LD-", metric, 0);
	}

}

void insertIntoRoutingTable(int dest, int next, int metric)
{
	RouteTableNode *prev = NULL;
	RouteTableNode *curr = route_table;

	// find the suitable position to update or insert new route to routing table
	// 1. The route exists and still alive, then compare the metric
	// 2. The route exists but already down. Then bring router online and reset TTL
	// 3. The route doesn't exist. Then insert into the route table and set flag to link up and set TTL to 0
	while (curr != NULL)
	{
		// Update the routing information
		if (curr->destination == dest && curr->next_hop == next)
		{
			// 1. The route exists and still alive, then compare the metric
			if (curr->flags[0] == 'L' && curr->flags[1] == 'U' && metric < curr->metric)
			{
				curr->metric = metric;
			}

			// 2. The route exists but already down. Then bring router online and reset TTL
			else if (curr->flags[0] == 'L' && curr->flags[1] == 'D')
			{
				curr->flags[1] = 'U';
				curr->metric = metric;

			}
			curr->TTL = 0;

			return;
		}

		prev = curr;
		curr = curr->next;
	}

	// 3. The route doesn't exist. Then insert into the route table and set flag to link up and set TTL to 0
	if (curr == NULL && prev == NULL) // insert at the front
	{
		route_table = create_new_entry(dest, next, 0, "LU-", metric, 0);
	}

	else if (curr == NULL && prev != NULL) // insert at the end
	{
		prev->next = create_new_entry(dest, next, 0, "LU-", metric, 0);
	}

}


int getMetricToRouter(int dest, int next)
{
	for (RouteTableNode *it = route_table; it != NULL; it = it->next)
	{
		if (it->destination == dest && it->next_hop == next)
		{
			return it->metric;
		}
	}
	// printf("Failed to get route for dest = %d, next = %d\n", dest, next);
	return -1;
}


void addEntryToRoutingTable(int dest, int next, int metric)
{
	int new_metric;

	// if the received entry is the router itself, then drop
	if (dest == next)
	{
		return;
	}

	// if the received entry is a dead link, or comes from split horizon with
	// poison reverse
	if (metric == 16)
	{
		insertDeadLinkIntoRoutingTable(dest, next, metric);
		return;
	}

	new_metric = getMetricToRouter(next, next) + metric; // get neighbour
	// if the link in database is already died
	if (getMetricToRouter(dest, next) == 16)
	{
		// if the new link count to 16, then we can consider as a dead link
		if (new_metric >= 16)
		{
			new_metric = 16;
			insertDeadLinkIntoRoutingTable(dest, next, new_metric);
			return;
		}
		insertIntoRoutingTable(dest, next, new_metric);
		return;
	}

	// if the new link count to 16, then we can consider as a dead link
	if (new_metric >= 16)
	{
		new_metric = 16;
		insertDeadLinkIntoRoutingTable(dest, next, new_metric);
		return;
	}

	// other links
	insertIntoRoutingTable(dest, next, new_metric);

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
				"NU-", // no link status in backup neighbour
				backup->metric,
				backup->TTL
				);

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

void printTable(RouteTableNode *table)
{
	RouteTableNode *current = table;

	printf("Router[%d] Routing table\n", router_id);
	printf("Destination    Next_Hop    Port    Metric    Flags    TTL\n");
	while (current != NULL)
	{
		printf("%6d%14d%12d%7d%7c%c%c%9d\n",
			current->destination,
			current->next_hop,
			current->port,
			current->metric,
			current->flags[0], current->flags[1], current->flags[2],
			current->TTL
			);
		current = current->next;
	}
}

void destroyTable(RouteTableNode *table)
{
	free_nodes(table);
}


void removeEntry(int dest, int next)
{
	RouteTableNode *prev = NULL;
	RouteTableNode *curr = route_table;


	while (curr != NULL)
	{
		if (curr->destination == dest && curr->next_hop == next)
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

RouteTableNode * updateTTL(void)
{
	// table of offline links
	RouteTableNode *table = NULL;
	RouteTableNode *temp = NULL;

	for (RouteTableNode *it = route_table;
		it != NULL;
		it = it->next)
	{
		// deal with neighbour router
		if (it->flags[0] == 'N')
		{
			// neighbour router offline
			if (it->flags[1] == 'U' && it->TTL == DEACTIVE_TIME)
			{
				it->TTL = 0;
				it->flags[1] = 'D';
				it->metric = 16;
				// insert to offline table list
				if (table == NULL)
				{
					table = create_new_entry(it->destination, it->next_hop, 0, "-UG", 16, 0);
				}
				else
				{
					temp = table;
					table = create_new_entry(it->destination, it->next_hop, 0, "-UG", 16, 0);
					table->next = temp;
				}

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
				removeEntry(it->destination, it->next_hop);
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
			if (it->flags[1] == 'U' && it->TTL == DEACTIVE_TIME)
			{
				it->TTL = 0;
				it->flags[1] = 'D';
				it->metric = 16;
			}
			else if (it->flags[1] == 'D' && it->TTL == GARBAGE_COLLECTION_TIME)
			{
				removeEntry(it->destination, it->next_hop);
			}
			else
			{
				it->TTL++;
			}
		}
	}

	return table;
}
