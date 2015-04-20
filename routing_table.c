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
};

// global variable
RTableNode *routing_table;

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
		// next_hop is unique
		if (head->row[NEXT_HOP] == next_hop)
		{
			// update the metric and addr if old metric > new_metric
			if (head->row[METRIC] > metric)
			{
				head->row[ADDR] = addr;
				head->row[METRIC] = metric;
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
		printf("NEXT_HOP = %d, PORT = %d, ADDR = %d, METRIC = %d\n",
			head->row[NEXT_HOP],
			head->row[PORT],
			head->row[ADDR],
			head->row[METRIC]
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

static int getAllHops_helper(RTableNode *head, int *hops)
{
	static int index = 0;
	int count = 0;

	if (head != NULL)
	{
		count += getAllHops_helper(head->left, hops);
		hops[index] = head->row[NEXT_HOP];
		hops[++index] = -1;
		count++;

		count += getAllHops_helper(head->right, hops);
	}

	return count;
}

int getAllHops(int *hops)
{
	return getAllHops_helper(routing_table, hops);
}

int getNumEntry_helper(RTableNode *head)
{
	int count = 0;

	if (head != NULL)
	{
		count += getNumEntry_helper(head->left);
		count++;
		count += getNumEntry_helper(head->right);
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
