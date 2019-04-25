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

typedef struct{
	int x;
	int y;
	char action[20];
}Mensaje;

typedef struct{
	int flag_alarm; //El flag que identifica que se ha activado la señal de alarm
	tipo_mapa mapa; //EL mapa
	tipo_nave nave[N_EQUIPOS][N_NAVES];
	} sharedMemoryStruct;


void manejador_SIGALRM(int sig) {
	/*Abrimos la memoria compartida*/
	int fd_shm = shm_open(SHM_NAME, O_RDWR, 0); 
	
	if(fd_shm == -1){
		fprintf (stderr, "Error opening the shared memory segment \n");
		return;
	}
	
	/* Map the memory segment */
	sharedMemoryStruct * example_struct = mmap(NULL, sizeof(*example_struct),PROT_READ, MAP_SHARED, fd_shm, 0);
	
	if(example_struct == MAP_FAILED){
		fprintf (stderr, "Error mapping the shared memory segment \n");
		return;
	}
	
	example_struct->flag_alarm = 1;

	munmap(example_struct, sizeof(*example_struct));
}

Mensaje ship_action(tipo_mapa mapa, int orix, int oriy, int range){
	
	int i, x, y;
	Mensaje action;


	//Search for the nearest ship, in circles profressively larger around the ship
	for(i = 0; i < MAPA_MAXX; i++){//Choose the circle radius
		for(x = -i; x<=i; x++){
			for(y = -i; y<=i; y++){
				if(x == i || y == i || x == -i || y == -i){//Only those in the circunference
					if(!mapa_is_casilla_vacia(&mapa, y, x))
						if(mapa_get_distancia(&mapa, oriy, orix, y, x) <= range){
							action.x = x;
							action.y = y;
							action.action = "ATAQUE";
							return action;
						}
						else{//Move closer
							if(x > orix)
								x = orix++;
							else
								x = orix--;
							
							if(y > oriy)
								y = oriy++;
							else
								y = oriy--;

							action.x = x;
							action.y = y;
							action.action = "MOVER";
							return action;
						}
				}
			}
		}
	}
}


int main() {
	
	/***************************************************************************/
	/***********************Inicialicamos variables*****************************/
	/***************************************************************************/
	int i, j, aux1, aux2, equipos_vivos; //Seguro que hace falta
	int end_simulation = 0;
	int ret = 0;
	struct sigaction act;

	char string_turno[] = "TURNO";
	char string_fin[] = "FIN";

	pid_t pid_boss, pid_ship; //Para los procesos hijos

	/*Necesito una pipe para comunicarme con el jefe de cada equipo*/
	int pipe_simulador_jefe[N_EQUIPOS][2];
	int nbytes, pipe_status[N_EQUIPOS];

	/*Cola de mensajes para comunicacion entre naves y simulador*/
	mqd_t msg_queue;
	Mensaje action;
	
	struct mq_attr attributes = {
			.mq_flags = 0,
			.mq_maxmsg = 10,
			.mq_curmsgs = 0,
			.mq_msgsize = sizeof(Mensaje)
	};

	/***************************************************************************/
	/***********************Inicialicamos recursos******************************/
	/***************************************************************************/

	/*************************Memoria compartida********************************/
	int fd_shm = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_EXCL,S_IRUSR | S_IWUSR);
	if(fd_shm == -1){
		printf("[ERROR] No se ha creado correctamente la memoria compartida");
		return -1;
	}

	int error = ftruncate(fd_shm, sizeof(sharedMemoryStruct));

	if(error == -1){fprintf (stderr, "Error resizing the shared memory segment \n");
		shm_unlink(SHM_NAME);
		return EXIT_FAILURE;
	
	}
	/* Mapeamos la memoria compartida */
	sharedMemoryStruct * shared_memory = mmap(NULL, sizeof(*shared_memory),
										PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);
	if(shared_memory == MAP_FAILED){
		fprintf (stderr, "Error mapping the shared memory segment \n");
		shm_unlink(SHM_NAME);
		return EXIT_FAILURE;
	}

	/*La memoria compartida ya esta creada*/
	
	/********************************el mapa************************************/

	/*Inicializamos todas las casillas con vacio.*/
	mapa_restore(&shared_memory->mapa);

	/*Inicializo cada nave*/
	for(i = 0; i<N_EQUIPOS; i++){
		for(j=0; j<N_NAVES; j++){
			shared_memory->nave[i][j].equipo = i;
			shared_memory->nave[i][j].vida = VIDA_MAX;
			shared_memory->nave[i][j].numNave = j;
			shared_memory->nave[i][j].viva = 1;
		}
	}

	/*Pongo a cada equipo en una parte*/
	aux1 = 0;
	aux2 = 0;
	for(i = 0; i<N_EQUIPOS; i++){
		for(j=0; j<N_NAVES; j++){
			shared_memory->nave[i][j].vida = VIDA_MAX;
			shared_memory->nave[i][j].posx = i+aux1;
			shared_memory->nave[i][j].posy = j+aux2;
			shared_memory->nave[i][j].viva = true;
			mapa_set_nave(&shared_memory->mapa, shared_memory->nave[i][j]);
		}
		aux1 += 5;
		aux2 += 5;
	}
	
	/************************Creamos la cola de mensajes************************/
	msg_queue = mq_open(MQ_NAME,
				O_CREAT, O_RDWR,
				&attributes);
   
	if(msg_queue == (mqd_t)-1){
		printf("[ERROR] Opening the message queue");
		return -1;
	}

	/***************************************************************************/
	/***********************Creamos los procesos jefes**************************/
	/***************************************************************************/

	for(i=0; i<N_EQUIPOS; i++){
		pid_boss = fork();

		if(pid_boss < 0){ //Caso de error
			printf("[ERROR] ha fallado el fork para el equipo %d\n",i);
			munmap(shared_memory, sizeof(*shared_memory));
			shm_unlink(SHM_NAME);
			return -1;
		}

		else if(pid_boss == 0){ //Hijo aka proceso jefe

		/***********Creamos los procesos nave***********/
			for(j = 0; j < N_NAVES; j++){
				pid_ship = fork();

				if(pid_ship < 0){//Caso de error
					printf("[ERROR] ha fallado el fork para la nave %d"
					" en el equipo %d\n", j, i);
					munmap(shared_memory, sizeof(*shared_memory));
					shm_unlink(SHM_NAME);
					return -1;
				}
				else if(pid_ship = 0){//Caso hijo (Nave)
					action = ship_action();

					if(mq_send(msg_queue, (char *)&action, sizeof(action), 1) == -1){
						printf("[ERROR] Enviando la accion al simulador"
						"desde la nave %d in team %d", j, i);
						return -1;
					}
				}
			}
		
			/*inicializamos la pipe con la que se comunicara con el jefe*/
			/* Cierre del descriptor de salida en el jefe */
			close(pipe_simulador_jefe[i][1]);

			/*Bucle que se ejecuta hasta el final de la partida*/
			while(end_simulation == 0){

			}
			/*Termina el proceso cuando termina la simulacion*/
			exit(EXIT_SUCCESS);
		}

		else{
			/*El proceso padre se comunicara con el hijo mediante tuberias*/
			/* Cierre del descriptor de entrada en el simulador */
			close(pipe_simulador_jefe[i][0]);
		}

	}

	/*Proceso simulador tras crear a los jefes*/

	/*Hacemos todo lo relativo a la alarma*/
	sigemptyset(&(act.sa_mask));
	act.sa_flags = 0;
	/* Se arma la señal SIGALRM. */
	act.sa_handler = manejador_SIGALRM;
	if (sigaction(SIGALRM, &act, NULL) < 0) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}

	/*Montamos la alarma*/
	if (alarm(ALARM))
		fprintf(stderr, "Existe una alarma previa establecida\n");

	while(end_simulation == 0){
		/*Cuando recibe la señal de alarma*/
		if(shared_memory->flag_alarm == 1){
			/*Restaurar el mapa*/
			mapa_restore(&shared_memory->mapa);

			/*Comprobar condicion de ganador*/
			equipos_vivos = 0;
			for(i=0; i<N_EQUIPOS; i++){
				for(j=0; j<N_NAVES; j++){
					/*Si hay al menos una nave viva en algun equipo*/
					if(shared_memory->nave[i][j].vida == true){
						equipos_vivos++;
						break;
					}
				}
				/*Aun hay equipos compitiendo*/
				if (equipos_vivos == 2){
					break;
				}
			}

			/*Terminamos la partida. Hay un ganador*/
			if(equipos_vivos == 1){
				for (i=0; i<N_EQUIPOS; i++){
					write(pipe_simulador_jefe[i][1], string_fin, strlen(string_fin));
				}
			}
			/*Nuevo turno para todos los equipos*/
			else{
				for (i=0; i<N_EQUIPOS; i++){
					write(pipe_simulador_jefe[i][1], string_turno, strlen(string_turno));
				}
			shared_memory->flag_alarm = 0;
			}
			
		}

	}



	/*Esperamos a los procesos hijos*/
	for(i=0; i<N_EQUIPOS; i++){
		wait(0);
	}

	/*Liberamos recursos*/
	munmap(shared_memory, sizeof(*shared_memory));
	shm_unlink(SHM_NAME);

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




