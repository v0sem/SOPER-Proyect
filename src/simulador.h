#ifndef SRC_SIMULADOR_H_
#define SRC_SIMULADOR_H_

#include <stdbool.h>

#define N_EQUIPOS 3 // Número de equipos
#define N_NAVES 3 // Número de naves por equipo

/*** SCREEN ***/
extern char symbol_equipos[N_EQUIPOS]; // Símbolos de los diferentes equipos en el mapa (mirar mapa.c)
#define MAPA_MAXX 20 // Número de columnas del mapa
#define MAPA_MAXY 20 // Número de
#define SCREEN_REFRESH 10000 // Frequencia de refresco del mapa en el monitor
#define SYMB_VACIO '.' // Símbolo para casilla vacia
#define SYMB_TOCADO '%' // Símbolo para tocado
#define SYMB_DESTRUIDO 'X' // Símbolo para destruido
#define SYMB_AGUA 'w' // Símbolo para agua

/*** SIMULACION ***/
#define VIDA_MAX 50 // Vida inicial de una nave
#define ATAQUE_ALCANCE 20 // Distancia máxima de un ataque
#define ATAQUE_DANO 10 // Daño de un ataque
#define MOVER_ALCANCE 1 // Máximo de casillas a mover
#define TURNO_SECS 5 // Segundos que dura un turno

/*** NOMBRES GLOBALES ***/
#define MQ_NAME "/mq_actions"
#define SEM "/sem_actions"
#define ALARM 30
#define SHM_NAME "simulador_shm"

/*** MAPA ***/
// Información de nave
typedef struct {
	int vida; // Vida que le queda a la nave
	int posx; // Columna en el mapa
	int posy; // Fila en el mapa
	int equipo; // Equipo de la nave
	int numNave; // Numero de la nave en el equipo
	bool viva; // Si la nave está viva o ha sido destruida
} tipo_nave;

// Información de una casilla en el mapa
typedef struct {
	char simbolo; // Símbolo que se mostrará en la pantalla para esta casilla
	int equipo; // Si está vacia = -1. Si no, número de equipo de la nave que está en la casilla
	int numNave; // Número de nave en el equipo de la nave que está en la casilla
} tipo_casilla;


typedef struct {
	tipo_nave info_naves[N_EQUIPOS][N_NAVES];
	tipo_casilla casillas[MAPA_MAXY][MAPA_MAXX];
	int num_naves[N_EQUIPOS]; // Número de naves vivas en un equipo
} tipo_mapa;

#define SHM_MAP_NAME "/shm_naves"


/**
 * @brief Mueve la nave introducida como argumento a la casilla introducida como argumento
 * @param nave La nave que se cambiará de posicion
 * @param new_x la coordenada x de la nueva casilla a la que se movera la nave
 * @param new_y la coordenada y de la nueva casilla a la que se movera la nave
 * @return Nada (tipo void)
 */
void mover(tipo_mapa *mapa,tipo_nave *nave, int new_x, int new_y);

/**
 * @brief Mueve la nave introducida como argumento a la casilla introducida como argumento
 * @param mapa el mapa en el que se esta jugando la partida
 * @param x la coordenada x de la nueva casilla a la que se movera la nave
 * @param y la coordenada y de la nueva casilla a la que se movera la nave
 * @return Nada (tipo void)
 */
tipo_nave atacar(tipo_mapa *mapa,tipo_nave nave_atacante ,int x, int y);

#endif /* SRC_SIMULADOR_H_ */
