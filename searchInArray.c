#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

int main()
{
    int rows, cols, key, i, j;
    printf("Enter number of rows: ");
    scanf("%d", &rows);
    printf("Enter number of columns: ");
    scanf("%d", &cols);
    printf("Enter the key to search: ");
    scanf("%d", &key);
    int **matrix = (int **)malloc(rows * sizeof(int *));
    int **indices = (int **)malloc(rows * cols * sizeof(int *));

    // intialization
    for (i = 0; i < rows; i++)
    {
        matrix[i] = (int *)malloc(cols * sizeof(int));
    }
    for (i = 0; i < rows * cols; i++)
    {
        indices[i] = (int *)malloc(2 * sizeof(int));
    }
    srand(time(NULL));
    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < cols; j++)
        {
            matrix[i][j] = rand()%100;
        }
    }

    int count = 0;

#pragma omp parallel for collapse(2) shared(matrix, rows, cols, key, indices, count) private(i, j)
    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < cols; j++)
        {
            if (matrix[i][j] == key)
            {
                indices[count][0] = i;
                indices[count][1] = j;
                count++;
                printf("Thread %d found key at index (%d, %d)\n", omp_get_thread_num(), i, j);
            }
        }
    }
    for (int i = 0; i < rows; i++)
    {
        free(matrix[i]);
    }
    free(matrix);

    if (count == 0)
    {
        printf("-1 \n");
        return 0;
    }

    printf("Indices where the key %d is found:\n", key);
    for (int i = 0; i < count; i++)
    {
        printf("(%d, %d)\n", indices[i][0], indices[i][1]);
    }

    for (int i = 0; i < count; i++)
    {
        free(indices[i]);
    }
    free(indices);

    return 0;
}
