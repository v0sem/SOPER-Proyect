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
<<<<<<< HEAD
=======
	int mensaje_simulador_jefe[N_EQUIPOS]; //para indicar los mensajes del jefe que debe leer el proceso simulador
	int mensaje_jefe_simulador[N_EQUIPOS]; //para indicar los mensajes del simulador que tiene que leer cada jefe
>>>>>>> a011b2dd2e09389daf3532665d892fa94cee3623
	} sharedMemoryStruct;


void manejador_SIGALRM(int sig) {
	/*Abrimos la memoria compartida*/
<<<<<<< HEAD
	int fd_shm = shm_open(SHM_NAME, O_RDWR, 0); 
=======
	int fd_shm = shm_open(SHM_NAME, O_RDWR, S_IWUSR); 
>>>>>>> a011b2dd2e09389daf3532665d892fa94cee3623
	
	if(fd_shm == -1){
		fprintf (stderr, "Error opening the shared memory segment \n");
		return;
	}
	
	/* Map the memory segment */
<<<<<<< HEAD
	sharedMemoryStruct * example_struct = mmap(NULL, sizeof(*example_struct),PROT_READ, MAP_SHARED, fd_shm, 0);
=======
	sharedMemoryStruct * example_struct = mmap(NULL, sizeof(*example_struct),PROT_WRITE, MAP_SHARED, fd_shm, 0);
>>>>>>> a011b2dd2e09389daf3532665d892fa94cee3623
	
	if(example_struct == MAP_FAILED){
		fprintf (stderr, "Error mapping the shared memory segment \n");
		return;
	}
	
	example_struct->flag_alarm = 1;
<<<<<<< HEAD

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


=======
	
	munmap(example_struct, sizeof(*example_struct));
}


void manejador_SIGUSR2(int sig) {
	printf("Se ha recibido la señal SIGUSR2");
	fflush(stdout);
}

Mensaje ship_action(tipo_mapa mapa, int orix, int oriy, int range);



>>>>>>> a011b2dd2e09389daf3532665d892fa94cee3623
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
<<<<<<< HEAD

	pid_t pid_boss, pid_ship; //Para los procesos hijos

	/*Necesito una pipe para comunicarme con el jefe de cada equipo*/
	int pipe_simulador_jefe[N_EQUIPOS][2];
	int nbytes, pipe_status[N_EQUIPOS];
=======
    char string_ataque[] = "ATAQUE";


    char readbuffer[80];

    

	pid_t pid_boss, pid_ship; //Para los procesos hijos

	/*Necesito una pipe por jefe para para que el simulador envie mensajes a los jefes*/
	int pipe_simulador_jefe[N_EQUIPOS][2];
    /*Necesito una pipe por jefe para que le envien mensajes al simulador*/
    int pipe_jefe_simulador[N_NAVES][2];
	/*Cada jefe tiene una pipe por cada nave para comunicarse con ellas*/
	int pipe_jefe_nave[N_EQUIPOS][N_NAVES][2];
	int nbytes, pipe_status
>>>>>>> a011b2dd2e09389daf3532665d892fa94cee3623

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
<<<<<<< HEAD
	int fd_shm = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_EXCL,S_IRUSR | S_IWUSR);
	if(fd_shm == -1){
		printf("[ERROR] No se ha creado correctamente la memoria compartida");
=======
    printf("Simulador: Gestionando la memoria compartida:\n\n");
	int fd_shm = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_EXCL,S_IRUSR | S_IWUSR);
	if(fd_shm == -1){
		printf("[ERROR] No se ha creado correctamente la memoria compartida\n");
>>>>>>> a011b2dd2e09389daf3532665d892fa94cee3623
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

<<<<<<< HEAD
	/*La memoria compartida ya esta creada*/
=======
	for(i = 0; i < N_EQUIPOS; i++){
		shared_memory->mensaje_simulador_jefe[i] = 0;
		shared_memory->mensaje_jefe_simulador[i] = 0;
	}

	



	/*La memoria compartida ya esta creada*/

	/*Creamos las pipes para la comunicacion entre en simulador y los jefes*/
	for(i=0 ; i<N_EQUIPOS; i++){
		pipe_status = pipe(pipe_simulador_jefe[i]);
		if(pipe_status == -1){
			printf("[ERROR] No se ha creado la tuberia para el equipo :%d\n",i);
			munmap(shared_memory, sizeof(*shared_memory));
			shm_unlink(SHM_NAME);
				
			exit(EXIT_FAILURE);
		}

        pipe_status = pipe(pipe_jefe_simulador[i]);
		if(pipe_status == -1){
			printf("[ERROR] No se ha creado la tuberia para el equipo :%d\n",i);
			munmap(shared_memory, sizeof(*shared_memory));
			shm_unlink(SHM_NAME);
				
			exit(EXIT_FAILURE);
		}
			
	}

	/*Creamos las pipes para la comunicacion entre los jefes y las naves*/
	for(i = 0; i < N_EQUIPOS; i++){
		for(j = 0; j<N_NAVES; j++){
			pipe_status = pipe(pipe_jefe_nave[i][j]);
			if(pipe_status == -1){
				printf("[ERROR] No se ha creado la tuberia para el el jefe %d y su nave %d\n",i,j);
				munmap(shared_memory, sizeof(*shared_memory));
				shm_unlink(SHM_NAME);

				exit(EXIT_FAILURE);
			}

		}
	}
	
>>>>>>> a011b2dd2e09389daf3532665d892fa94cee3623
	
	/********************************el mapa************************************/

	/*Inicializamos todas las casillas con vacio.*/
<<<<<<< HEAD
=======
    printf("Simulador: Inicializando el mapa...\n\n");

>>>>>>> a011b2dd2e09389daf3532665d892fa94cee3623
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
<<<<<<< HEAD
	
	/************************Creamos la cola de mensajes************************/
	msg_queue = mq_open(MQ_NAME,
				O_CREAT, O_RDWR,
=======
    /*Para ver que se han creado los boses*/
    /*Hacemos todo lo relativo a la alarma*/
	sigemptyset(&(act.sa_mask));
	act.sa_flags = 0;

    act.sa_handler = manejador_SIGUSR2;
    if (sigaction(SIGUSR2, &act, NULL) < 0) {
		munmap(shared_memory, sizeof(*shared_memory));
		shm_unlink(SHM_NAME);
        perror("sigaction");
        return EXIT_FAILURE;
    }
	
	/************************Creamos la cola de mensajes************************/
    printf("Simulador: Gestionando mp...\n\n");

	msg_queue = mq_open(MQ_NAME,
				O_CREAT,
                S_IRUSR | S_IWUSR,
>>>>>>> a011b2dd2e09389daf3532665d892fa94cee3623
				&attributes);
   
	if(msg_queue == (mqd_t)-1){
		printf("[ERROR] Opening the message queue");
<<<<<<< HEAD
		return -1;
	}
=======
		munmap(shared_memory, sizeof(*shared_memory));
		shm_unlink(SHM_NAME);
		return -1;
	}
    printf("Simulador: Creando a los procesos jefes\n\n");
>>>>>>> a011b2dd2e09389daf3532665d892fa94cee3623

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
<<<<<<< HEAD

		/***********Creamos los procesos nave***********/
			for(j = 0; j < N_NAVES; j++){
				pid_ship = fork();

=======
            
		/***********Creamos los procesos nave***********/
			for(j = 0; j < N_NAVES; j++){
				pid_ship = fork();
                
>>>>>>> a011b2dd2e09389daf3532665d892fa94cee3623
				if(pid_ship < 0){//Caso de error
					printf("[ERROR] ha fallado el fork para la nave %d"
					" en el equipo %d\n", j, i);
					munmap(shared_memory, sizeof(*shared_memory));
					shm_unlink(SHM_NAME);
					return -1;
				}
<<<<<<< HEAD
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
=======
				else if(pid_ship == 0){//Caso hijo (Nave)

				close(pipe_jefe_nave[i][j][1]);

                while(1){



                    //if(mq_receive(msg_queue, (char *)&action, sizeof(action), NULL) == -1){
                    //    printf("ERRORSITO\n");
                    //}
                    //printf("%s\n",action.action);
                    ///*La nave que es es su j y el equipo es el del proceso jefe (la i)*/
					//action = ship_action(shared_memory->mapa, shared_memory->nave[i][j].posx, shared_memory->nave[i][j].posy, ATAQUE_ALCANCE);
                    //printf("Accion que estoy enviando: %s y a la posicion %d %d\n\n",action.action,action.x, action.y);
					//if(mq_send(msg_queue, (char *)&action, sizeof(action), 1) == -1){
					//	printf("[ERROR] Enviando la accion al simulador "
					//	"desde la nave %d in team %d\n", j, i);
					//	return -1;
					//}
                }
                    
				}
				return 0;
			}
            
		

            /******************************************************************/
            
		}
		/*****************CONTINUACION PROCESO JEFE. YA ESTAN LAS NAVES************************/
		
		/*inicializamos la pipe con la que se comunicara con el jefe*/
		/* Cierre del descriptor de salida en el jefe */
        close(pipe_simulador_jefe[i][1]);
        /*Cierre del descriptor de entrada en esta*/
        close(pipe_jefe_simulador[i][0]);
		/*Cerramos las pipes para comunicarnos con cada nave*/
		for(j=0; j<N_NAVES; j++){
			/*aqui envia*/
			close(pipe_jefe_nave[i][j][0]);
		}

		kill(getppid(),SIGUSR2);
		
        
		/*bucle que se ejecuta hasta el final de la partida*/
		while(end_simulation == 0){
			if(shared_memory->mensaje_jefe_simulador[i] > 0){
				read(pipe_simulador_jefe[i][0], readbuffer, sizeof(readbuffer));
            	printf("[JEFE] %s\n",readbuffer);
				shared_memory->mensaje_jefe_simulador[i] --;
				if(strcmp("TURNO",readbuffer) == 0){
                	printf("[JEFE]: escribiendo... \n");
                	write(pipe_jefe_simulador[i][1],string_ataque, strlen(string_ataque));
					shared_memory->mensaje_simulador_jefe[i]++;
            	}
			return 0;
			}
            
            
		}
		/*Termina el proceso cuando termina la simulacion*/
		exit(EXIT_SUCCESS);
		}
        else{
            pause();
            printf(" del proceso jefe con pid: %d\n",pid_boss);
        }
>>>>>>> a011b2dd2e09389daf3532665d892fa94cee3623

	}

	/*Proceso simulador tras crear a los jefes*/

<<<<<<< HEAD
	/*Hacemos todo lo relativo a la alarma*/
	sigemptyset(&(act.sa_mask));
	act.sa_flags = 0;
=======
>>>>>>> a011b2dd2e09389daf3532665d892fa94cee3623
	/* Se arma la señal SIGALRM. */
	act.sa_handler = manejador_SIGALRM;
	if (sigaction(SIGALRM, &act, NULL) < 0) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}
<<<<<<< HEAD

	/*Montamos la alarma*/
	if (alarm(ALARM))
		fprintf(stderr, "Existe una alarma previa establecida\n");

	while(end_simulation == 0){
=======
	/*Montamos la alarma*/
	if (alarm(5))
		fprintf(stderr, "Existe una alarma previa establecida\n");

    printf("Simulador: NUEVO TURNO\n");
	for (i=0; i<N_EQUIPOS; i++){
        close(pipe_simulador_jefe[i][0]);
        close(pipe_jefe_simulador[i][1]);
		write(pipe_simulador_jefe[i][1], string_turno, strlen(string_turno));
		shared_memory->mensaje_jefe_simulador[i]++;
	}

	shared_memory->flag_alarm = 0;

	while(end_simulation == 0){
        printf("[SIMULADOR] Entrando en el bucle de la simulacion\n");
>>>>>>> a011b2dd2e09389daf3532665d892fa94cee3623
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
<<<<<<< HEAD
=======
                    printf("[SIMULADOR] FIN DE LA PARTIDA\n");
>>>>>>> a011b2dd2e09389daf3532665d892fa94cee3623
					write(pipe_simulador_jefe[i][1], string_fin, strlen(string_fin));
				}
			}
			/*Nuevo turno para todos los equipos*/
			else{
<<<<<<< HEAD
=======
                printf("Simulador: NUEVO TURNO\n");
>>>>>>> a011b2dd2e09389daf3532665d892fa94cee3623
				for (i=0; i<N_EQUIPOS; i++){
					write(pipe_simulador_jefe[i][1], string_turno, strlen(string_turno));
				}
			shared_memory->flag_alarm = 0;
			}
<<<<<<< HEAD
			
		}

=======

            
			
		}

            for(i=0; i < N_EQUIPOS; i++){
				if(shared_memory->mensaje_simulador_jefe[i] > 0){
					read(pipe_jefe_simulador[i][0],readbuffer,sizeof(readbuffer));
                	printf("[SIMULADOR] leido delreturn hijo %d: %s\n",i,readbuffer);
					shared_memory->mensaje_simulador_jefe[i]--;
				}
            }
            

>>>>>>> a011b2dd2e09389daf3532665d892fa94cee3623
	}



	/*Esperamos a los procesos hijos*/
	for(i=0; i<N_EQUIPOS; i++){
		wait(0);
	}

	/*Liberamos recursos*/
	munmap(shared_memory, sizeof(*shared_memory));
	shm_unlink(SHM_NAME);
<<<<<<< HEAD

=======
>>>>>>> a011b2dd2e09389daf3532665d892fa94cee3623
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
<<<<<<< HEAD
=======
}

Mensaje ship_action(tipo_mapa mapa, int orix, int oriy, int range){
	
	int i, x, y;
	Mensaje action;


	//Search for the nearest ship, in circles profressively larger around the ship
	for(i = 0; i < MAPA_MAXX; i++){//Choose the circle radius
		for(x = -i; x<=i; x++){
			for(y = -i; y<=i; y++){
				if(x == i || y == i || x == -i || y == -i){//Only those in the circunference
					if(mapa_is_casilla_vacia(&mapa, y, x) == false){
                        if(mapa_get_distancia(&mapa, oriy, orix, y, x) <= range){
							action.x = x;
							action.y = y;
                            strcpy(action.action, "ATAQUE");
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
							strcpy(action.action, "MOVER");
							return action;
						}
                    }
						
				}
			}
		}
	}
    return action;
>>>>>>> a011b2dd2e09389daf3532665d892fa94cee3623
}




