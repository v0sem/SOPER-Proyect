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

#include <simulador.h>
#include <gamescreen.h>
#include <mapa.h>


void mapa_print(tipo_mapa *mapa)
{
	int i,j;

	for(j=0;j<MAPA_MAXY;j++) {
		for(i=0;i<MAPA_MAXX;i++) {
			tipo_casilla cas=mapa_get_casilla(mapa,j, i);
			//printf("%c",cas.simbolo);
			screen_addch(j, i, cas.simbolo);
		}
		//printf("\n");
	}
	screen_refresh();
}


int main() {


	screen_init();



	screen_end();


	exit(EXIT_SUCCESS);
}
