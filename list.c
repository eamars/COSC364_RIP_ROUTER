/**
 * Library for processing comma separated list
 * 1, 2, 3 -> ['1', '2', '3']
 * Author: Ran Bao
 */

#include "list.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_STRING_LEN 512



// Create a new node for linar linked list
static LNode *create_new_lin_node(char *string)
{
	// allocate memory for node
	LNode *node = (LNode *) malloc (sizeof(LNode));

	// allocate memory for string
	node->string = (char *) malloc (strlen(string) + 1);

	// copy content to node
	strcpy (node->string, string);

	// set next node to NULL
	node->next = NULL;

	return node;
}

// Append to the head of list
static LNode *insert(LNode *head, char *string)
{

	LNode *prevHead;

	// prevHead can be null
	prevHead = head;

	// replace the head with new node
	head = create_new_lin_node(string);

	// append prevHead to the next to current head
	head->next = prevHead;

	return head;
}

static void free_lin_node(LNode *node)
{
	free(node->string);
	free(node);
}

static void free_lin_nodes(LNode *head)
{
	if (head != NULL)
	{
		free_lin_nodes(head->next);
		free_lin_node(head);
	}
}

static void print_lin_nodes(LNode *head)
{
	if (head != NULL)
	{
		print_lin_nodes(head->next);
		printf("%s\n", head->string);
	}

}

LNode *split(char *raw_string, char splitter)
{
	char string_buf[MAX_STRING_LEN];
	LNode *head = NULL;

	// flags
	int string_index = 0;

	// Initilize buffers
	memset(string_buf, 0, sizeof(string_buf));

	while (*raw_string != '\0')
	{
		// is the splitter
		if (*raw_string == splitter)
		{
			if (string_index != 0)
			{
				head = insert(head, string_buf);
				memset(string_buf, 0, sizeof(string_buf));
				string_index = 0;
			}
		}

		// is the space
		else if (*raw_string == ' ')
		{
			// do nothing
		}

		// is the character
		else
		{
			string_buf[string_index++] = *raw_string;
		}
		raw_string ++;
	}

	// still remained some in buffer
	if (string_index != 0)
	{
		head = insert(head, string_buf);
		memset(string_buf, 0, sizeof(string_buf));
		string_index = 0;
	}
	return head;
}

LNode *getNext(LNode * list)
{
	return list->next;
}

void getString(LNode* list, char *string)
{
	strcpy(string, list->string);
}

size_t getLength(SingleLinkedList head)
{
	size_t length = 0;
	while (head != NULL)
	{
		head = getNext(head);
		length ++;
	}
	return length;
}

void destroyList(SingleLinkedList list)
{
	free_lin_nodes(list);
}
