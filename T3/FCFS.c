#include <stdio.h>
#include <stdlib.h>

void encontrar_tempo_espera(int processos[], int n,
                            int bt[], int wt[])
{
    wt[0] = 0;

    for(int i = 1; i < n; i++)
        wt[i] = bt[i-1] + wt[i-1];

}

void encontrar_tempo_de_resposta(int processos[], int n,
                            int bt[], int wt[], int tat[])
{
    for(int i = 0; i < n; i++)
        tat[i] = bt[i] + wt[i];
}

void encontrar_tempo_medio(int processos[], int n, int bt[])
{
    int wt[n], tat[n], total_wt = 0, total_tat = 0;

    encontrar_tempo_espera(processos, n, bt, wt);

    encontrar_tempo_de_resposta(processos, n, bt, wt, tat);

    printf("Processos    tempo de explosão  tempo de espera   tempo de resposta\n");

    for(int i = 0; i < n; i++)
    {
        total_wt = total_wt + wt[i];
        total_tat = total_tat + tat[i];
        printf("   %d ", (i+1));
        printf("        %d", bt[i] );
        printf("        %d",wt[i] );
        printf("        %d\n",tat[i] ); 
    }
    float s=(float)total_wt / (float)n;
    float t=(float)total_tat / (float)n;
    printf("tempo de espera médio = %f",s);
    printf("\n");
    printf(" tempo de resposta médio = %f ",t); 
}

