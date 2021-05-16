/* DMUTEX (2009) Sistemas Operativos Distribuidos
 * C�digo de Apoyo
 *
 * ESTE C�DIGO DEBE COMPLETARLO EL ALUMNO:
 *    - Para desarrollar las funciones de mensajes, reloj y
 *      gesti�n del bucle de tareas se recomienda la implementaci�n
 *      de las mismas en diferentes ficheros.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

/* Include my libs */
#include "clock.h"
#include "hashmap.h"

/* My consts */
#define MSG 0
#define LOCK 1
#define OK 2

#define OFFSET 2

int puerto_udp;

/* My structs */

struct message
{
  char *process_name;
  int *clock;
};

struct process
{
  int puerto;
  char *name;
};

/* Process HashMap methods */

int process_compare(const void *a, const void *b, void *pdata)
{
  const struct process *pa = a;
  const struct process *pb = b;
  return strcmp(pa->name, pb->name);
}

bool process_iter(const void *item, void *pdata)
{
  const struct process *p = item;
  printf("%s (puterto=%d)\n", p->name, p->puerto);
  return true;
}

uint64_t process_hash(const void *item, uint64_t seed0, uint64_t seed1)
{
  const struct process *p = item;
  return hashmap_sip(p->name, strlen(p->name), seed0, seed1);
}

/* My global variables */

int process_id;

char *process_name;

/* Auxiliar functions */
char get_token(char *line);

/* Main function */
int main(int argc, char *argv[])
{
  int port, s;
  char line[80], proc[80];

  struct clock *logic_clock;
  struct hashmap *process_map;
  struct process *new_process;

  struct sockaddr_in addr;
  socklen_t addr_len;

  char *address;

  if (argc < 2)
  {
    fprintf(stderr, "Uso: proceso <ID>\n");
    return 1;
  }

  /* Establece el modo buffer de entrada/salida a l�nea */
  setvbuf(stdout, (char *)malloc(sizeof(char) * 80), _IOLBF, 80);
  setvbuf(stdin, (char *)malloc(sizeof(char) * 80), _IOLBF, 80);

  /* Creacion del socket UDP */
  if ((s = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
  {
    perror("error creando socket");
    return 1;
  }

  bzero((char *)&addr, sizeof(addr));
  addr.sin_port = htons(0);
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_family = AF_INET;

  /* Hacemos el bind para obtener direccion y puerto */
  if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    perror("error en bind");
    close(s);
    return 1;
  }

  addr_len = sizeof(addr);

  /* Obtenemos direccion del puerto y el size del socket address */
  if (getsockname(s, (struct sockaddr *)&addr, (socklen_t *)&addr_len) < 0)
  {
    perror("error en getsockname");
    close(s);
    return 1;
  }

  /* Guardamos el puerto obtenido para este proceso */
  puerto_udp = ntohs(addr.sin_port);
  fprintf(stdout, "%s: %d\n", argv[1], puerto_udp);

  /* Guardamos el nombre y la direccion del proceso */
  //address = inet_ntoa(addr.sin_addr);
  process_name = argv[1];

  if (!process_map)
  {
    process_map = hashmap_new(sizeof(struct process),
                              0, 0, 0, process_hash, process_compare, NULL);
  }
  for (; fgets(line, 80, stdin);)
  {
    if (!strcmp(line, "START\n"))
      break;

    sscanf(line, "%[^:]: %d", proc, &port);

    if (!strcmp(proc, argv[1]))
      process_id = hashmap_count(process_map);

    /* Creacion del nuevo proceso */
    new_process = malloc(sizeof(struct process));
    /* Declarar puerto */
    new_process->puerto = port;
    /* Declarar address 
      new_process->dir = (char *)malloc(strlen(proc));
      strcpy(new_process->dir, proc);*/
    /* Declarar nombre del proceso */
    new_process->name = (char *)malloc(strlen(proc));
    strcpy(new_process->name, proc);

    hashmap_set(process_map, new_process);
  }

  int num_proc = hashmap_count(process_map);
  //printf("num_proc = %d\n", num_proc);
  //hashmap_scan(process_map, process_iter, NULL);

  /* Creacion del logic clock */
  logic_clock = malloc(sizeof(struct clock));
  logic_clock->lc = (int *)malloc(num_proc * sizeof(int));
  init_clock(logic_clock, num_proc);

  char action[80], token;
  bool finish = 0;

  //hashmap_scan(process_map, process_iter, NULL);

  /* TODO: check finish condition here */
  while (fgets(line, 80, stdin))
  {
    int i;

    struct message *msg;
    struct sockaddr_in client_address;
    struct clock *client_clock;

    sscanf(line, "%s %s", action, proc);

    token = get_token(action);

    switch (token)
    {
    case 'G':
      get_clock(logic_clock->lc, num_proc, process_name);
      break;

    case 'E':
      tick(logic_clock->lc, process_id, process_name, 0);
      break;

    case 'F':
      finish = 1;
      break;

    case 'R':
      /* Receive the message and store it */
      if (recv(s, &msg, sizeof(struct message), 0) < 0)
      {
        perror("error en recv");
        close(s);
        return 1;
      }

      update_clock(logic_clock->lc, msg->clock, num_proc);

      fprintf(stdout, "%s: RECEIVE(MSG,%s)|", process_name, msg->process_name);
      tick(logic_clock->lc, process_id, process_name, 1);
      break;

    case 'M':

      tick(logic_clock->lc, process_id, process_name, 2);

      /* Get the process with the same name as proc string */
      struct process *message_process = hashmap_get(process_map, &(struct process){.name = proc});

      /* Allocate memory for the message */
      msg = malloc(sizeof(struct message));

      /* Load the message process name */
      msg->process_name = malloc(strlen(argv[1]));
      strcpy(msg->process_name, argv[1]);

      /* Load the message clock */
      msg->clock = (int *)malloc(sizeof(logic_clock->lc));
      load_message_clock(msg->clock, logic_clock->lc, num_proc);

      /* Load the message client address */
      socklen_t client_size = sizeof(struct sockaddr_in);

      client_address.sin_family = AF_INET;
      client_address.sin_addr.s_addr = INADDR_ANY;
      client_address.sin_port = htons(message_process->puerto);

      /* Send message to client */
      if (sendto(s, &msg, sizeof(struct message), 0, (struct sockaddr *)&client_address, client_size) < 0)
      {
        printf("error en sendto\n");
        close(s);
        return 1;
      }

      fprintf(stdout, "|SEND(MSG,%s)\n", proc);
      break;

    default:
      break;
    }

    /* TODO: check finish condition above */
    if (finish)
      break;
  }

  return 0;
}

char get_token(char *line)
{
  if (!strcmp(line, "EVENT"))
    return 'E';
  if (!strcmp(line, "GETCLOCK"))
    return 'G';
  if (!strcmp(line, "MESSAGETO"))
    return 'M';
  if (!strcmp(line, "RECEIVE"))
    return 'R';
  /* Under development */
  if (!strcmp(line, "LOCK"))
    return 'L';
  /* Under development */
  if (!strcmp(line, "UNLOCK"))
    return 'U';
  if (!strcmp(line, "FINISH"))
    return 'F';
  /* In case non of the defined actions are found, X represents error */
  return 'X';
}
