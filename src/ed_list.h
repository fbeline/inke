/*
 * ed_list.h
 *
 * A simple implementation of a circular doubly linked list in C.
 *
 * This library provides functions to manage a circular doubly linked
 * list data structure, allowing for the following operations:
 * - Appending elements to the list
 * - Removing elements from the list
 * - Calculating the length of the list
 *
 * The `ed_list` structure represents each element in the list,
 * with pointers to both the previous and next elements, allowing for
 * efficient traversal in both directions.
 *
 * Functions:
 * - `size_t ed_list_len(struct ed_list **list)`: Returns the number
 *   of elements in the list. Returns 0 if the list is empty or NULL.
 *
 * - `int ed_list_append(struct ed_list **list, struct ed_list *elem)`:
 *   Adds an element to the end of the list. Returns 0 on success and
 *   -1 if the element is invalid.
 *
 * - `int ed_list_remove(struct ed_list **list, struct ed_list *elem)`:
 *   Removes a specified element from the list. Returns 0 on success
 *   and -1 if the list or element is NULL or if the element is not found.
 *
 *
 * Author: Felipe Beline Baravieira
 * Date: 2024-10-24
 * License: MIT
 */

#ifndef ED_QUEUE_H
#define ED_QUEUE_H

#include <stddef.h>

#ifndef NULL
#define NULL 0
#endif

/**
 * @struct ed_list
 * @brief A structure representing a node in a circular doubly linked list.
 *
 * Each node points to both the previous and next nodes in the list.
 */
struct ed_list {
   struct ed_list *prev;
   struct ed_list *next;
};

/**
 * @brief Checks if the list is empty.
 *
 * This function determines whether a doubly indirect pointer to a linked list
 * structure (`ed_list`) is empty. It achieves this by performing a null check
 * on both the pointer to the list (`list`) and the first element of the list
 * (`*list`). If either of these is `NULL`, the list is considered empty.
 *
 * @param list Pointer to a pointer to the head of the list.
 * @return An integer value indicating whether the list is empty:
 *         - Returns `1` (true) if the list is empty (either `list` or `*list`
 *           is `NULL`).
 *         - Returns `0` (false) if the list is not empty.
 */
static inline int ed_list_empty(struct ed_list **list) {
  return list == NULL || *list == NULL;
}

/**
 * @brief Calculates the length of the list.
 *
 * This function traverses the list starting from the head and counts
 * the number of nodes until it loops back to the head.
 *
 * @param list Pointer to a pointer to the head of the list.
 * @return The number of elements in the list. Returns 0 if the list
 *         is empty or NULL.
 */
static inline size_t ed_list_len(struct ed_list **list) {
  if (ed_list_empty(list)) return 0;

  size_t len = 0;
  struct ed_list *head = *list;
  struct ed_list *next = head->next;
  while (next != NULL && head != next) {
    len++;
    next = next->next;
  }

  return len + 1;
}

/**
 * @brief Appends an element to the end of the list.
 *
 * This function adds a new element to the circular doubly linked list.
 * If the list is empty, the element becomes the head of the list.
 *
 * @param list Pointer to a pointer to the head of the list.
 * @param elem Pointer to the element to be added to the list.
 * @return 0 on success, -1 if the element is NULL, already in a list,
 *         or if the operation fails.
 */
static inline int ed_list_append(struct ed_list **list, struct ed_list *elem) {
  if (elem == NULL || elem->next != NULL || elem->prev != NULL)
    return -1;

  if (ed_list_empty(list)) {
    *list = elem;
    elem->next = elem;
    elem->prev = elem;
    return 0;
  }

  struct ed_list *head = *list;
  struct ed_list *tail = (*list)->prev;
  elem->prev = tail;
  elem->next = head;

  head->prev = elem;
  tail->next = elem;

  return 0;
}

/**
 * @brief Removes an element from the list.
 *
 * This function removes a specified element from the circular doubly linked list.
 * If the list becomes empty as a result of the removal, the head pointer is set to NULL.
 *
 * @param list Pointer to a pointer to the head of the list.
 * @param elem Pointer to the element to be removed from the list.
 * @return 0 on success, -1 if the list or element is NULL,
 *         or if the operation fails.
 */
static inline int ed_list_remove(struct ed_list **list, struct ed_list *elem) {
  if (list == NULL || *list == NULL || elem == NULL) {
    return -1;
  }

  struct ed_list *head = *list;

  if (head->next == head) {
    if (head == elem) {
      *list = NULL;
      head->next = NULL;
      head->prev = NULL;
      return 0;
    } else {
      return -1;
    }
  }

  struct ed_list *current = head;
  do {
    if (current == elem) {
      current->prev->next = current->next;
      current->next->prev = current->prev;

      if (current == head) {
        if (current->next == head) {
          *list = NULL;
        } else {
          *list = current->next;
        }
      }
      current->next = NULL;
      current->prev = NULL;

      return 0;
    }
    current = current->next;
  } while (current != head);

  return -1;
}

#endif
