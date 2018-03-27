/** @file libpriqueue.c
 */

#include <stdlib.h>
#include <stdio.h>

#include "stdbool.h"
#include "libpriqueue.h"


/**
  Initializes the priqueue_t data structure.

  Assumtions
    - You may assume this function will only be called once per instance of priqueue_t
    - You may assume this function will be the first function called using an instance of priqueue_t.
  @param q a pointer to an instance of the priqueue_t data structure
  @param comparer a function pointer that compares two elements.
  See also @ref comparer-page
 */
void priqueue_init(priqueue_t *q, int(*comparer)(const void *, const void *))
{
	q->head = NULL;
  q->tail = NULL;
  q->size = 0;
  q->compare = comparer;
}


/**
  Inserts the specified element into this priority queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr a pointer to the data to be inserted into the priority queue
  @return The zero-based index where ptr is stored in the priority queue, where 0 indicates that ptr was stored at the front of the priority queue.
 */
int priqueue_offer(priqueue_t *q, void *ptr)
{
	node_t* new_node = malloc(sizeof(node_t));
	new_node->prev_node = NULL;
	new_node->next_node = NULL;
	new_node->value = ptr;
	int index = 0;

	if(q->head == NULL){
		q->head = new_node;
		q->tail = new_node;
	}
	else{
		node_t* temp_node = q->head;
		bool found_spot = false;

		while(temp_node != NULL){
			if(q->compare(new_node->value, temp_node->value) < 0){
				if(temp_node->prev_node == NULL){
					q->head = new_node;
				}
				else{
					temp_node->prev_node->next_node = new_node;
				}
				new_node->prev_node = temp_node->prev_node;
				new_node->next_node = temp_node;
				temp_node->prev_node = new_node;
				found_spot = true;
				break;
			}
			temp_node = temp_node->next_node;
			index = index+1;
		}

		if(!found_spot){
			index = q->size;
			temp_node = q->tail;
			temp_node->next_node = new_node;
			new_node->prev_node = temp_node;
			q->tail = new_node;
		}


	}

	q->size = q->size +1;

	return index;
}


/**
  Retrieves, but does not remove, the head of this queue, returning NULL if
  this queue is empty.

  @param q a pointer to an instance of the priqueue_t data structure
  @return pointer to element at the head of the queue
  @return NULL if the queue is empty
 */
void *priqueue_peek(priqueue_t *q)
{
	if(q->head == NULL){
		return NULL;
	}
	else{
		return q->head->value;
	}
}


/**
  Retrieves and removes the head of this queue, or NULL if this queue
  is empty.

  @param q a pointer to an instance of the priqueue_t data structure
  @return the head of this queue
  @return NULL if this queue is empty
 */
void *priqueue_poll(priqueue_t *q)
{
	void* to_return = NULL;
	if(q->head == NULL){
			// to_return = NULL;
	}
	else{
		to_return = q->head->value;
		node_t* temp_next = q->head->next_node;
		free(q->head);
		if(temp_next == NULL){
			q->head = NULL;
			q->tail = NULL;
		}
		else{
			q->head = temp_next;
			temp_next->prev_node = NULL;
		}
		q->size = q->size -1;
	}

	return to_return;
}


/**
  Returns the element at the specified position in this list, or NULL if
  the queue does not contain an index'th element.

  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of retrieved element
  @return the index'th element in the queue
  @return NULL if the queue does not contain the index'th element
 */
void *priqueue_at(priqueue_t *q, int index)
{
	void* to_return = NULL;
	if(q->head == NULL || index >= q->size || index < 0){
		// return NULL;
	}
	else{
		node_t* temp_node = q->head;
		int i = 0;
		while(temp_node != NULL){
			// printf("%s\n", "WHILE LOOP ENTERED\n");
			if(i == index){
				to_return = temp_node->value;
				// printf("%s\n", "FOUND INDEX\n");
				break;
			}
			temp_node = temp_node->next_node;
			i = i+1;
		}
	}

	return to_return;
}


/**
  Removes all instances of ptr from the queue.

  This function should not use the comparer function, but check if the data contained in each element of the queue is equal (==) to ptr.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr address of element to be removed
  @return the number of entries removed
 */
int priqueue_remove(priqueue_t *q, void *ptr)
{
	int hits = 0;
	if(q->head == NULL){
		//hits = 0;
	}
	else{
		node_t* temp_node = q->head;
		while(temp_node != NULL){
			if(temp_node->value == ptr){
				node_t* temp_prev = temp_node->prev_node;
				node_t* temp_next = temp_node->next_node;
				hits = hits+1;
				if(temp_prev == NULL){
					q->head = temp_next;
					if(temp_next != NULL){
						temp_next->prev_node = NULL;
					}
					else{
						q->tail = temp_next;
					}
				}
				else{
					temp_prev->next_node = temp_next;
					temp_next->prev_node = temp_prev;
				}

				free(temp_node);
			}
			temp_node = temp_node->next_node;
		}
	}

	q->size = q->size - hits;
	return hits;
}


/**
  Removes the specified index from the queue, moving later elements up
  a spot in the queue to fill the gap.

  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of element to be removed
  @return the element removed from the queue
  @return NULL if the specified index does not exist
 */
void *priqueue_remove_at(priqueue_t *q, int index)
{
	void* to_return = NULL;
	if(q->head == NULL || index < 0 || index >= q->size){
		//to_return = NULL;
	}
	else{
		node_t* temp_node = q->head;
		int i = 0;
		while(temp_node != NULL){
			if(i == index){
				to_return = temp_node->value;
				node_t* temp_prev = temp_node->prev_node;
				node_t* temp_next = temp_node->next_node;
				if(temp_prev == NULL){
					q->head = temp_next;
					if(temp_next != NULL){
						temp_next->prev_node = NULL;
					}
					else{
						q->tail = temp_next;
					}
				}
				else{
					temp_prev->next_node = temp_next;
					temp_next->prev_node = temp_prev;
				}

				free(temp_node);
			}
			temp_node = temp_node->next_node;
			i = i+1;
		}
	}

	return to_return;
}


/**
  Returns the number of elements in the queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @return the number of elements in the queue
 */
int priqueue_size(priqueue_t *q)
{
	return q->size;
}


/**
  Destroys and frees all the memory associated with q.

  @param q a pointer to an instance of the priqueue_t data structure
 */
void priqueue_destroy(priqueue_t *q)
{
	while(q->head != NULL){
		priqueue_poll(q);
	}
	// free(q);
}
