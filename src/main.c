#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "split.h"
#include "util.h"



#define MAX_PROCESOS 25
#define MAX_COLAS 10

int numProcesos=0;
int numColas=0;

typedef struct {
	char nombre[10];
	int llegada;
	int ejecucion;
	int espera;
	int finalizacion;
} Proceso;

typedef enum{RR,SJF}Estrategia;

typedef struct{
	Proceso procesos[MAX_PROCESOS];
	int Cquantum;
	Estrategia es;
}colaPrioridad;

void imprimirTablaResultados(Proceso procesos[], int n, int tiempoTotal) {
	int totalEspera = 0;
	
	// tiempo de espera
	for (int i = 0; i < n; i++) {
		totalEspera += procesos[i].espera;
	}
	
	printf("\nResultados de la simulación\n");
	printf("Colas de prioridad: 3\n");
	printf("Tiempo total de la simulación: %d unidades de tiempo\n", tiempoTotal);
	printf("Tiempo promedio de espera: %.2f unidades de tiempo\n", (float)totalEspera / n);
	printf("\n# Proceso | Nombre  | T. Llegada | Tamaño | T. Espera | T. Finalización\n");
	printf("---------------------------------------------------------------------\n");
	
	for (int i = 0; i < n; i++) {
		printf("%-9d | %-7s | %-10d | %-6d | %-8d | %-15d\n", 
			   i + 1, procesos[i].nombre, procesos[i].llegada, 
			   procesos[i].ejecucion, procesos[i].espera, procesos[i].finalizacion);
	}
}

void roundRobin(Proceso procesos[], int n, int QUANTUM) {
	int tiempo = 0, completados = 0;
	int tiempoRestante[MAX_PROCESOS];
	
	for (int i = 0; i < n; i++) {
		tiempoRestante[i] = procesos[i].ejecucion; 
		procesos[i].espera = 0;
	}
	
	while (completados < n) {
		int cambio = 0;  // Para saber si algún proceso ejecutó
		
		for (int i = 0; i < n; i++) {
			if (tiempoRestante[i] > 0 && procesos[i].llegada <= tiempo) {
				cambio = 1;
				
				int tiempoEjecutado = (tiempoRestante[i] > QUANTUM) ? QUANTUM : tiempoRestante[i];
				tiempo += tiempoEjecutado;
				tiempoRestante[i] -= tiempoEjecutado;
				
				// Si el proceso terminó
				if (tiempoRestante[i] == 0) {
					procesos[i].finalizacion = tiempo;
					procesos[i].espera = procesos[i].finalizacion - procesos[i].llegada - procesos[i].ejecucion;
					if (procesos[i].espera < 0) procesos[i].espera = 0;
					completados++;
				}
			}
		}
		
		if (!cambio) tiempo++;  // Si ningún proceso ejecutó, avanza el tiempo
	}
	
	imprimirTablaResultados(procesos, n, tiempo);
}
void sjf(Proceso procesos[], int n) {
	int tiempo = 0, completados = 0;
	int terminado[MAX_PROCESOS] = {0};
	
	while (completados < n) {
		int menorTiempo = 9999, index = -1;
		
		// Buscar el proceso con menor tiempo de ejecución que ya haya llegado
		for (int i = 0; i < n; i++) {
			if (!terminado[i] && procesos[i].llegada <= tiempo && procesos[i].ejecucion < menorTiempo) {
				menorTiempo = procesos[i].ejecucion;
				index = i;
			}
		}
		
		// Si encontramos un proceso, ejecutarlo
		if (index != -1) {
			tiempo += procesos[index].ejecucion;
			procesos[index].finalizacion = tiempo;
			procesos[index].espera = procesos[index].finalizacion - procesos[index].llegada - procesos[index].ejecucion;
			if (procesos[index].espera < 0) procesos[index].espera = 0;
			terminado[index] = 1;
			completados++;
		} else {
			tiempo++;  // Si no hay procesos disponibles, avanzar el tiempo
		}
	}
	
	imprimirTablaResultados(procesos, n, tiempo);
}

Proceso crearProceso(int lle,int eje){
	Proceso p;
	p.llegada=lle;
	p.ejecucion=eje;
	p.espera=0;
	p.finalizacion=0;
	return p;
}


int main(int argc, char *argv[]) {
	
	/*
	Proceso procesos[MAX_PROCESOS] = {
		{"P1", 2, 7},
	{"P2", 0, 4},
		{"P3", 4, 6},
	{"P4", 5, 5},
		{"P5", 3, 3}
	};
	*/
	Proceso procesos[MAX_PROCESOS];
	colaPrioridad colas[MAX_COLAS];
	
	/*Lecturta del archivo de configuracion */
	int q;
	int i;
	FILE *fd;
	split_list *t;
	char linea[80];
	char **args;
	int llegada;
	int ejecucion;
	int espera;
	int finalizacion;
	
	/*Nombre del archivo con parametrso de simulacion*/
	char *filename;
	filename="gantt";
	
	/* Si no ingresa ningun argumento finaliza el programa*/
	if (argc < 2 ){
		printf("Se debe ingresar archivo para la simulacion");
		exit(-1);
	}
	
	/*Abrir el archivo y finalizar programa si este no existe*/
	filename = argv[1];
	fd = fopen(filename, "r");
	if (!fd){
		printf("EL archivo ingresado no existe");
		exit(-1);
	}
	
	/*Mientras el archivo tenga contenido se seguira leyendo*/
	while(!feof(fd)){
		/*Leer una linea*/
		memset(linea,0,80);
		fgets(linea,80,fd);
		/*Si la linea leida esta en blanco o comienza con # se lee la siguiente linea*/
		if (strlen(linea) <= 1)
		{
			continue;
		}
		if (linea[0] == '#')
		{
			continue;
		}
		
		/*Convertir linea leida en minusculas*/
		lcase(linea);
		
		// Partir la linea en tokens
		t = split(linea,0);
		
		// Ignora lineas sin tokens
		if (t->count == 0)
		{
			free_split_list(t);
			continue;
		}
		/*procesar cada linea*/
		args = t->parts;
		if (equals(args[0], "define") && t->count >= 3){
			// Definir numero de colas
			if (equals(args[1], "queues"))
			{
				numColas = atoi(args[2]);
			}else if (equals(args[1], "scheduling") && t->count >= 4){
				/*Definir estrategia de las colas*/
				i= atoi(args[2])-1;
				if(equals(args[3],"rr")){
					colas[i].es=RR;
				}else{
					colas[i].es=SJF;
				}
			}else if (equals(args[1], "quantum") && t->count >= 4){
				/*Definir quantum de las colas*/
				i= atoi(args[2])-1;
				q= atoi(args[3]);
				colas[i].Cquantum=q;
			}
		}else if (equals(args[0], "process") && t->count >= 4){
			/*Crear los procesos*/
			numProcesos+=1;
			llegada=atoi(args[2]);
			ejecucion=atoi(args[3]);
			Proceso p= crearProceso(llegada,ejecucion);
			procesos[numProcesos-1]=p;
			strcpy(procesos[numProcesos-1].nombre,args[1]);
		}else if (equals(args[0], "start" )){
			printf("Comenzar simulacion");
			free_split_list(t);
			break;
		}
	}
	
	int y;
	for (i=0;i<numColas;i++) {
		for(y=0;y<numProcesos;y++){
	            colas[i].procesos[y]=procesos[y];
	        }
		if (colas[i].es==RR){
			roundRobin(colas[i].procesos, numProcesos, colas[i].Cquantum);
		}else{
			sjf(colas[i].procesos, numProcesos);
		}
	}	
	return 0;
}
