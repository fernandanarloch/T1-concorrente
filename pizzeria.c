#include "pizzeria.h"
#include "queue.h"
#include "helper.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>

int pizzaria_aberta = 1; 
int mesas_disponiveis, pizzaiolos_finalizados = 0;
int num_pizzaiolos_g, num_mesas_g;
sem_t garcons_disponiveis, espaco_forno, pizzaria_vazia, pizzaiolos_encerrados;
pthread_mutex_t mutex_mesas, mutex_pa, mutex_balcao, mutex_pizzaiolos;
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
    pthread_mutex_init(&mutex_pa, NULL);
    pthread_mutex_init(&mutex_balcao, NULL);
    pthread_mutex_init(&mutex_pizzaiolos, NULL);
    
    sem_init(&espaco_forno, 0, tam_forno);
    sem_init(&pizzaiolos_encerrados, 0, 0);
    sem_init(&pizzaria_vazia, 0, 0);

    pthread_t t_pizzaiolos[n_pizzaiolos];
    for (int i = 0; i < n_pizzaiolos; i++) {
    	pthread_create(&(t_pizzaiolos[i]), NULL, producao_pizza, NULL);
    }

    // garcom_tchau
    num_mesas_g = n_mesas;

    //pizzeria_destroy
    num_pizzaiolos_g = n_pizzaiolos;

}

void pizzeria_close() {
    pizzaria_aberta = 0;

    sem_wait(&pizzaria_vazia);
	
	for (int i = 0; i < num_pizzaiolos_g; i++) {
		queue_push_back(&fila_pedidos, NULL);
	}

	sem_wait(&pizzaiolos_encerrados); 
}

void pizzeria_destroy() {

	// pegar_mesas
    pthread_mutex_destroy(&mutex_mesas);

	// garcom_chamar
	sem_destroy(&garcons_disponiveis);

    // fazer_pedido
    queue_destroy(&fila_pedidos);

    // producao_pizza
    pthread_mutex_destroy(&mutex_pa);
    pthread_mutex_destroy(&mutex_balcao);
    pthread_mutex_destroy(&mutex_pizzaiolos);
    sem_destroy(&espaco_forno);
    sem_destroy(&pizzaiolos_encerrados);

    // pizzeria_close
    sem_destroy(&pizzaria_vazia);
    sem_destroy(&pizzaiolos_encerrados);
}

void *garcom_leva_pizza(void* arg) {
	garcom_entregar((pizza_t*)arg);
	sem_post(&garcons_disponiveis);

	pthread_exit(NULL);
}

void *producao_pizza(void* arg) {
	
	while (1) {
		pedido_t* pedido = queue_wait(&fila_pedidos);

		if (pedido == NULL)
			break;

	   	pizza_t* pizza = pizzaiolo_montar_pizza(pedido);
	   	sem_init(&pizza->sem, 0, 0);

	   	sem_wait(&espaco_forno);
	   	pthread_mutex_lock(&mutex_pa);
	   	pizzaiolo_colocar_forno(pizza);
	   	pthread_mutex_unlock(&mutex_pa); 

	   	// verifica se pizza está pronta
	   	sem_wait(&pizza->sem);
	   	sem_destroy(&pizza->sem);
	   	
	   	pthread_mutex_lock(&mutex_pa);
		pizzaiolo_retirar_forno(pizza);
		pthread_mutex_unlock(&mutex_pa);
		sem_post(&espaco_forno);

		pthread_mutex_lock(&mutex_balcao);
		garcom_chamar();
		pthread_mutex_unlock(&mutex_balcao);

		pthread_t garcom;
		pthread_create(&garcom, NULL, garcom_leva_pizza, pizza);
	}
	
	pthread_mutex_lock(&mutex_pizzaiolos);
	pizzaiolos_finalizados++;
	if (pizzaiolos_finalizados == num_pizzaiolos_g){
		pthread_mutex_unlock(&mutex_pizzaiolos);
		sem_post(&pizzaiolos_encerrados);
	}

	pthread_mutex_unlock(&mutex_pizzaiolos);

	pthread_exit(NULL);
}

void pizza_assada(pizza_t* pizza) {
	sem_post(&pizza->sem);
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
	if (!pizzaria_aberta && mesas_disponiveis == num_mesas_g)
		sem_post(&pizzaria_vazia);
	pthread_mutex_unlock(&mutex_mesas);

    sem_post(&garcons_disponiveis);
}

void garcom_chamar() {
	sem_wait(&garcons_disponiveis);
}

void fazer_pedido(pedido_t* pedido) {
    queue_push_back(&fila_pedidos, pedido);
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
