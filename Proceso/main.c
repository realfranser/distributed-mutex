/* DMUTEX (2009) Sistemas Operativos Distribuidos
 * C�digo de Apoyo
 *
 * ESTE C�DIGO DEBE COMPLETARLO EL ALUMNO:
 *    - Para desarrollar las funciones de mensajes, logic_clock y
 *      gesti�n del bucle de tareas se recomienda la implementaci�n
 *      de las mismas en diferentes ficheros.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int puerto_udp;

struct process
{

  char *name;
  int port;
};

struct message
{
  char p_name[80];
  int lc[80];
};

struct process *process_list;
int num_proc;

int main(int argc, char *argv[])
{
  int port, s, i, process_id;

  char line[80], proc[80];

  char *process_name;

  struct sockaddr_in addr;
  socklen_t addr_len;

  if (argc < 2)
  {
    fprintf(stderr, "Uso: proceso <ID>\n");
    return 1;
  }

  // Establece el modo buffer de entrada/salida a linea
  setvbuf(stdout, (char *)malloc(sizeof(char) * 80), _IOLBF, 80);
  setvbuf(stdin, (char *)malloc(sizeof(char) * 80), _IOLBF, 80);

  if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
  {
    fprintf(stderr, "Error en la creacion del socket\n");
    close(s);
    return 1;
  }

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(0);

  if (bind(s, (struct sockaddr *)&addr, sizeof(addr)))
  {
    fprintf(stderr, "Error en el bind\n");
    close(s);
    return 1;
  }

  addr_len = sizeof(addr);
  if (getsockname(s, (struct sockaddr *)&addr, &addr_len) < 0)
  {
    fprintf(stderr, "Error en getsockname\n");
    close(s);
    return 1;
  }

  process_name = argv[1];

  puerto_udp = ntohs(addr.sin_port);
  fprintf(stdout, "%s: %d\n", argv[1], puerto_udp);

  process_list = (struct process *)malloc(sizeof(struct process));

  for (; fgets(line, 80, stdin);)
  {
    if (!strcmp(line, "START\n"))
      break;

    sscanf(line, "%[^:]: %d", proc, &port);

    if (!strcmp(proc, argv[1]))
      process_id = num_proc;

    insert_process(port, proc);
  }

  char action[80];
  /* Inicializar logic_clock */
  int logic_clock[num_proc];

  for (i = 0; i < num_proc; i++)
    logic_clock[i] = 0;

  /* Procesar Acciones */

  while (fgets(line, 80, stdin))
  {
    sscanf(line, "%s %s", action, proc);

    if (!strcmp(action, "GETCLOCK"))
    {
      fprintf(stdout, "%s: ", process_list[process_id].name);
      fprintf(stdout, "LC[");

      int j = 0;

      for (j = 0; j < num_proc; j++)
        fprintf(stdout, "%d,", logic_clock[j]);

      fprintf(stdout, "%d]\n", logic_clock[num_proc - 1]);
    }

    if (!strcmp(action, "EVENT"))
    {
      logic_clock[process_id]++;
      fprintf(stdout, "%s: TICK\n", process_list[process_id].name);
    }

    if (!strcmp(action, "FINISH"))
    {
      break;
    }

    struct message msg;
    struct sockaddr_in client_addr;
    int client_clock[num_proc];
    int k = 0;
    if (!strcmp(action, "RECEIVE"))
    {

      recv(s, &msg, sizeof(struct message), 0);

      int o = 0;
      while (o < num_proc)
      {
        client_clock[o] = msg.lc[o];
        o++;
      }
      logic_clock[process_id]++;
      while (k < num_proc)
      {
        if (k != process_id)
        {
          if (client_clock[k] > logic_clock[k])
          {
            logic_clock[k] = client_clock[k];
          }
        }
        k++;
      }

      fprintf(stdout, "%s: RECEIVE(MSG,%s)|TICK\n", process_list[process_id].name, msg.p_name);
    }

    if (!strcmp(action, "MESSAGETO"))
    {
      logic_clock[process_id]++;

      fprintf(stdout, "%s: ", process_list[process_id].name);
      int m = 0;
      int npuerto = port;

      while (m < num_proc)
      {
        if (!strcmp(proc, process_list[m].name))
        {
          bzero((char *)&addr, sizeof(addr));
          addr.sin_family = AF_INET;
          addr.sin_addr.s_addr = INADDR_ANY;
          npuerto = process_list[m].port;
        }
        m++;
      }

      int t = 0;
      while (t < num_proc)
      {
        msg.lc[t] = logic_clock[t];
        t++;
      }

      strcpy(msg.p_name, process_name);

      client_addr.sin_family = AF_INET;
      client_addr.sin_addr.s_addr = INADDR_ANY;
      client_addr.sin_port = htons(npuerto);
      socklen_t tam_dir = sizeof(struct sockaddr_in);

      sendto(s, &msg, sizeof(struct message), 0, (struct sockaddr *)&client_addr, addr_len);
      fprintf(stdout, "TICK|SEND(MSG,%s)\n", proc);
    }
  }
  return 0;
}

int insert_process(int port, char *name)
{
  struct process new_process;

  new_process.port = port;

  new_process.name = (char *)malloc(strlen(name));
  strcpy(new_process.name, name);

  process_list[num_proc++] = new_process;

  process_list = (struct process *)realloc(process_list, (num_proc + 1) * sizeof(struct process));
}