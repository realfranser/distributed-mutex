#include "clock.h"

/* Increment the value of the process given by its process descriptor
located in logic clock, it also prints the tick action with the format:
process name : action */
void tick(int *lc, int pd, char *p_name, int print_mode)
{
    lc[pd]++;
    switch (print_mode)
    {
    case 0:
        printf("%s: TICK\n", p_name);
        break;

    case 1:
        printf("TICK\n");
        break;

    default:
        printf("%s: TICK", p_name);
        break;
    }
}

void init_clock(struct clock *logic_clock, int num_proc)
{
    int i;
    for (i = 0; i < num_proc; i++)
        logic_clock->lc[i] = 0;
}

void update_clock(int *my_lc, int *client_lc, int num_proc)
{
    int i;
    for (i = 0; i < num_proc; i++)
        my_lc[i] = my_lc[i] > client_lc[i] ? my_lc[i] : client_lc[i];
}

void get_clock(int *lc, int num_proc, char *p_name)
{
    int i;
    printf("%s: LC[", p_name);
    for (i = 0; i < num_proc; i++)
    {
        if (i > 0)
            printf(",");
        printf("%d", lc[i]);
    }
    printf("]\n");
}

void load_message_clock(int *message_clock, int *lc, int num_proc)
{
    int i;
    for (i = 0; i < num_proc; i++)
        message_clock[i] = lc[i];
}
