#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "stack.h"

// returns the current length, in number of items
int lenstack(struct stack *s) {
  return s->index + 1;
}

// pops the stack s
void *popstack(struct stack *s) {
  assert(s->index > -1);
  void *ret = s->data + (s->index * s->isize);
  s->index -= 1;
  return ret;
}

// pushes the value pointed to by val onto the stack s, returns new length
int pushstack(struct stack *s, void *val) {
  if (s->index == (s->size / s->isize) - 1)
    s->data = realloc(s->data, s->size * 2);
  s->size *= 2;
  s->index += 1;
  memcpy(s->data + (s->index * s->isize), val, s->isize);
  return lenstack(s);
}

// constructs a stack with space for numitems items of isize bytes per item
struct stack *constructstack(int isize, int numitems) {
  Stack *ret = malloc(sizeof(Stack));
  ret->data = malloc(isize * numitems);
  ret->isize = isize;
  ret->index = -1;
  ret->size = isize * numitems;
  return ret;
}
