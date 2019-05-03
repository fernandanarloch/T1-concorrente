#include "pizzeria.h"
#include "queue.h"
#include "helper.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>

bool pizzaria_aberta = true;
pthread_sem_t garcons_disponiveis;
pthread_mutex_t pegador, smart_deck;
queue_t *fila_pedidos;

void pizzeria_init(int tam_forno, int n_pizzaiolos, int n_mesas,
                   int n_garcons, int tam_deck, int n_grupos) {
    // pegar_mesas

    // garcom_chamar
    sem_init(&garcons_disponiveis, 0, n_garcons);
    
    // fazer_pedido
    queue_init(fila_pedidos, tam_deck);
    pthread_mutex_init(&smart_deck, NULL);

    // pizza_pegar_fatia
    pthread_mutex_init(&pegador, NULL);
}

void pizzeria_close() {
    pizzaria_aberta = false;
}

void pizzeria_destroy() {
    // fazer_pedido
    queue_destroy(fila_pedidos);
    pthread_mutex_destroy(&smart_deck);

    // pizza_pegar_fatia
    pthread_mutex_destroy(&pegador);
}

void pizza_assada(pizza_t* pizza) {
}

int pegar_mesas(int tam_grupo) {
    mesas_grupo = (tam_grupo + 3)/4;

    if (!pizzaria_aberta)
        return -1;
}

void garcom_tchau(int tam_grupo) {
}

void garcom_chamar() {
    sem_wait(&garcons_disponiveis);
    sem_post(&garcons_disponiveis);
}

void fazer_pedido(pedido_t* pedido) {
    pthread_mutex_lock(&smart_deck);
    queue_push_back(fila_pedidos, pedido);
    pthread_mutex_unlock(&smart_deck);
}

int pizza_pegar_fatia(pizza_t* pizza) {
    pthread_mutex_lock(&pegador);
    if (pizza->fatias > 0) {
        pizza->fatias--;
        pthread_mutex_unlock(&pegador);
        return 0;
    }
    pthread_mutex_unlock(&pegador);
    return -1;
}
