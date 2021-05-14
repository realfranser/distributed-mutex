
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "hashmap.h"
#include "clock.h"

char get_token(char *linea);

/* My structs */
struct process
{
    int puerto;
    char *id, *dir;
};

int process_compare(const void *a, const void *b, void *pdata)
{
    const struct process *pa = a;
    const struct process *pb = b;
    return strcmp(pa->id, pb->id);
}

bool process_iter(const void *item, void *pdata)
{
    const struct process *p = item;
    printf("%s (puterto=%d)\n", p->id, p->puerto);
    return true;
}

uint64_t process_hash(const void *item, uint64_t seed0, uint64_t seed1)
{
    const struct process *p = item;
    return hashmap_sip(p->id, strlen(p->id), seed0, seed1);
}

struct hashmap *map;

void main()
{
    /*if (!map)
    {
        map = hashmap_new(sizeof(struct process),
                          0, 0, 0, process_hash, process_compare, NULL);
    }

    printf("Number of elements: %d\n", hashmap_count(map));*/

    /*char buffer[3];

    buffer[0] = 'H';
    buffer[1] = 'O';
    buffer[2] = '\0';

    printf("El token es: %c", get_token(buffer));*/

    /*struct clock *logic_clock;
    struct clock *in_clock;

    logic_clock = malloc(sizeof(struct clock));
    in_clock = malloc(sizeof(struct clock));

    logic_clock->lc = malloc(2 * sizeof(int));
    in_clock->lc = malloc(2 * sizeof(int));

    logic_clock->lc[0] = 0;
    logic_clock->lc[1] = 1;

    in_clock->lc[0] = 1;
    in_clock->lc[1] = 0;

    incoming_clock(logic_clock->lc, in_clock->lc, 2, 1, 2);

    printf("This are the new values: %d, %d\n", logic_clock->lc[0], logic_clock->lc[1]);*/
}

char get_token(char *linea)
{
    if (!strcmp(linea, "HO"))
    {
        printf("Ha funcionado\n");
    }

    return 'P';
}