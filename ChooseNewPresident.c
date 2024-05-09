#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>

void getCounts(int *candidatesCount, int *votersCount, FILE *fptr)
{
    char input[100];
    int lines = 1;
    while (fgets(input, 100, fptr) && lines <= 2)
    {
        if (lines == 1)
        {
            *candidatesCount = atoi(input);
        }
        else
        {
            *votersCount = atoi(input);
        }
        lines++;
    }
}

void getMyVoters(int (*votes)[100], int candidatesCount, int startPos, int myCount, FILE *fptr)
{
    char input[100];
    int lines = 1;
    int i = 0;
    while (fgets(input, 100, fptr) && lines <= startPos + myCount)
    {
        if (lines > startPos)
        {
            int j = 0, k = 0;
            for (; j < candidatesCount; j++)
            {
                int l = 0;
                char tmp[100];
                while (k < strlen(input) && input[k] != ' ')
                {
                    tmp[l++] = input[k++];
                }
                k++;

                votes[i][j] = atoi(tmp);
            }
            i++;
        }
        lines++;
    }
}

void getRoundsVotes(int candidatesCount, int votes[][100], int freq[100], int round, int index1, int index2, int votersCount)
{
    int j = 0;
    for (j = 0; j < votersCount; j++)
    {
        if (round == 1)
        {
            freq[votes[j][0]]++;
        }
        else
        {
            int k = 0;
            for (; k < candidatesCount; k++)
            {
                if (votes[j][k] == index1 || votes[j][k] == index2)
                {
                    freq[votes[j][k]]++;
                    break;
                }
            }
        }
    }
}

void getMax2(int candidatesCount, int allFreq[100], int *index1, int *index2)
{
    int mx1 = 0, mx2 = 0, j = 1;
    for (; j <= candidatesCount; j++)
    {
        if (allFreq[j] >= mx1)
        {
            mx1 = allFreq[j];
            *index1 = j;
        }
    }
    j = 1;
    for (; j <= candidatesCount; j++)
    {
        if (allFreq[j] >= mx2 && j != *index1)
        {
            mx2 = allFreq[j];
            *index2 = j;
        }
    }
}

void print(int candidatesCount, int votersCount, int allFreq[100], int index1, int index2, int round, int *winnerId, bool *winner, int *winnerPerc)
{
    int totalVotes = 0;
    for (int j = 1; j <= candidatesCount; j++)
    {
        totalVotes += allFreq[j];
    }

    int j = 1;
    for (; j <= candidatesCount; j++)
    {
        if (round == 2)
        {
            if (j != index1 && j != index2)
                continue;
        }

        double f = allFreq[j];
        double percent = (f / totalVotes) * 100;

        if (percent > 50)
        {
            *winnerId = j;
            *winner = true;
            *winnerPerc = percent;
        }
        printf("Candidate %d : %f \n", j, percent);
    }
}
void getInput(char filename[100])
{
    int choice;
    printf("If you want to enter the input from the console enter 1 otherwise enter 2 : \n");
    scanf("%d", &choice);
    if (choice == 1)
    {
        int candidatesCount, votersCount;
        printf("Enter the candidtes count : \n");
        scanf("%d", &candidatesCount);
        printf("Enter the voters count : \n");
        scanf("%d", &votersCount);
        FILE *fptr;
        fptr = fopen("myfile.txt", "w");
        fprintf(fptr, "%d\n%d\n", candidatesCount, votersCount);
        printf("Enter the votes for each voter (separated by spaces):\n");
        for (int i = 0; i < votersCount; i++)
        {
            for (int j = 0; j < candidatesCount; j++)
            {
                int vote;
                scanf("%d", &vote);
                fprintf(fptr, "%d ", vote);
            }
            fprintf(fptr, "\n");
        }
        fclose(fptr);
        strcpy(filename,"myfile.txt");
    }
    else
    {
        printf("Enter Filename:");
        fflush(stdout);
        scanf("%s", filename);
    }
}
int main(int argc, char *argv[])
{
    int my_rank, p, tag = 0;
    int round = 1, index1 = 0, index2 = 0;
    bool winner = false, winner2 = false;
    char filename[100];
    int candidatesCount, votersCount = 0, myCount, end;

    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    if (my_rank == 0)
    {
        getInput(filename);
        FILE *fptr;
        fptr = fopen(filename, "r");
        getCounts(&candidatesCount, &votersCount, fptr);
        myCount = votersCount / p;
        for (int i = 1; i < p; i++)
        {
            MPI_Send(&myCount, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
        }
        fclose(fptr);
    }

    MPI_Bcast(&filename, 100, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&candidatesCount, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&votersCount, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (my_rank != 0)
    {
        MPI_Recv(&myCount, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
        FILE *fptr;
        fptr = fopen(filename, "r");
        int votes[1000][100];
        int startPos = ((my_rank - 1) * myCount) + 2;
        getMyVoters(votes, candidatesCount, startPos, myCount, fptr);
        int freq[100] = {0};
        getRoundsVotes(candidatesCount, votes, freq, 1, index1, index2, votersCount);
        MPI_Send(&freq, 100, MPI_INT, 0, tag, MPI_COMM_WORLD);
        fclose(fptr);
    }
    else
    {
        int allFreq[100] = {0};
        int freq[100] = {0};

        for (int i = 1; i < p; i++)
        {
            MPI_Recv(&freq, 100, MPI_INT, i, tag, MPI_COMM_WORLD, &status);
            for (int j = 1; j <= candidatesCount; j++)
            {
                allFreq[j] += freq[j];
            }
        }

        int startPos = myCount * (p - 1) + 2;
        FILE *fptr;
        fptr = fopen(filename, "r");
        int votes[1000][100];
        getMyVoters(votes, candidatesCount, startPos, 1000, fptr);
        getRoundsVotes(candidatesCount, votes, allFreq, 1, index1, index2, votersCount);

        printf("Round %d :\n", 1);
        int winnerId, winnerPerc;
        print(candidatesCount, votersCount, allFreq, index1, index2, 1, &winnerId, &winner, &winnerPerc);
        if (!winner)
        {
            round = 2;
            getMax2(candidatesCount, allFreq, &index1, &index2);
            int i;
            for (i = 1; i < p; i++)
            {
                MPI_Send(&index1, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
                MPI_Send(&index2, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
            }
        }
        else
        {
            printf("Candidate %d with percentage %d won in round %d\n", winnerId, winnerPerc, 1);
        }
        fclose(fptr);
    }

    MPI_Allreduce(&round, &end, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
    if (end == 2)
    {
        if (my_rank != 0)
        {
            MPI_Recv(&index1, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
            MPI_Recv(&index2, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);

            FILE *fptr;
            fptr = fopen(filename, "r");
            int votes2[1000][100];
            int startPos = ((my_rank - 1) * myCount) + 2;
            getMyVoters(votes2, candidatesCount, startPos, myCount, fptr);
            int freq2[100] = {0};
            getRoundsVotes(candidatesCount, votes2, freq2, 2, index1, index2, votersCount);
            MPI_Send(&freq2, 100, MPI_INT, 0, tag, MPI_COMM_WORLD);
            fclose(fptr);
        }
        else
        {
            int allFreq2[100] = {0};
            int freq2[100] = {0};
            for (int i = 1; i < p; i++)
            {
                MPI_Recv(&freq2, 100, MPI_INT, i, tag, MPI_COMM_WORLD, &status);
                for (int j = 1; j <= candidatesCount; j++)
                {
                    allFreq2[j] += freq2[j];
                }
            }

            int startPos = myCount * (p - 1) + 2;
            FILE *fptr;
            fptr = fopen(filename, "r");
            int votes2[1000][100];
            getMyVoters(votes2, candidatesCount, startPos, 1000, fptr);
            getRoundsVotes(candidatesCount, votes2, allFreq2, 2, index1, index2, votersCount);

            printf("Round %d :\n", round);
            int winnerId, winnerPerc;
            print(candidatesCount, votersCount, allFreq2, index1, index2, 2, &winnerId, &winner2, &winnerPerc);
            if (winner2)
            {
                printf("Candidate %d with percentage %d won in round %d\n", winnerId, winnerPerc, 2);
            }
            fclose(fptr);
        }
    }

    MPI_Finalize();
    return 0;
}

// tested Cases //
// 6
// 8
// 1 3 4 2 5 6
// 2 6 3 4 5 1
// 3 1 2 5 4 6
// 4 5 2 3 1 6
// 1 3 4 2 5 6
// 6 5 3 4 2 1
// 3 1 2 5 4 6
// 4 5 2 3 1 6
///////////////
// 3
// 5
// 1 2 3
// 1 2 3
// 2 1 3
// 2 3 1
// 3 2 1
//////////////
// 2
// 3
// 2 1
// 1 2
// 2 1
