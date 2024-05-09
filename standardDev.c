#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <time.h>

double globalMean, globalSum, standardDeviation;

int main()
{
    int number;
    printf("Please enter a number:\n");
    scanf("%d", &number);

    int numberOfProcess = omp_get_max_threads();
    int totalNumOfProcess = number * numberOfProcess;
    int i;

    // Generate n random elements
    int nGeneratedNumbers[number];
    double localSum = 0.0;
    double localSquaredDifference = 0.0;
    double localMean = 0.0;

    time_t t;
    // srand((unsigned)time(&t));
    srand(time(NULL));


    #pragma omp parallel shared(number, globalSum) private(i, nGeneratedNumbers, localSum, localMean, localSquaredDifference)
    {
        for(i = 0;i < number;i++){
            nGeneratedNumbers[i] = rand() % 100;
            localSum += nGeneratedNumbers[i];
        }
        for(i = 0;i < number;i++){
            printf("Thread %d: %d \n", omp_get_thread_num() , nGeneratedNumbers[i]);
        }
        
        // Calculate local mean of the generated n elements
        double localMean = localSum / number;
        // Calculate local sum of squared differences from the mean
        for (i = 0; i < number; i++)
        {
            localSquaredDifference += pow((nGeneratedNumbers[i] - localMean),2.0);
            // printf("Thread %d: localsquDiff: %f\n",omp_get_thread_num(), localSquaredDifference);
        }
        // Add local sum of squared differences to global sum
        #pragma omp atomic
            globalSum += localSquaredDifference;
        printf("Thread %d: localsum: %f localMean: %f localSquaredDiff: %f globalSum: %f\n", omp_get_thread_num() , localSum,localMean,localSquaredDifference, globalSum);
    }
    // Calculate the global mean
    double globalMean = globalSum / totalNumOfProcess;

    // Calculate the square root of the global mean
    standardDeviation = sqrt(globalMean);

    printf("\nThe standard deviation is %.2f\n", standardDeviation);

    return 0;
}
