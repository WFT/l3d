#pragma once

// These stacks handle one type at a time
typedef struct stack {
  void *data;	// data
  int isize;	// size of an individual data item
  int index;	// current index
  int size;	// number of bytes allocated for data
} Stack;

// returns the current length, in number of items
int lenstack(struct stack *s);

// pops the stack s
void *popstack(struct stack *s);

// pushes the value pointed to by val onto the stack s
int pushstack(struct stack *s, void *val);

// constructs a stack with space for numitems items of isize bytes per item
struct stack *constructstack(int isize, int numitems);
