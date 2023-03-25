// Operating Systems - HW4 - PQ1
// Aryan Ahadinia, 98103878

#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int INF_VAL = 2147483647;
char *INF_SYMBOL = "∞";

pthread_barrier_t barrier;

struct thread_arg
{
    int **matrix_state;
    int **matrix_init;
    int n;
    int i;
    int j;
    int result;
};

int minimum(int a, int b)
{
    if (a < b)
        return a;
    else
        return b;
}

void calculate(struct thread_arg *targ)
{
    int **matrix_init = targ->matrix_init;
    int **matrix_state = targ->matrix_state;
    int n = targ->n;
    int i = targ->i;
    int j = targ->j;
    int min_val = matrix_state[i][j];
    for (size_t k = 0; k < n; k++)
        if (matrix_state[i][k] != INF_VAL && matrix_init[k][j] != INF_VAL)
            min_val = minimum(min_val, matrix_state[i][k] + matrix_init[k][j]);
    targ->result = min_val;
}

void *thread_worker(void *arg)
{
    struct thread_arg *targ = (struct thread_arg *)arg;
    for (size_t i = 0; i < targ->n; i++)
    {
        calculate(targ);
        targ->matrix_state[targ->i][targ->j] = targ->result;
        pthread_barrier_wait(&barrier);
    }
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        printf("Invalid number of arguments.\n");
        printf("Usage: ./main <num_nodes> <in_path> <out_path>\n");
        return -1;
    }

    int n = atoi(argv[1]);
    char *in_path = argv[2];
    char *out_path = argv[3];

    int **matrix;
    matrix = (int **)malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++)
        matrix[i] = (int *)malloc(n * sizeof(int));

    FILE *file_in_pointer;
    file_in_pointer = fopen(in_path, "r");
    if (NULL == file_in_pointer)
    {
        printf("Cannot open file.\n");
        return -1;
    }
    char read_char_buffer[32];
    int read_num_buffer;
    for (size_t i = 0; i < n; i++)
    {
        for (size_t j = 0; j < n; j++)
        {
            fscanf(file_in_pointer, "%s\n", read_char_buffer);
            if (strtol(read_char_buffer, NULL, 10) != 0 || strcmp(read_char_buffer, "0") == 0)
                matrix[i][j] = strtol(read_char_buffer, NULL, 10);
            else
                matrix[i][j] = INF_VAL;
            if (i == j)
                matrix[i][j] = 0;
        }
    }
    fclose(file_in_pointer);

    int **matrix_init;
    matrix_init = (int **)malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++)
        matrix_init[i] = (int *)malloc(n * sizeof(int));
    for (size_t i = 0; i < n; i++)
        for (size_t j = 0; j < n; j++)
            matrix_init[i][j] = matrix[i][j];

    struct thread_arg **targ_matrix;
    targ_matrix = (struct thread_arg **)malloc(n * sizeof(struct thread_arg *));
    for (int i = 0; i < n; i++)
        targ_matrix[i] = (struct thread_arg *)malloc(n * sizeof(struct thread_arg));

    for (size_t i = 0; i < n; i++)
    {
        for (size_t j = 0; j < n; j++)
        {
            targ_matrix[i][j].matrix_state = matrix;
            targ_matrix[i][j].matrix_init = matrix_init;
            targ_matrix[i][j].n = n;
            targ_matrix[i][j].i = i;
            targ_matrix[i][j].j = j;
        }
    }

    pthread_barrier_init(&barrier, NULL, n * n + 1);

    pthread_t **threads;
    threads = (pthread_t **)malloc(n * sizeof(pthread_t *));
    for (int i = 0; i < n; i++)
        threads[i] = (pthread_t *)malloc(n * sizeof(pthread_t));

    for (size_t i = 0; i < n; i++)
        for (size_t j = 0; j < n; j++)
            pthread_create(&threads[i][j], NULL, thread_worker, &targ_matrix[i][j]);

    for (size_t i = 0; i < n; i++)
        pthread_barrier_wait(&barrier);

    for (size_t i = 0; i < n; i++)
        matrix[i][i] = INF_VAL;

    printf("Outfile: %s\n", out_path);

    FILE *file_out_pointer;
    file_out_pointer = fopen(out_path, "w");
    if (NULL == file_out_pointer)
    {
        printf("Cannot open file.\n");
        return -1;
    }
    for (size_t i = 0; i < n; i++)
    {
        for (size_t j = 0; j < n; j++)
        {
            if (matrix[i][j] == INF_VAL)
            {
                fprintf(file_out_pointer, "%s ", INF_SYMBOL);
            }
            else
            {
                fprintf(file_out_pointer, "%d ", matrix[i][j]);
            }
            fflush(file_out_pointer);
        }
        fprintf(file_out_pointer, "\n");
    }
    fclose(file_out_pointer);

    for (size_t i = 0; i < n; i++)
    {
        for (size_t j = 0; j < n; j++)
        {
            if (matrix[i][j] == INF_VAL)
            {
                printf("%s ", INF_SYMBOL);
            }
            else
            {
                printf("%d ", matrix[i][j]);
            }
        }
        printf("\n");
    }

    return 0;
}