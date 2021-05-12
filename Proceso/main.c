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

int puerto_udp;

int main(int argc, char *argv[])
{
  int port, s;
  char line[80], proc[80];
  struct sockaddr_in dir, dir_cliente;

  if (argc < 2)
  {
    fprintf(stderr, "Uso: proceso <ID>\n");
    return 1;
  }

  /* Establece el modo buffer de entrada/salida a l�nea */
  setvbuf(stdout, (char *)malloc(sizeof(char) * 80), _IOLBF, 80);
  setvbuf(stdin, (char *)malloc(sizeof(char) * 80), _IOLBF, 80);

  puerto_udp = 1111; /* Se determina el puerto UDP que corresponda.
                      Dicho puerto debe estar libre y quedar�
                      reservado por dicho proceso. */

  fprintf(stdout, "%s: %d\n", argv[1], puerto_udp);

  if ((s = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
  {
    perror("error creando socket");
    return 1;
  }

  //dir.sin_port = htons(atoi(argv[1]));
  dir.sin_port = puerto_udp;
  dir.sin_addr.s_addr = INADDR_ANY;
  dir.sin_family = PF_INET;

  if (bind(s, (struct sockaddr *)&dir, sizeof(dir)) < 0)
  {
    perror("error en bind");
    close(s);
    return 1;
  }

  for (; fgets(line, 80, stdin);)
  {
    if (!strcmp(line, "START\n"))
      break;

    sscanf(line, "%[^:]: %d", proc, &port);
    /* Habra que guardarlo en algun sitio */

    if (!strcmp(proc, argv[1]))
    { /* Este proceso soy yo */
    }
  }

  /* Inicializar Reloj */

  /* Procesar Acciones */

  return 0;
}
