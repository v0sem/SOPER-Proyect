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

#include <mapa.h>


int main() {
    
    /***************************************************************************/
    /***********************Inicialicamos variables*****************************/
    /***************************************************************************/
    int i, j, aux1, aux2; //Seguro que hace falta
    int ret = 0;
    tipo_mapa *mapa = NULL; //El mapa. Ya tiene las naves asignadas.
    tipo_nave nave[N_EQUIPOS][N_NAVES];

    pid_t pid_boss; //Para los procesos hijos

    /*Necesito una pipe para comunicarme con el jefe de cada equipo*/
    int pipe_enviar[N_EQUIPOS][2], pipe_recibir[N_EQUIPOS][2];
    int nbytes, pipe_status[N_EQUIPOS];


    /***************************************************************************/
    /***********************Inicialicamos recursos******************************/
    /***************************************************************************/

    /********************************el mapa************************************/

    /*Le asignamos memoria*/
    mapa = (tipo_mapa*)calloc(1,sizeof(tipo_mapa));
    if (mapa == NULL){
        printf("[ERROR] El mapa no se ha reservado memoria de forma correcta\n");
        return -1;
    }

    /*Inicializamos todas las casillas con vacio.*/
    mapa_restore(mapa);

    /*Inicializo cada nave*/
    for(i = 0; i<N_EQUIPOS; i++){
        for(j=0; j<N_NAVES; j++){
            nave[i][j].vida = VIDA_MAX;
            nave[i][j].equipo = i;
            nave[i][j].numNave = j;
            nave[i][j].viva = 1;
        }
    }

    /*Pongo a cada equipo en una parte*/
    aux1 = 0;
    aux2 = 0;
    for(i = 0; i<N_EQUIPOS; i++){
        for(j=0; j<N_NAVES; j++){
            nave[i][j].vida = VIDA_MAX;
            nave[i][j].posx = i+aux1;
            nave[i][j].posy = j+aux2;
            nave[i][j].viva = true;
            mapa_set_nave(mapa, nave[i][j]);
        }
        aux1 += 5;
        aux2 += 5;
    }

    /***************************************************************************/
    /***********************Creamos los procesos jefes**************************/
    /***************************************************************************/


    for(i=0; i<N_EQUIPOS; i++){
        pid_boss = fork();

        if(pid_boss < 0){ //Caso de error
            printf("[ERROR] ha fallado el fork para el equipo %d\n",i);
            free(mapa);
            return -1;
        }

        else if(pid_boss == 0){ //Hijo aka proceso jefe

        }

        else{ //Proceso padre aka simulador


            
        }

    }



    /*Esperamos a los procesos hijos*/
    for(i=0; i<N_EQUIPOS; i++){
        wait(0);
    }

	/*Liberamos memoria*/
	free(mapa);

    exit(ret);
    
}




/* Funcion encargada de mover una nave a una casilla */
void mover(tipo_mapa *mapa,tipo_nave *nave, int new_x, int new_y){

    /*Control de los argumentos*/
    if(nave == NULL || mapa == NULL){
        printf("[ERROR] Se ha introducido erroneamente el mapa o la nave a mover\n");
    }
        return;

    /*Comprobacion de que la casilla esta en el rango*/
    if(new_x < 0 || new_y < 0 || new_x >= MAPA_MAXX || new_y >= MAPA_MAXY){
        printf("[ERROR] No se puede mover la nave %d del equipo %d a la casilla %d %d porque se sale de los limites del mapa\n",nave->numNave,nave->equipo,new_x,new_y);
        return;
    }

    /*Comprobacion de que nos podemos desplazar esa distancia*/
    if(mapa_get_distancia(mapa, nave->posy, nave->posx, new_y, new_x) > MOVER_ALCANCE){
        printf("[ERROR] la nave no se ha podido mover de la posicion %d %d a la posicion %d %d porque esta muy lejos\n",nave->posx, nave->posy, new_x, new_y);
        return;
    }

    /*Comprobacion de que la casilla no esta ocupada*/
    if(mapa_is_casilla_vacia(mapa, new_x, new_y) == false){
        printf("No se ha podido mover la nave numero %d del equipo %d a la casilla %d %d porque la casilla esta ocupada.\n",nave->numNave,nave->equipo, new_x, new_y);
        return;
    }

    /*Si ha pasado todos los controles anteriores procedemos a desplazarla*/
    /*La casilla vieja estará vacia*/
    mapa_clean_casilla(mapa, nave->posy, nave->posx);

    /*La nave estará en la nueva posicion*/
    nave->posx = new_x;
    nave->posy = new_y;

    /*La nueva casilla tendrá los datos de la nave*/
    mapa_set_nave(mapa, *nave);

    return;
}

/* Funcion que permite atacar a una casilla */
void atacar(tipo_mapa *mapa,tipo_nave nave_atacante ,int x, int y){

    tipo_casilla casilla_objetivo;
    tipo_nave nave_objetivo;

    /*Control de los argumentos*/
    if(mapa == NULL){
        printf("[ERROR] Se ha introducido erroneamente el mapa en la funcion atacar\n");
    }
        return;

    /*Comprobacion de que la casilla esta en el rango*/
    if(x < 0 || y < 0 || x >= MAPA_MAXX || y >= MAPA_MAXY){
        printf("[ERROR] No se puede atacar a la casilla %d %d porque se sale de los limites del mapa\n",x ,y);
        return;
    }

    /*Comprobacion de que podemos atacar a esa distancia*/
    if(mapa_get_distancia(mapa, nave_atacante.posy, nave_atacante.posx, y, x) > ATAQUE_ALCANCE){
        printf("[ERROR] la nave no se ha podido atacar a la casilla %d %d porque esta muy lejos de la nave atacante\n",x ,y);
        return;
    }

    /* Si ha pasado los controles puede atacar a esa casilla. */

    casilla_objetivo = mapa_get_casilla(mapa, y, x);

    /* Primero generamos la animacion */
    mapa_send_misil(mapa, nave_atacante.posy, nave_atacante.posx, y, x);
    
    /* En el caso de que la casilla este vacia: agua*/
    if(casilla_is_vacia(mapa, casilla_objetivo)){
        mapa_set_symbol(mapa, y, x, SYMB_AGUA);
    }

    /* En el caso de que no este vacia: Se daña a la nave que haya en la casilla */
    else{
        nave_objetivo = mapa_get_nave(mapa, casilla_objetivo.equipo, casilla_objetivo.numNave);
        nave_objetivo.vida -= ATAQUE_DANO;
        /*En el caso de que siga con vida*/
        if(nave_objetivo.vida > 0){
            /* Marcamos como tocado */
            mapa_set_symbol(mapa, y, x, SYMB_TOCADO);
            mapa_set_nave(mapa, nave_objetivo);
        }
        /*Si la hemos destruido*/
        else{
            mapa_set_symbol(mapa, y, x, SYMB_DESTRUIDO);
            nave_objetivo.viva = false;
            /* TODO decirle al hijo que la destruya */
        }
    }
    return;
}




