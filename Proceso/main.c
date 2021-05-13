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
#include "actions.h"
#include "hashmap.h"

/* My consts */
#define MSG 0
#define LOCK 1
#define OK 2

int puerto_udp;

/* Auxiliar functions declaration */
char get_token(char *line);

/* My structs */

struct mutex
{
  char *name;
  int mutex, requested, ok_num, req_num, *req_id;
};

struct process
{
  int puerto;
  char *name, *dir;
};

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

int mutex_compare(const void *a, const void *b, void *pdata)
{
  const struct mutex *pa = a;
  const struct mutex *pb = b;
  return strcmp(pa->name, pb->name);
}

bool mutex_iter(const void *item, void *pdata)
{
  const struct mutex *p = item;
  printf("Mutex: %s\n", p->name);
  return true;
}

uint64_t mutex_hash(const void *item, uint64_t seed0, uint64_t seed1)
{
  const struct mutex *p = item;
  return hashmap_sip(p->name, strlen(p->name), seed0, seed1);
}

/* My global variables */
struct hashmap *map;
struct hashmap *critical_section;
struct clock *logic_clock;

int process_id, num_proc;

char *process_name;

int main(int argc, char *argv[])
{
  int port, s;
  char line[80], proc[80];

  struct sockaddr_in dir, dir_cliente;
  int dir_size;
  char *address;

  struct process *new_process;

  char token;

  /* Comprobamos que el numero de argumentos sea correcto */
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

  dir.sin_port = htons(0);
  dir.sin_addr.s_addr = INADDR_ANY;
  dir.sin_family = AF_INET;

  /* Hacemos el bind para obtener direccion y puerto */
  if (bind(s, (struct sockaddr *)&dir, sizeof(dir)) < 0)
  {
    perror("error en bind");
    close(s);
    return 1;
  }

  /* Obtenemos direccion del puerto y el size del socket address */
  if (getsockname(s, (struct sockaddr *)&dir, (socklen_t *)&dir_size) < 0)
  {
    perror("error en getsockname");
    close(s);
    return 1;
  }

  /* Guardamos el puerto obtenido para este proceso */
  puerto_udp = dir.sin_port;
  fprintf(stdout, "%s: %d\n", argv[1], puerto_udp);

  /* Guardamos el nombre y la direccion */
  address = inet_ntoa(dir.sin_addr);
  process_name = argv[1];

  /* En caso de que el hashmap de procesos no exista, lo creamos */
  if (!map)
  {
    map = hashmap_new(sizeof(struct process),
                      0, 0, 0, process_hash, process_compare, NULL);
  }

  if (!critical_section)
  {
    critical_section = hashmap_new(sizeof(struct mutex),
                                   0, 0, 0, mutex_hash, mutex_compare, NULL);
  }

  for (; fgets(line, 80, stdin);)
  {
    if (!strcmp(line, "START\n"))
      break;

    sscanf(line, "%[^:]: %d", proc, &port);
    /* Habra que guardarlo en algun sitio */

    /* Asignamos el numero de elementos del mapa como id del proceso */
    /* Este proceso soy yo */
    if (!strcmp(proc, argv[1]))
      process_id = hashmap_count(map);

    /* Definir el nuevo proceso */
    new_process = malloc(sizeof(struct process));
    /* Declarar puerto */
    new_process->puerto = puerto_udp;
    /* Declarar address */
    new_process->dir = (char *)malloc(strlen(address));
    strcpy(new_process->dir, address);
    /* Declarar nombre del proceso */
    new_process->name = (char *)malloc(strlen(process_name));
    strcpy(new_process->name, process_name);

    /* Insertar el proceso en al hashmap */
    if (hashmap_set(map, new_process) == NULL)
    {
      perror("new process couldn't be added");
      return 1;
    }
  }

  /* Obtener numero de procesos actuales */
  if ((num_proc = hashmap_count(map)) == 0)
    printf("empty hashmap\n");

  /* Inicializar Reloj */
  logic_clock = malloc(sizeof(struct clock));
  logic_clock->lc = (int *)malloc(num_proc * sizeof(int));
  init_clock(logic_clock, num_proc);

  /* Procesar Acciones */
  for (; fgets(line, 80, stdin);)
  {
    char action[80], section[80], buffer[80];
    char *client_name;

    token = get_token(line);

    switch (token)
    {
    case 'E':
      /* Action: EVENT */
      tick(logic_clock->lc, process_id, process_name);
      break;

    case 'G':
      /* Action: GETCLOCK */
      get_clock(logic_clock->lc, num_proc, process_name);
      break;

    case 'R':
      /* Action: RECEIVE */
      if (read(s, buffer, sizeof(buffer)) < 0)
      {
        fprintf(stderr, "Error leyendo mensaje en el RECEIVE\n");
        free(logic_clock->lc);
        free(logic_clock);
        hashmap_free(map);
        return 1;
      }

      int rec_action = buffer[0];
      /* Get the client process from the hashmap */
      struct process *client_process;
      client_process = hashmap_get(map, &(struct process){.puerto = (int)buffer[1]});
      client_name = malloc(strlen(client_process->name));
      client_name = client_process->name;

      char action_strings[3][10] = {
          {'M', 'S', 'G', '\0'},
          {'L', 'O', 'C', 'K', '\0'},
          {'O', 'K', '\0'}};

      /* Print receive message and make one tick */
      if (rec_action == MSG || rec_action == LOCK || rec_action == OK)
      {
        printf("%s: RECEIVE(%s,%s)\n", process_name, action_strings[rec_action], client_name);
        tick(logic_clock->lc, process_id, process_name);
      }

      /* Receive the incomming clock via message and compare to this clock */
      struct clock *in_clock;
      int offset = 2;
      int i;
      int in_id = buffer[1];

      in_clock = malloc(sizeof(struct clock));
      in_clock->lc = malloc(num_proc * sizeof(int));

      for (i = 0; i < num_proc; i++)
        in_clock->lc[i] = buffer[i + offset];

      /* Returns 1 if the process has priority over incomming process */
      int allowed = incoming_clock(logic_clock->lc, in_clock->lc, num_proc, process_id, in_id);

      /* Gets the selected mutex */
      struct mutex *mtx;
      char *msg_name;
      msg_name = malloc(strlen(&buffer[num_proc + offset]));
      strcpy(msg_name, &buffer[num_proc + offset]);

      mtx = hashmap_get(critical_section, &(struct mutex){.name = msg_name});

      if (rec_action == LOCK)
      {
        /* If mutex doesn't exist, feel free to enter critical section */
        if (!mtx)
        {
          tick(logic_clock->lc, process_id, process_name);
          /* Enviar mensaje */
          /* Insertar en seccion critica */
        }
        else
        {
          if (mtx->mutex == 1)
          {
            /* Poner proceso a espera */
          }
          else if (mtx->requested == 1)
          {
            if (allowed)
            {
              tick(logic_clock->lc, process_id, process_name);
              /* Enviar mensaje */
            }
            else
            {
              /* Poner proceso a esperar */
            }
          }
          else
          {
            tick(logic_clock->lc, process_id, process_name);
            /* Enviar mensaje */
          }
        }
      }

      if (rec_action == OK)
      {
        mtx->ok_num--;
        if (mtx->ok_num == 0)
        {
          /* Hacer mutex */
        }
      }

    case 'F':
      /* Action: FINISH */
      free(buffer);

      /*-----------------------------------------------------------------------------
-----------------------------------------------------------------------------*/
    default:
      /* Check for messageto, lock and unlock */
      sscanf(line, "%s %s", action, section);
      token = get_token(action);

      switch (token)
      {
      case 'M':
        /* Action: MESSAGETO */
        tick(logic_clock->lc, process_id, process_name);
        /* Enviar mensaje */
        break;

      case 'L':
        /* Action: LOCK */
        /* Get section name */
        char *selected_critical_section;
        selected_critical_section = malloc(strlen(section));
        strcpy(selected_critical_section, section);
        /* Get existing section or create it otherwise */
        struct mutex *mtx;
        mtx = hashmap_get(critical_section, &(struct mutex){.name = selected_critical_section});
        if (!mtx)
        {
          /* TODO: create and add new mutex 
          struct mutex *new_mutex = malloc(sizeof(struct mutex));
          hashmap_set(mtx,)
          break; */
        }
        /* Solicitar mutex existente */
        break;

      case 'U':
        /* Action: UNLOCK */
        /* Get section name */
        char *selected_critical_section;
        selected_critical_section = malloc(strlen(section));
        strcpy(selected_critical_section, section);

        /* TODO: enviar mensaje */

        /* Get existing section or create it otherwise */
        struct mutex *mtx;
        mtx = hashmap_get(critical_section, &(struct mutex){.name = selected_critical_section});

        mtx->req_id = (int *)malloc(sizeof(int));
        mtx->mutex = 0;
        break;

      default:
        /* invalid token */
      }
    }
  }

  return 0;
}

char get_token(char *line)
{
  if (!strcmp(line, "EVENT\n"))
    return 'E';
  if (!strcmp(line, "GETCLOCK\n"))
    return 'G';
  if (!strcmp(line, "MESSAGETO"))
    return 'M';
  if (!strcmp(line, "RECEIVE\n"))
    return 'R';
  if (!strcmp(line, "LOCK"))
    return 'L';
  if (!strcmp(line, "UNLOCK"))
    return 'U';
  if (!strcmp(line, "FINISH\n"))
    return 'F';
  /* In case non of the defined actions are found, X represents error */
  return 'X';
}
