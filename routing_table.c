#include <stdio.h>
#include <stdlib.h>
#include "routing_table.h"
#include "list.h"


typedef struct route_table_s RTableNode;
struct route_table_s
{
	int row[4];
	RTableNode *left;
	RTableNode *right;
	int state;

	int active_timeout; // time to set deactive
	int garbage_timeout; // time to remove
};

enum TSTATE {NORMAL = 0, DEACTIVED};

// global variable
RTableNode *routing_table;

extern const int DEACTIVE_TIME;
extern const int GARBAGE_COLLECTION_TIME;
extern unsigned router_id;

static RTableNode *create_new_node(int next_hop, int port, int addr, int metric)
{
	// allocate memory for node
	RTableNode *node = (RTableNode *) malloc (sizeof(RTableNode));

	// copy variable
	node->row[NEXT_HOP] = next_hop;
	node->row[PORT] = port;
	node->row[ADDR] = addr;
	node->row[METRIC] = metric;

	node->left = NULL;
	node->right = NULL;

	node->state = NORMAL;

	node->active_timeout = 0; // newly created or updated
	node->garbage_timeout = 0;

	return node;
}


static RTableNode *insert (RTableNode *head, int next_hop, int port, int addr, int metric)
{
	if (head == NULL)
	{
		head = create_new_node(next_hop, port, addr, metric);
	}
	else
	{
		// printf("UPDATE: %d %d %d %d\n", next_hop, port, addr, metric);
		// next_hop is unique
		// update if duplicated
		if (head->row[NEXT_HOP] == next_hop)
		{

			if (head->state == NORMAL)
			{
				// update the metric and addr if old metric > new_metric
				// or metric == 16
				if (metric + head->row[METRIC] >= 16)
				{
					head->row[ADDR] = addr;
					head->row[METRIC] = 16;
					head->state = DEACTIVED;
				}
				else if (head->row[METRIC] > metric)
				{
					head->row[ADDR] = addr;
					head->row[METRIC] = metric;
				}
				head->active_timeout = 0; // reset
				head->garbage_timeout = 0;
			}

			else if (head->state == DEACTIVED)
			{
				if (metric >= 16)
				{

				}
				else if (head->row[METRIC] > metric)
				{
					head->row[ADDR] = addr;
					head->row[METRIC] = metric;
					head->state = NORMAL;

					head->active_timeout = 0; // reset
					head->garbage_timeout = 0;
				}

			}


		}

		else if (head->row[NEXT_HOP] > next_hop)
		{
			// insert at left
			head->left = insert(head->left, next_hop, port, addr, metric);
		}
		else
		{
			// insert at right
			head->right = insert(head->right, next_hop, port, addr, metric);
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
		printf("NEXT_HOP = %d, PORT = %d, ADDR = %d, METRIC = %d, AT = %d, GT = %d, STATE = %d\n",
			head->row[NEXT_HOP],
			head->row[PORT],
			head->row[ADDR],
			head->row[METRIC],
			head->active_timeout,
			head->garbage_timeout,
			head->state
		);
		print_table(head->right);
	}
}

static int getValue_helper(RTableNode *head, int next_hop, int selection)
{
	if (head == NULL)
	{
		return -1;
	}
	else
	{
		if (head->row[NEXT_HOP] == next_hop)
		{
			return head->row[selection];
		}
		else if (head->row[NEXT_HOP] > next_hop)
		{
			return getValue_helper(head->left, next_hop, selection);
		}
		else
		{
			return getValue_helper(head->right, next_hop, selection);
		}
	}
}

int getValue(int next_hop, int selection)
{
	return getValue_helper(routing_table, next_hop, selection);
}

void updateTable(int next_hop, int port, int addr, int metric)
{
	routing_table = insert(routing_table, next_hop, port, addr, metric);
}


void createRoutingTableFromConfig(SingleLinkedList list)
{
	int next_hop, port, addr, metric;

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

		updateTable(next_hop, port, addr, metric);
	}

}

void getAllHops_helper(RTableNode *head, int *hops, int index)
{
	if (head != NULL)
	{
		hops[index] = head->row[NEXT_HOP];
		hops[index + 1] = -1;
		getAllHops_helper(head->left, hops, index + 1);
		getAllHops_helper(head->right, hops, index + 2);

	}

}

void getAllHops(int *hops)
{
	getAllHops_helper(routing_table, hops, 0);
}

static int getNumEntry_helper(RTableNode *head)
{
	int count = 0;

	if (head != NULL)
	{
		count = 1 + getNumEntry_helper(head->left) + getNumEntry_helper(head->right);
	}

	return count;
}

int getNumEntry(void)
{
	return getNumEntry_helper(routing_table);
}


void printRoutingTable(void)
{
	print_table(routing_table);
}


void destroyRoutingTable(void)
{
	free_nodes(routing_table);
}

static RTableNode *timeoutUpdate_helper(RTableNode *head)
{
	if (head != NULL)
	{
		head->left = timeoutUpdate_helper(head->left);
		if (head->row[PORT] != 0)
		{
			if (head->state == NORMAL)
			{
				head->active_timeout += 1;
			}
			else if (head->state == DEACTIVED)
			{
				head->garbage_timeout += 1;
			}

		}
		if (head->active_timeout > DEACTIVE_TIME)
		{
			head->active_timeout = 0;
			head->row[METRIC] = 16;
			head->state = DEACTIVED;
		}
		head->right = timeoutUpdate_helper(head->right);
	}
	return head;
}

void timeoutUpdate(void)
{
	timeoutUpdate_helper(routing_table);
}

