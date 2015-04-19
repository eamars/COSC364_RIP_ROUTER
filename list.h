/**
 * Library for processing comma separated list
 * 1, 2, 3 -> ['1', '2', '3']
 * Author: Ran Bao
 */

#ifndef LIST_H
#define LIST_H

#include <stddef.h> // size_t

// use SingleLinkedList as the interface
typedef struct lin_node_s LNode;
struct lin_node_s
{
	char *string;
	LNode *next;
};

typedef LNode* SingleLinkedList;

#define ITEM_IN_(X) SingleLinkedList it = X; it != NULL; it = getNext(it)
#define ITEM it


/**
 * split the string into sub-strings
 * @param  raw_string input string
 * @param  splitter   character that split the string (cannot be space)
 * @return            address to new list
 */
SingleLinkedList split(char *raw_string, char splitter);

/**
 * return the next node of current, return NULL if at end
 * @param  list   current node
 * @return        next node
 */
SingleLinkedList getNext(SingleLinkedList list);

/**
 * get the string of current node
 * @param list   current list
 * @param string output string
 */
void getString(SingleLinkedList list, char *string);

size_t getLength(SingleLinkedList head);

/**
 * free the memory
 * @param SingleLinkedList list
 */
void destroyList(SingleLinkedList list);


#endif
