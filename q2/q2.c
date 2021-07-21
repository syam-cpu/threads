#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define RED "\x1b[1;31m"
#define GREEN "\x1b[1;32m"
#define YELLOW "\x1b[1;33m"
#define BLUE "\x1b[1;34m"
#define MAGENTA "\x1b[1;35m"
#define CYAN "\x1b[1;36m"
#define RESET "\x1b[0m"

pthread_mutex_t color;


pthread_mutex_t eights[10000];  // for putting a lock on slots alloted
pthread_mutex_t remcomp[10000];   // for comapany data
pthread_mutex_t waiting_students; //for counting number of waiting students
pthread_mutex_t comp_students;    // number of students vaccinated and tested positive
typedef struct s
{
    int id;
    int count;              // for indicating student round
    float recent_vacc_prob; // recently vacinated vaccine probability
} s;
typedef struct zone
{
    
    int total; //for counting total number of vaccines
    int id;
    int eight; //number of slots alloted

} zone;
typedef struct company
{
    int id;
    float prob;
    int batches;//number of batches it  has currently
    int curbatch; // number of vaccines in currently prepared batches
} comp;
comp *input2[10000];
zone *input1[10000];
s *input[10000];
int n, m, o;  // number of comp,number of zones ,number of students
float prob[10000]; //probaility of vaccines
int waiting;// for counting number of waiting students
int completed; // for counting number of suucessfully or unsuccessfully completed their vaccination
int student_zone[10000][10000];    //student_zone [x][y]] is one  when a student x is alloted to vaccination zone  y 0 else
int remcheck[10000];               //for storing how many vaccines a company deleiverd - how many of them have got used
int zone_comp_count[10000][10000]; //for track of how many vaccines a zone has recieved from a company
void *pharmacomp(void *inp)
{   sleep(1);

    comp *inputs = (comp *)inp;
    int slp = 2 + (rand() % 3);
    while (1)
    {
        comp *inputs = (comp *)inp;
        int slp = 2 + (rand() % 3);
        sleep(slp);
        int batch;
        batch = 1 + (rand() % 4);

        printf(RED);
        printf("pharmacetical company %d is preparing %d batches of vaccine which have success probability %f\n\n", inputs->id, batch, prob[inputs->id]);
        printf(RESET);

        int preparing = 2 + rand() % 4;
        int cnt = 10 + (rand() % 11);
        sleep(preparing);
        printf(YELLOW);
        printf("pharmacetical company %d has prepared %d batches of vaccine which have success probability %f\n\n", inputs->id, batch, prob[inputs->id]);
        printf(RESET);
        pthread_mutex_lock(&remcomp[inputs->id]);
        inputs->curbatch = cnt;
        inputs->batches = 1 + (rand() % 4);
        remcheck[inputs->id] = cnt * (inputs->batches);
        pthread_mutex_unlock(&remcomp[inputs->id]);

        
        int ok = 1;

        while (remcheck[inputs->id] > 0 && (o - completed) > 0)
        {
        }
        if (remcheck[inputs->id] == 0)
        {
            printf(CYAN);
            printf("all the vaccines prepared by pharmacetical company %d are emptied resuming production now\n\n", inputs->id);
            printf(RESET);
        }
        if (o - completed <= 0)
            break;
    }
}
void *vaccizone(void *inp)
{ 
    while (o - completed > 0)
    {   sleep(1);
        zone *inputs = (zone *)inp;
        printf(CYAN);
       
        while (o - completed > 0 && inputs->total == 0)
        {
            for (int i = 1; i <= n; i++)
            {
                if (input2[i]->batches != 0)
                {
                
                    inputs->total = input2[i]->curbatch;
                    zone_comp_count[inputs->id][i] = input2[i]->curbatch; 
                    pthread_mutex_lock(&remcomp[i]);
                    input2[i]->batches--;

                    pthread_mutex_unlock(&remcomp[i]);
                    printf(GREEN);
                    printf("pharmacetical company %d is delivering a vaccine batch to vaccination zone %d which has success probabilty %f\n\n", i, inputs->id, input2[i]->prob);
                    printf(RESET);
                    break;
                }
            }
        }
        if (o - completed <= 0)
            return NULL;

        pthread_mutex_lock(&eights[inputs->id]);

        if (inputs->total <= 8)
        {

            inputs->eight = inputs->total;
        }
        else
        {
            inputs->eight = 8;
        }

        
        printf(CYAN);
        printf("vaccination zone %d is ready to vaccinate with %d slots\n\n", inputs->id, inputs->eight);
        printf(RESET);

        int temp = inputs->eight; // number of slots alloted

    
        pthread_mutex_unlock(&eights[inputs->id]);
        while (waiting != 0 && inputs->eight != 0)
        {
            
        }

        pthread_mutex_lock(&eights[inputs->id]);
        int temp2 = temp - inputs->eight; //number of vaccines willbe used in a vaccination phase
        inputs->total -= (temp2);
        inputs->eight = 0;
        pthread_mutex_unlock(&eights[inputs->id]);

        
        pthread_mutex_lock(&color);
        printf(RED);
        printf("vaccination zone %d entering vaccination phase\n\n", inputs->id);
        printf(RESET);
        pthread_mutex_unlock(&color);

        int ind = 1;

        
        for (int i = 1; i <= o; i++)
        {
            if (student_zone[i][inputs->id] == 1)
            {
                for (int j = ind; j <= n; j++)
                {
                    if (zone_comp_count[inputs->id][j] > 0)
                    {
                        pthread_mutex_lock(&remcomp[inputs->id]);
                        remcheck[inputs->id]--;
                        pthread_mutex_unlock(&remcomp[inputs->id]);
                        zone_comp_count[inputs->id][i]--;
                        pthread_mutex_lock(&color);
                        printf(GREEN);
                        printf("student %d on vaccination zone %d has been vaccinated which has success probaiblity %f\n\n", i, inputs->id, prob[j]);
                        printf(RESET);
                        input[i]->recent_vacc_prob = prob[j];
                        student_zone[i][inputs->id] = 0;
                        pthread_mutex_unlock(&color);
                        sleep(1);
                        ind = j;
                        continue;
                    }
                }
            }
        }
        if(inputs->total == 0){
            printf(RED);
            printf("vaccination zone %d has run out of vaccines\n",inputs->id);
            printf(RESET);
        }

        printf(RESET);
    
        sleep(10);
    }
}
void *student(void *inp)
{   
    sleep(1);
    s *inputs = (s *)inp;
    while (inputs->count <= 3)
    {
        int slp = rand() % 10;
        sleep(slp);

        printf(YELLOW);
        printf("Student %d has arrived for his %d round of vaccination\n\n", inputs->id, inputs->count);

        printf(CYAN);
        printf("student %d is waiting to be allocated a vaccination zone\n\n", inputs->id);
        printf(RESET);

        pthread_mutex_lock(&waiting_students);
        waiting++;
        pthread_mutex_unlock(&waiting_students);
        printf(RESET);

        sleep(1);

        int ok = 1;
        while (ok)
        {
            for (int i = 1; i <= m; i++)
            {   
                pthread_mutex_lock(&eights[i]);
                

                if (input1[i]->eight > 0)
                {
                

                    input1[i]->eight--;
                    pthread_mutex_lock(&color);
                    printf(MAGENTA);
                    printf("Student %d assigned a slot on vaccination zone %d and waiting to be vaccinated\n\n", inputs->id, i);
                    printf(RESET);
                    pthread_mutex_unlock(&color);
                    pthread_mutex_unlock(&eights[i]);
                    pthread_mutex_lock(&waiting_students);
                    waiting--;
                    pthread_mutex_unlock(&waiting_students);
                    student_zone[inputs->id][i] = 1;

                    
                    while (student_zone[inputs->id][i])
                    {
                    
                    }
                    ok = 0;
                    sleep(4);
                    float p = inputs->recent_vacc_prob;
                    int round = p * 100;
                    int decider = 1 + rand() % 100;
                    if (decider < round)
                    {
                        pthread_mutex_lock(&color);
                        printf(GREEN);
                        printf("student %d has tested positive for antibodies\n\n", inputs->id);
                        printf(RESET);
                        inputs->count = 5;
                        pthread_mutex_unlock(&color);
                        pthread_mutex_lock(&comp_students);
                        completed++;
                        pthread_mutex_unlock(&comp_students);
                    }
                    else
                    {
                        pthread_mutex_lock(&color);
                        printf(RED);
                        printf("student %d has tested negative for antibodies\n\n", inputs->id);
                        printf(RESET);
                        pthread_mutex_unlock(&color);
                        inputs->count += 1;
                    }
                    //pthread_mutex_unlock(&queue[i]);
                }
                else
                {
                    
                    pthread_mutex_unlock(&eights[i]);
                }
            }
        }
    }
    if (inputs->count == 4)
    {
        pthread_mutex_lock(&comp_students);
        completed++;
        pthread_mutex_unlock(&comp_students);
    }
    printf("iam %d iam out\n", inputs->id);
    return NULL;
}

int main()
{
    scanf("%d", &n);
    scanf("%d", &m);
    scanf("%d", &o);

    int i;
    for (i = 1; i <= n; i++)
    {
        scanf("%f", &prob[i]);
    }

    pthread_mutex_init(&color, NULL);
    pthread_mutex_init(&waiting_students, NULL);
    pthread_mutex_init(&comp_students, NULL);
    for (int i = 0; i < m; i++)
    {
        pthread_mutex_init(&eights[i + 1], NULL);
    }
    for (int i = 1; i <= n; i++)
    {
        pthread_mutex_init(&remcomp[i], NULL);
    }
   pthread_t tid[o];

    for (i = 1; i <= o; i = i + 1)
    {
        input[i] = (s *)malloc(sizeof(s));
        input[i]->id = i;
        input[i]->count = 1;
        pthread_create(&tid[i - 1], NULL, student, (void *)(input[i]));
    }
    
    

    pthread_t tid1[m];
    for (i = 1; i <= m; i = i + 1)
    {
        input1[i] = (zone *)malloc(sizeof(zone));
        input1[i]->id = i;
        input1[i]->total = 0;
        pthread_create(&tid1[i - 1], NULL, vaccizone, (void *)(input1[i]));
    }





    pthread_t tid2[n];
    for (i = 1; i <= n; i = i + 1)
    {
        input2[i] = (comp *)malloc(sizeof(comp));
        input2[i]->id = i;
        input2[i]->prob = prob[i];

        pthread_create(&tid2[i - 1], NULL, pharmacomp, (void *)(input2[i]));
    }

    for (i = 0; i < o; i = i + 1)
    {
        pthread_join(tid[i], NULL);
    }
    for (i = 0; i < m; i = i + 1)
    {
        pthread_join(tid1[i], NULL);
    }
    for (i = 0; i < n; i = i + 1)
    {
        pthread_join(tid2[i], NULL);
    }
    printf(GREEN);
    printf("Simulation is over \n\n");
    printf(RESET);
    return 0;
}