#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define P 4
#define K 2
#define W 3
#define S 3

#define PIZZAS_NEEDED 10

typedef struct {
    char plates[W][6];
    char store[S][6];

    int started;
    int produced;
    int delivered;

    pthread_mutex_t mutex;
    pthread_cond_t changed;
} Pizzeria;

Pizzeria pizzeria;

void next_step(char *pizza)
{
    if (strcmp(pizza, "") == 0)
        strcpy(pizza, "p");
    else if (strcmp(pizza, "p") == 0)
        strcpy(pizza, "pi");
    else if (strcmp(pizza, "pi") == 0)
        strcpy(pizza, "piz");
    else if (strcmp(pizza, "piz") == 0)
        strcpy(pizza, "pizz");
    else if (strcmp(pizza, "pizz") == 0)
        strcpy(pizza, "pizza");
}

int find_empty_store_place()
{
    for (int i = 0; i < S; ++i) {
        if (strcmp(pizzeria.store[i], "") == 0)
            return i;
    }

    return -1;
}

int find_ready_pizza_in_store()
{
    for (int i = 0; i < S; ++i) {
        if (strcmp(pizzeria.store[i], "pizza") == 0)
            return i;
    }

    return -1;
}

int find_ready_plate()
{
    for (int i = 0; i < W; ++i) {
        if (strcmp(pizzeria.plates[i], "pizza") == 0)
            return i;
    }

    return -1;
}

int find_unfinished_plate()
{
    for (int i = 0; i < W; ++i) {
        if (strcmp(pizzeria.plates[i], "") != 0 &&
            strcmp(pizzeria.plates[i], "pizza") != 0)
            return i;
    }

    return -1;
}

int find_empty_plate()
{
    for (int i = 0; i < W; ++i) {
        if (strcmp(pizzeria.plates[i], "") == 0)
            return i;
    }

    return -1;
}

void *cook(void *arg)
{
    int id = *(int *)arg;

    while (1) {
        pthread_mutex_lock(&pizzeria.mutex);

        if (pizzeria.delivered >= PIZZAS_NEEDED) {
            pthread_mutex_unlock(&pizzeria.mutex);
            break;
        }

        int worked = 0;

        int plate = find_ready_plate();
        int place = find_empty_store_place();

        if (plate != -1 && place != -1) {
            strcpy(pizzeria.store[place], "pizza");
            strcpy(pizzeria.plates[plate], "");

            pizzeria.produced++;

            printf("Cook %d put pizza into store\n", id);

            worked = 1;
        }

        if (!worked) {
            plate = find_unfinished_plate();

            if (plate != -1) {
                next_step(pizzeria.plates[plate]);

                printf("Cook %d: plate %d -> %s\n",
                       id, plate, pizzeria.plates[plate]);

                worked = 1;
            }
        }

        if (!worked && pizzeria.started < PIZZAS_NEEDED) {
            plate = find_empty_plate();

            if (plate != -1) {
                next_step(pizzeria.plates[plate]);
                pizzeria.started++;

                printf("Cook %d started pizza %d on plate %d\n",
                       id, pizzeria.started, plate);

                worked = 1;
            }
        }

        if (worked) {
            pthread_cond_broadcast(&pizzeria.changed);
        } else {
            pthread_cond_wait(&pizzeria.changed, &pizzeria.mutex);
        }

        pthread_mutex_unlock(&pizzeria.mutex);
    }

    printf("Cook %d went home\n", id);

    return NULL;
}

void *courier(void *arg)
{
    int id = *(int *)arg;

    while (1) {
        pthread_mutex_lock(&pizzeria.mutex);

        int place = find_ready_pizza_in_store();

        while (place == -1 && pizzeria.delivered < PIZZAS_NEEDED) {
            pthread_cond_wait(&pizzeria.changed, &pizzeria.mutex);
            place = find_ready_pizza_in_store();
        }

        if (pizzeria.delivered >= PIZZAS_NEEDED) {
            pthread_mutex_unlock(&pizzeria.mutex);
            break;
        }

        strcpy(pizzeria.store[place], "");
        pizzeria.delivered++;

        printf("Courier %d delivered pizza. Total: %d\n",
               id, pizzeria.delivered);

        pthread_cond_broadcast(&pizzeria.changed);

        pthread_mutex_unlock(&pizzeria.mutex);
    }

    printf("Courier %d went home\n", id);

    return NULL;
}

int main(void)
{
    pthread_t cooks[P];
    pthread_t couriers[K];

    int cook_ids[P];
    int courier_ids[K];

    for (int i = 0; i < W; ++i)
        strcpy(pizzeria.plates[i], "");

    for (int i = 0; i < S; ++i)
        strcpy(pizzeria.store[i], "");

    pizzeria.started = 0;
    pizzeria.produced = 0;
    pizzeria.delivered = 0;

    pthread_mutex_init(&pizzeria.mutex, NULL);
    pthread_cond_init(&pizzeria.changed, NULL);

    for (int i = 0; i < P; ++i) {
        cook_ids[i] = i;
        pthread_create(&cooks[i], NULL, cook, &cook_ids[i]);
    }

    for (int i = 0; i < K; ++i) {
        courier_ids[i] = i;
        pthread_create(&couriers[i], NULL, courier, &courier_ids[i]);
    }

    for (int i = 0; i < P; ++i)
        pthread_join(cooks[i], NULL);

    for (int i = 0; i < K; ++i)
        pthread_join(couriers[i], NULL);

    pthread_mutex_destroy(&pizzeria.mutex);
    pthread_cond_destroy(&pizzeria.changed);

    return 0;
}
