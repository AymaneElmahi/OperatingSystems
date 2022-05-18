// Fichier lib.h à rédiger
#pragma once
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <pthread.h>
#include <stdnoreturn.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>    /* For O_* constants */
#include <sys/stat.h> /* For mode constants */

#define CHK(op)            \
    do                     \
    {                      \
        if ((op) == -1)    \
            raler(1, #op); \
    } while (0)
#define NCHK(op)           \
    do                     \
    {                      \
        if ((op) == NULL)  \
            raler(1, #op); \
    } while (0)
#define TCHK(op)                \
    do                          \
    {                           \
        if ((errno = (op)) > 0) \
            raler(1, #op);      \
    } while (0)
noreturn void raler(int syserr, const char *msg, ...);

/**
 * @brief structure to store a product
 */
struct produit
{
    char nom[20];
    int quantite;
    int clients_waiting;
};

/**
 * @brief check if a file is empty
 *
 * @param fd  file descriptor
 * @return int  1 if empty, 0 if not
 */
int is_empty(int fd);

/**
 * @brief Set the semaphore to a signal that we added a product
 *
 * @param sem
 * @param name
 * @param value
 * @return sem_t*
 */
sem_t *set_sem(sem_t **sem, char *name, int value);

/**
 * @brief Set the semaphore to protect the file
 *
 * @param sem
 * @param name
 * @param value
 * @return sem_t*
 */
sem_t *set_sem_file(sem_t **sem, char *name, int value);

/**
 * @brief  group products in the array
 *
 * @param products
 * @param argv
 * @param nb
 * @return int
 */
int group_products(struct produit *products, char *argv[], int nb);

/**
 * @brief  check if a file exists
 *
 * @param file
 * @return int
 */
int file_exists(char *file);
