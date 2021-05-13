#include "clock.h"

/* Increment the value of the process given by its process descriptor
located in logic clock, it also prints the tick action with the format:
process name : action */
void tick(int *lc, int pd, char *p_name)
{
    lc[pd]++;
    printf("%s: TICK\n", p_name);
}

void init_clock(struct clock *logic_clock, int num_proc)
{
    int i;
    for (i = 0; i < num_proc; i++)
        logic_clock->lc[i] = 0;
}

int incoming_clock(int *my_lc, int *in_lc, int num_proc, int process_id, int in_id)
{
    int i, res, greater = 1;
    for (i = 0; i < num_proc; i++)
    {
        if (my_lc[i] < in_lc[i])
        {
            greater = 1;
            break;
        }
        if (my_lc[i] > in_lc[i])
        {
            greater = 0;
        }
    }

    if (greater == 1)
    {
        for (i = 0; i < num_proc; i++)
        {
            if (my_lc[i] > in_lc[i])
            {
                greater = 2;
                break;
            }
            if (my_lc[i] < in_lc[i])
            {
                greater = 1;
            }
        }
    }

    switch (greater)
    {
    case 0:
        /* Puede entrar */
        res = 1;
    case 1:
        /* No puede entrar */
        res = 0;
    case 2:
        if (process_id > in_id)
        {
            /* Puede entrar */
            res = 1;
            break;
        }
        /* No puede entrar */
        res = 0;
    }

    for (i = 0; i < num_proc; i++)
        my_lc[i] = my_lc[i] > in_lc[i] ? my_lc[i] : in_lc[i];

    return res;
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
