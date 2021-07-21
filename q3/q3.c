#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <wait.h>

#define RED "\x1b[1;31m"
#define GREEN "\x1b[1;32m"
#define YELLOW "\x1b[1;33m"
#define BLUE "\x1b[1;34m"
#define MAGENTA "\x1b[1;35m"
#define CYAN "\x1b[1;36m"
#define RESET "\x1b[0m"

char name[1000][30];
char instrmnt[1000];
int arriv_time[1000];
int k, a, e, c, t1, t2, t;

pthread_mutex_t onstage[10000];
pthread_mutex_t acho_occupied, singer;
pthread_mutex_t elec_occupied, non_singer;
sem_t acho;
sem_t elec;
sem_t t_shirts;
int acho_on;
int elec_on;

typedef struct s
{
    int id;
    int arrival;
    int stage_took;
    int increment_time;
    int on_which_acho_stage;
    int on_which_elec_stage;
    int acho_added_id;
    int elec_added_id;

} s;
int acho_time_inc[1000];
int elec_time_inc[1000];
int acho_added_id[1000];
int elec_added_id[1000];
s *input[10000];
int acho_stage[1000]; //is zero if empty or 1 if occupied by other than singer  or 2 if occupied by singer 0r 3 if occupied by singer and muscian
int elec_stage[1000]; //is zero if empty or 1 if occupied by other than singer  or 2 if occupied by singer 0r 3 if occupied by singer and muscian
int acho_stage_id[1000];
int elec_stage_id[1000];
void *acho_func(void *inp)
{
    s *inputs = (s *)inp;
    sleep(inputs->arrival);
    printf(GREEN);
    if (instrmnt[inputs->id] != 'b')
        printf("%s, %c, arrived\n", name[inputs->id], instrmnt[inputs->id]);
    printf(RESET);
    int start = 0;
    int end = t1 + (rand() % (t2 - t1+1));
    if (instrmnt[inputs->id] == 's')
    {    
        for (int i = 1; i <= a; i++)
        {
            if (acho_stage[i] == 1)
            {
                pthread_mutex_lock(&onstage[inputs->id]);
                usleep(0.1);
                inputs->stage_took++;
                pthread_mutex_unlock(&onstage[inputs->id]);
                if (inputs->stage_took == 2)
                {
                    return NULL;
                }
                acho_stage[i] = 3;
                acho_added_id[i] = inputs->id;
                inputs->on_which_acho_stage = i;
                acho_time_inc[i] = 2;
            }
        }
        if (inputs->on_which_acho_stage == 0)
        {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts); 
            ts.tv_sec += t;

            int st;
        
            st = sem_timedwait(&elec, &ts);
            
            if(st==-1){
                printf("%s %c left beacause of impatinece\n",name[inputs->id],instrmnt[inputs->id]);
                return NULL;

            }
            int i;
            for (i = 1; i <= a; i++)
            {
                if (acho_stage[i] == 0)
                {
                    acho_stage[i] = 2;
                    break;
                }
            }
            printf("%s is performing %c at acoustic stage for %d duration\n", name[inputs->id], instrmnt[inputs->id], end - start);
            sleep(end - start);
            acho_stage[i] = 0;
            sem_post(&acho);
            printf("%s performance at acoustic stage ended\n", name[inputs->id]);
            return NULL;
        }
        printf("%s joined %s performance ,extended by two minutes\n", name[inputs->id], name[acho_stage_id[inputs->on_which_acho_stage]]);
        sleep(end - start);
    }
    else
    {

        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
            
        ts.tv_sec += t;

        int st;

        st = sem_timedwait(&acho, &ts);
        

        if (st == -1)
        {
            printf("%s %c left beacause of impatinece\n", name[inputs->id], instrmnt[inputs->id]);
            return NULL;
        }
        for (int i = 1; i <= a; i++)
        {
            if (acho_stage[i] == 0)
            {
                pthread_mutex_lock(&onstage[inputs->id]);
                usleep(0.1);
                inputs->stage_took++;
                pthread_mutex_unlock(&onstage[inputs->id]);
                if (inputs->stage_took == 2)
                {
                    return NULL;
                }

                inputs->on_which_acho_stage = i;
                acho_stage_id[i] = inputs->id;
                acho_stage[i] = 1;
                break;
            }
        }
        printf("%s is performing %c at acoustic stage for %d duration\n", name[inputs->id], instrmnt[inputs->id], end - start);
        sleep(end - start);
        sleep(acho_time_inc[inputs->on_which_acho_stage]);
        printf("%s performance at electric stage ended\n", name[inputs->id]);
        acho_stage[inputs->on_which_acho_stage] = 0;
        sem_post(&acho);
        //printf("%s performance at electric stage ended\n", name[inputs->id]);
        acho_stage[inputs->on_which_acho_stage] = 0;
        acho_time_inc[inputs->on_which_acho_stage] = 0;
        int id = acho_added_id[inputs->on_which_acho_stage];
        if (id != 0)
        {
            printf("%s performance at electric stage ended\n", name[id]);
        }

        sem_wait(&t_shirts);
        printf("%s collecting tshirt\n", name[inputs->id]);
        sleep(2);
        sem_post(&t_shirts);
    }
    return NULL;
}

void *elec_func(void *inp)
{
    s *inputs = (s *)inp;
    sleep(inputs->arrival);
    printf(GREEN);
    if (instrmnt[inputs->id] == 'b')
        printf("%s, %c, arrived\n", name[inputs->id], instrmnt[inputs->id]);
    printf(RESET);
    int start = 0;
    int end = t1 + (rand() % (t2 - t1 + 1));
    if (instrmnt[inputs->id] == 's')
    {
        for (int i = 1; i <= e; i++)
        {
            if (elec_stage[i] == 1)
            {
                pthread_mutex_lock(&onstage[inputs->id]);
                sleep(1);
                inputs->stage_took++;
                pthread_mutex_unlock(&onstage[inputs->id]);
                if (inputs->stage_took == 2)
                {
                    return NULL;
                }
                elec_stage[i] = 3;
                elec_added_id[i] = inputs->id;
                inputs->on_which_elec_stage = i;
                elec_time_inc[i] = 2;
            }
        }
        if (inputs->on_which_elec_stage == 0)
        {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
    
            ts.tv_sec += t;

            int st;

            st = sem_timedwait(&elec, &ts);
            

            if (st == -1)
            {
                printf("%s %c left beacause of impatinece\n", name[inputs->id], instrmnt[inputs->id]);
                return NULL;
            }
            int i;

            for (i = 1; i <= e; i++)
            {
                if (elec_stage[i] == 0)
                {
                    elec_stage[i] = 2;
                    break;
                }
            }
            printf("%s is performing %c at electice stage for %d duration\n", name[inputs->id], instrmnt[inputs->id], end - start);
            sleep(end - start);
            elec_stage[i] = 0;
            sem_post(&elec);
            printf("%s performance at electric stage ended\n", name[inputs->id]);
            return NULL;
        }
        printf("%s joined %s performance ,extended by two minutes\n", name[inputs->id], name[elec_stage_id[inputs->on_which_elec_stage]]);
        sleep(end - start);
    }
    else
    {

        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        
        ts.tv_sec += t;

        int st;

        st = sem_timedwait(&elec, &ts);


        if (st == -1)
        {
            printf("%s %c left beacause of impatinece\n", name[inputs->id], instrmnt[inputs->id]);
            return NULL;
        }
        for (int i = 1; i <= e; i++)
        {
            if (elec_stage[i] == 0)
            {
                pthread_mutex_lock(&onstage[inputs->id]);
                usleep(0.1);
                inputs->stage_took++;
                pthread_mutex_unlock(&onstage[inputs->id]);
                if (inputs->stage_took == 2)
                {
                    return NULL;
                }

                inputs->on_which_elec_stage = i;
                elec_stage_id[i] = inputs->id;
                elec_stage[i] = 1;
                break;
            }
        }

        printf("%s is performing %c at electice stage for %d duration\n", name[inputs->id], instrmnt[inputs->id], end - start);

        sleep(end - start);
        sem_post(&elec);
        sleep(elec_time_inc[inputs->on_which_elec_stage]);
        sem_post(&elec);
        printf("%s performance at electric stage eneded\n", name[inputs->id]);
            elec_stage[inputs->on_which_elec_stage] = 0;
        elec_time_inc[inputs->on_which_elec_stage] = 0;
        int id = elec_added_id[inputs->on_which_elec_stage];
        if (id != 0)
        {
            printf("%s performance at electric stage ended\n", name[id]);
        }

        sem_wait(&t_shirts);
        printf("%s collecting tshirt\n", name[inputs->id]);
        sleep(2);
        sem_post(&t_shirts);
    }

    return NULL;
}
int main()
{
    scanf("%d %d %d %d %d %d %d", &k, &a, &e, &c, &t1, &t2, &t);
    for (int i = 1; i <= k; i++)
    {
        scanf("%s%s%d", name[i], &instrmnt[i], &arriv_time[i]);
    }
    sem_init(&acho, 0, a);
    sem_init(&elec, 0, e);
    sem_init(&t_shirts, 0, c);


    pthread_t tid[k],tid1[k];
    for (int i = 1; i <= k; i++)
    {
        pthread_mutex_init(&onstage[i], NULL);
    }
    for (int i = 1; i <= c; i++)
    {
    }
    //printf( "iam here\n");
    for(int i = 1; i<=k;i++){
        input[i] = (s *)malloc(sizeof(s));
        input[i]->id = i;
        input[i]->arrival = arriv_time[i];
    }
    int ind=0;
    int ind1=0;
    for (int i = 1; i <= k; i++)
    {
        
        if (instrmnt[i] != 'b'){

            pthread_create(&tid[ind - 1], NULL, acho_func, (void *)(input[i]));
            ind++;
        }

        
    }
    for(int i = 1 ;i <=k;i++){
        if(instrmnt[i]!='v'){
            pthread_create(&tid1[ind1 - 1], NULL, elec_func, (void *)(input[i]));
            ind1++;

        }
    }
    
    for (int i = 0; i < ind; i++)
    {
        pthread_join(tid[i], NULL);
    }
    for(int i = 0;i < ind1;i++){
        pthread_join(tid1[i],NULL);
    }
}