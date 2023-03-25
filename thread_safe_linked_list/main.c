#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linked_list.c"

LinkedList_t list;

struct worker_args
{
    int id;
    int num_iterations;
};

char *randomString(int len)
{
    char *str = malloc(sizeof(char) * (len + 1));
    for (int i = 0; i < len; i++)
        str[i] = rand() % 26 + 'a';
    str[len] = '\0';
    return str;
}

void *worker(void *arg)
{
    struct worker_args *args = (struct worker_args *)arg;
    int id = args->id;
    int num_iterations = args->num_iterations;
    for (int i = 0; i < num_iterations; i++)
    {
        char *value = randomString(14);
        Element_t *element = malloc(sizeof(Element_t));
        element->value = value;
        insert(&list, element);
        int size = get_length(&list);
        while (!delete (lookup(&list, value)))
            ;
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    int num_threads = 0;
    int num_iterations = 0;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--num_threads") == 0)
            num_threads = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "--num_iterations") == 0)
            num_iterations = atoi(argv[i + 1]);
    }

    printf("num_threads: %d\n", num_threads);
    printf("num_iterations: %d\n", num_iterations);

    list.value = NULL;
    list.next = &list;
    list.prev = &list;

    pthread_t *threads;
    threads = malloc(sizeof(pthread_t) * num_threads);
    for (int i = 0; i < num_threads; i++)
    {
        struct worker_args *args = malloc(sizeof(struct worker_args));
        args->id = i;
        args->num_iterations = num_iterations;
        pthread_create(&threads[i], NULL, worker, (void *)args);
    }
    for (int i = 0; i < num_threads; i++)
        pthread_join(threads[i], NULL);
    return 0;
}