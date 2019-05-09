#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <semaphore.h>

#include <simulador.h>
#include <gamescreen.h>
#include <mapa.h>

void mapa_print(tipo_mapa *mapa)
{
	int i, j;

	for (j = 0; j < MAPA_MAXY; j++)
	{
		for (i = 0; i < MAPA_MAXX; i++)
		{
			tipo_casilla cas = mapa_get_casilla(mapa, j, i);
			printf("%c",cas.simbolo);
			screen_addch(j, i, cas.simbolo);
		}
		printf("\n");
	}
	screen_refresh();
}

typedef struct
{
	int flag_alarm; //El flag que identifica que se ha activado la señal de alarm
	tipo_mapa mapa; //EL mapa
	tipo_nave nave[N_EQUIPOS][N_NAVES];
	int mensaje_simulador_jefe[N_EQUIPOS]; //para indicar los mensajes del jefe que debe leer el proceso simulador
	int mensaje_jefe_simulador[N_EQUIPOS]; //para indicar los mensajes del simulador que tiene que leer cada jefe
	int mensaje_jefe_nave[N_EQUIPOS][N_NAVES];
	int contador_mqqueue;
	int ready;
} sharedMemoryStruct;

int main()
{

	screen_init();

	/*Abrimos la memoria compartida*/
	int fd_shm = shm_open(SHM_NAME, O_RDWR, S_IWUSR);

	if (fd_shm == -1)
	{
		fprintf(stderr, "Error opening the shared memory segment \n");
		return -1;
	}

	/* Map the memory segment */
	sharedMemoryStruct *example_struct = mmap(NULL, sizeof(*example_struct), PROT_WRITE, MAP_SHARED, fd_shm, 0);

	if (example_struct == MAP_FAILED)
	{
		fprintf(stderr, "Error mapping the shared memory segment \n");
		return -1;
	}

	example_struct->flag_alarm = 1;
	example_struct->ready = 1;

	munmap(example_struct, sizeof(*example_struct));

	while(example_struct->ready != 0)
		;

	while (1)
	{
		mapa_print(&(example_struct->mapa));
	}

	screen_end();

	return 0;
}
