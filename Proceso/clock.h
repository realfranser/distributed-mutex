#ifndef COMMANDS_H_
#define COMMANDS_H_

#include <stdio.h>

/* Clock struct */
struct clock
{
    int *lc;
};
/* Methods */
/* Creates a new clock */
void init_clock(struct clock *logic_clock, int num_proc);
/* Updates the clock with a new tick */
void tick(int *lc, int pd, char *p_name);
/* Compares and updates the clock with an incomming clock */
int incoming_clock(int *my_lc, int *in_lc, int num_proc, int process_id, int in_id);
/* Prints the clock via stdout */
void get_clock(int *lc, int num_proc, char *p_name);

#endif