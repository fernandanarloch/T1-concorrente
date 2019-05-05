#include "pizzeria.h"
#include "queue.h"
#include "helper.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>

_Bool pizzaria_aberta = 1; //                                                                    Por que?
int mesas_disponiveis;
sem_t garcons_disponiveis, pizzaiolos_disponiveis, espaco_forno;
pthread_mutex_t mutex_mesas, pa_pizza, balcao;
queue_t fila_pedidos;


void pizzeria_init(int tam_forno, int n_pizzaiolos, int n_mesas,
                   int n_garcons, int tam_deck, int n_grupos) {
    // pegar_mesas
    mesas_disponiveis = n_mesas;
    pthread_mutex_init(&mutex_mesas, NULL);

    // garcom_chamar
    sem_init(&garcons_disponiveis, 0, n_garcons);
    
    // fazer_pedido
    queue_init(&fila_pedidos, tam_deck);

    // producao_pizza
    sem_init(&pizzaiolos_disponiveis, 0, n_pizzaiolos);
    sem_init(&espaco_forno, 0, tam_forno);
    pthread_mutex_init(&pa_pizza, NULL);
    pthread_mutex_init(&balcao, NULL);
}

void pizzeria_close() {
    pizzaria_aberta = 0;
}

void pizzeria_destroy() {
	// pegar_mesas
    pthread_mutex_destroy(&mutex_mesas);

	// garcom_chamar
	sem_destroy(&garcons_disponiveis);

    // fazer_pedido
    queue_destroy(&fila_pedidos);

    // producao_pizza
    sem_destroy(&pizzaiolos_disponiveis);
    sem_destroy(&espaco_forno);
    pthread_mutex_destroy(&pa_pizza);
    pthread_mutex_destroy(&balcao);
}

void pizza_assada(pizza_t* pizza) {
   	pthread_mutex_lock(&pa_pizza);
	pizzaiolo_retirar_forno(pizza);
	pthread_mutex_unlock(&pa_pizza);
	sem_post(&espaco_forno);

	pthread_mutex_lock(&balcao);

    sem_post(&pizzaiolos_disponiveis);
	
	sem_wait(&garcons_disponiveis);
	pthread_mutex_unlock(&balcao);
    garcom_entregar(pizza);
    sem_post(&garcons_disponiveis);
}

int pegar_mesas(int tam_grupo) {
    int mesas_grupo = (tam_grupo + 3)/4;

    while (1) {
		if (!pizzaria_aberta)
	    	return -1;

	    pthread_mutex_lock(&mutex_mesas);
	    if (mesas_grupo <= mesas_disponiveis)
	    	break;

	    pthread_mutex_unlock(&mutex_mesas);
	}
	mesas_disponiveis -= mesas_grupo;
	pthread_mutex_unlock(&mutex_mesas);
	return 0;   
}

void garcom_tchau(int tam_grupo) {
	int mesas_grupo = (tam_grupo + 3)/4;

	pthread_mutex_lock(&mutex_mesas);
	mesas_disponiveis += mesas_grupo;
	pthread_mutex_unlock(&mutex_mesas);
}

void garcom_chamar() {
    sem_wait(&garcons_disponiveis);
    sem_post(&garcons_disponiveis);
}

void fazer_pedido(pedido_t* pedido) {
    queue_push_back(&fila_pedidos, pedido);
    producao_pizza();
}

void producao_pizza() {
	sem_wait(&pizzaiolos_disponiveis);
   	pizza_t* pizza;
   	pizza = pizzaiolo_montar_pizza(queue_wait(&fila_pedidos));

   	sem_wait(&espaco_forno);
   	pthread_mutex_lock(&pa_pizza);
   	pizzaiolo_colocar_forno(pizza);
   	pthread_mutex_unlock(&pa_pizza);
}

int pizza_pegar_fatia(pizza_t* pizza) {
    pthread_mutex_lock(&pizza->pegador);
    if (pizza->fatias > 0) {
        pizza->fatias--;
        pthread_mutex_unlock(&pizza->pegador);
        return 0;
    }
    pthread_mutex_unlock(&pizza->pegador);
    return -1;
}
