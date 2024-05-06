#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>

int getNumberCount(char fileName[], int x)
{
    FILE *file = fopen(fileName, "r");
    if (file == NULL)
    {
        return -1;
    }
    int count = 0;
    int number;
    while (fscanf(file, "%d", &number) == 1)
    {
        if (number == x)
        {
            count++;
        }
    }
    fclose(file);
    return count;
}

int main(int argc, char *argv[])
{
    int my_rank;
    int p;
    int tag = 0;
    int freq[100];
    int round = 1;
    int index1 = 0, index2 = 0;
    bool winner = false;

    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    char filename[100];
    int candidatesCount;
    int votersCount = 0;
    int myCount;

    if (my_rank == 0)
    {
        printf("Enter Filename:");
        fflush(stdout);
        scanf("%s", filename);
        FILE *fptr;
        fptr = fopen(filename, "r");
        char input[100];
        int lines = 1;
        while (fgets(input, 100, fptr) && lines <= 2)
        {
            if (lines == 1)
            {
                candidatesCount = atoi(input);
            }
            else
            {
                votersCount = atoi(input);
            }
            lines++;
        }
        myCount = votersCount / p;
        int i;
        for (i = 1; i < p; i++)
        {
            MPI_Send(&myCount, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
        }
        fclose(fptr);
    }

    MPI_Bcast(&filename, 100, MPI_CHAR, my_rank, MPI_COMM_WORLD);
    MPI_Bcast(&candidatesCount, 1, MPI_INT, my_rank, MPI_COMM_WORLD);
    for (; round <= 2; round++)
    {

        if (my_rank != 0)
        {
            MPI_Recv(&myCount, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
            if (round == 2)
            {
                MPI_Recv(&index1, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
                MPI_Recv(&index2, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
            }

            FILE *fptr;
            fptr = fopen(filename, "r");
            int votes[1000][100];
            char input[100];
            int lines = 1;
            int startPos = (my_rank * myCount) + 2;
            int i = 0;
            while (fgets(input, 100, fptr) && lines < startPos + myCount)
            {

                if (lines >= startPos)
                {
                    int j = 0, k = 0;
                    for (; j < candidatesCount; j++)
                    {
                        int l = 0;
                        char tmp[100];
                        while (k < strlen(input) && input[k] != ' ')
                        {
                            tmp[l] = input[k];
                            l++;
                            k++;
                        }
                        if (input[k] == ' ')
                            k++;

                        votes[i][j] = atoi(tmp);
                        // if(round == 2){
                        //     printf("%d \n" ,votes[i][j] );
                        // }
                    }
                    i++;
                }
                lines++;
            }
            int q = 0;
            for (; q < 100; q++)
            {
                freq[q] = 0;
            }

            int j = 0;
            for (; j <= i; j++)
            {
                if (round == 1)
                {
                    freq[votes[j][0]]++;
                }
                else
                {
                    printf("CC %d\n", candidatesCount);
                    int k = 0;
                    for (; k < candidatesCount; k++)
                    {

                        // printf("index1 :  %d - index2 : %d\n", index1, index2);
                        if (votes[j][k] == index1 || votes[j][k] == index2)
                        {
                            freq[votes[j][k]]++;
                            // printf("freq %d : %d round %d\n", votes[j][k], freq[votes[j][k]], round);
                            break;
                        }
                    }
                }
            }
            MPI_Send(&freq, 100, MPI_INT, 0, tag, MPI_COMM_WORLD);
        }
        else
        {
            int allFreq[100];
            int q = 0;
            for (; q < 100; q++)
            {
                allFreq[q] = 0;
            }

            int i = 1;
            for (; i < p; i++)
            {
                MPI_Recv(&freq, 100, MPI_INT, i, tag, MPI_COMM_WORLD, &status);
                int j = 0;
                for (; j < 100; j++)
                {
                    allFreq[j] += freq[j];
                }
            }

            int startPos = myCount * (p - 1) + 3;
            FILE *fptr;
            fptr = fopen(filename, "r");
            int votes[1000][100];
            char input[100];
            int lines = 1;
            while (fgets(input, 100, fptr))
            {

                if (lines >= startPos)
                {
                    int j = 0, k = 0;
                    for (; j < candidatesCount; j++)
                    {
                        int l = 0;
                        char tmp[100];
                        while (k < strlen(input) && input[k] != ' ')
                        {
                            tmp[l] = input[k];
                            l++;
                            k++;
                        }
                        if (input[k] == ' ')
                            k++;

                        votes[i][j] = atoi(tmp);
                    }
                    i++;
                }
                lines++;
            }
            fclose(fptr);

            int j = 0;
            for (; j <= i; j++)
            {
                if (round == 1)
                {
                    allFreq[votes[j][0]]++;
                }
                else
                {
                    int k = 0;
                    for (; k < candidatesCount; k++)
                    {
                        if (votes[j][k] == index1 || votes[j][k] == index2)
                        {
                            allFreq[votes[j][k]]++;
                            break;
                        }
                    }
                }
            }
            j = 1;
            printf("Round %d :\n", round);
            int winnerId, winnerPerc;
            for (; j <= candidatesCount; j++)
            {
                if (round == 2)
                {
                    printf("freq %d : %d", j, allFreq[j]);
                    if (j != index1 && j != index2)
                        continue;
                    // votersCount = 2;
                }

                double f = allFreq[j];

                double percent = (f / votersCount) * 100;
                if (percent > 50)
                {
                    winnerId = j;
                    winner = true;
                    winnerPerc = percent;
                }
                // printf("Candidate %d : %d\n" , j , allFreq[j]);
                printf("Candidate %d : %f \n", j, percent);
            }
            if (winner)
            {
                printf("Candidate %d with percentage %d won in round %d\n", winnerId, winnerPerc, round);
                break;
            }
            else
            {
                int mx1 = 0, mx2 = 0, j = 1;
                for (; j <= candidatesCount; j++)
                {
                    if (allFreq[j] >= mx1)
                    {
                        mx1 = allFreq[j];
                        index1 = j;
                    }
                }
                j = 1;
                for (; j <= candidatesCount; j++)
                {
                    if (allFreq[j] >= mx2 && j != index1)
                    {
                        mx2 = allFreq[j];
                        index2 = j;
                    }
                }
                int i;
                printf("my count %d\n", myCount);
                for (i = 1; i < p; i++)
                {
                    MPI_Send(&myCount, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
                    MPI_Send(&index1, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
                    MPI_Send(&index2, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
                }
                // printf("%d indx1 - %d indx2 \n",index1,index2);
                //  MPI_Send(&round, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
            }
        }
    }
    /* Shutdown MPI */
    MPI_Finalize();
    return 0;
}
