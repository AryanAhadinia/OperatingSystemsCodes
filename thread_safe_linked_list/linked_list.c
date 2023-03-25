#include <pthread.h>
#include <string.h>

#include "linked_list.h"

// GitHub Copilot helped me write this code.

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void insert(LinkedList_t *list, Element_t *element) {
    pthread_mutex_lock(&mutex);
    element->next = list;
    element->prev = list->prev;
    list->next->prev = element;
    list->next = element;
    pthread_mutex_unlock(&mutex);
    printf("thread %d inserted %s\n", pthread_self(), element->value);
}

int delete(Element_t *element) {
    if (element->prev == NULL || element->next == NULL) {
        return 0;
    }
    pthread_mutex_lock(&mutex);
    element->prev->next = element->next;
    element->next->prev = element->prev;
    element->prev = NULL;
    element->next = NULL;
    pthread_mutex_unlock(&mutex);
    printf("thread %d deleted %s\n", pthread_self(), element->value);
    return 1;
}

Element_t *lookup(LinkedList_t *list, const char *value) {
    pthread_mutex_lock(&mutex);
    Element_t *element = list;
    while ((element = element->next) != list) {
        if (strcmp(element->value, value) == 0) {
            pthread_mutex_unlock(&mutex);
            printf("thread %d found %s\n", pthread_self(), value);
            return element;
        }
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int get_length(LinkedList_t *list) {
    pthread_mutex_lock(&mutex);
    int length = 0;
    Element_t *element = list;
    while ((element = element->next) != list)
        length++;
    pthread_mutex_unlock(&mutex);
    printf("thread %d length is %d\n", pthread_self(), length);
    return length;
}
