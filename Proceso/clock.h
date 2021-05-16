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
void tick(int *lc, int pd, char *p_name, int print_mode);
/* Compares and updates the clock with an incomming clock */
void update_clock(int *my_lc, int *in_lc, int num_proc);
/* Prints the clock via stdout */
void get_clock(int *lc, int num_proc, char *p_name);
/* Loads the clock section of the message */
void load_message_clock(int *message_clock, int *lc, int num_proc);

#endif