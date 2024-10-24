#ifndef ED_STACK_H
#define ED_STACK_H

#include <stddef.h>

#ifndef NULL
#define NULL 0
#endif

struct ed_stack {
  struct ed_stack *next;
};

/**
 * @brief Check if the stack is empty.
 *
 * This function checks if the stack is empty by verifying if the
 * stack pointer is NULL or if the first element of the stack is NULL.
 *
 * @param stack A pointer to the stack (pointer to the head of the stack).
 * @return int 1 if the stack is empty, 0 otherwise.
 */
static inline int ed_stack_empty(struct ed_stack **stack) {
  return stack == NULL || *stack == NULL;
}

/**
 * @brief Get the number of elements in the stack.
 *
 * This function calculates the length of the stack by traversing the linked list
 * starting from the head of the stack until it reaches the end (NULL).
 *
 * @param stack A pointer to the stack (pointer to the head of the stack).
 * @return size_t The number of elements in the stack.
 */
static inline size_t ed_stack_len(struct ed_stack **stack) {
  if (ed_stack_empty(stack)) return 0;

  size_t len = 0;
  struct ed_stack *next = (*stack)->next;
  while (next != NULL) {
    len++;
    next = next->next;
  }

  return len + 1;
}

/**
 * @brief Push an element onto the stack.
 *
 * This function adds an element to the top of the stack. It first checks if the element
 * to be pushed is valid (non-NULL and not already linked). If the stack is empty, the
 * element becomes the new head. Otherwise, the element is added to the top of the stack,
 * and its `next` pointer is set to the previous head.
 *
 * @param stack A pointer to the stack (pointer to the head of the stack).
 * @param elem The element to push onto the stack.
 * @return int 0 on success, -1 if the element is invalid (NULL or already linked).
 */
static inline int ed_stack_push(struct ed_stack **stack, struct ed_stack *elem) {
  if (elem == NULL || elem->next != NULL)
    return -1;

  if (ed_stack_empty(stack)) {
    *stack = elem;
    return 0;
  }

  struct ed_stack *aux = *stack;
  *stack = elem;
  elem->next = aux;

  return 0;
}

/**
 * @brief Pop an element from the stack.
 *
 * This function removes and returns the top element of the stack. If the stack is empty,
 * it returns NULL. Otherwise, the top element is removed from the stack and the next
 * element becomes the new head.
 *
 * @param stack A pointer to the stack (pointer to the head of the stack).
 * @return struct ed_stack* The popped element, or NULL if the stack is empty.
 */
static inline struct ed_stack *ed_stack_pop(struct ed_stack **stack) {
  if (ed_stack_empty(stack)) {
    return NULL;
  }
  struct ed_stack *aux = *stack;
  *stack = aux->next;
  return aux;
}

/**
 * @brief Peek at the top element of the stack.
 *
 * This function returns the top element of the stack without removing it. If the stack
 * is empty, it will return NULL.
 *
 * @param stack A pointer to the stack (pointer to the head of the stack).
 * @return struct ed_stack* The top element of the stack, or NULL if the stack is empty.
 */
static inline struct ed_stack *ed_stack_peek(struct ed_stack **stack) {
  return *stack;
}

#endif
