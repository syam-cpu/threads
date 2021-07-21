#define _POSIX_C_SOURCE 199309L //required for clock
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>

int *shareMem(size_t size)
{
    key_t mem_key = IPC_PRIVATE;
    int shm_id = shmget(mem_key, size, IPC_CREAT | 0666);
    return (int *)shmat(shm_id, NULL, 0);
}

void selectionsort(int arr[], int l, int r)
{

    for (int i = l; i <= r; i++)
    {
        int min_index = i;
        for (int j = i + 1; j <= r; j++)
        {
            if (arr[j] < arr[min_index])
            {
                min_index = j;
            }

            int temp = arr[i];
            arr[i] = arr[min_index];
            arr[min_index] = temp;
        }
    }
}
void merge(int arr[], int l, int m, int r)
{
    int n1 = m - l + 1;
    int n2 = r - m;

    // Create temp arrays
    int L[n1], R[n2];

    // Copy data to temp arrays L[] and R[]
    for (int i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (int j = 0; j < n2; j++)
        R[j] = arr[m + 1 + j];

    // Merge the temp arrays back into arr[l..r]

    // Initial index of first subarray
    int i = 0;

    // Initial index of second subarray
    int j = 0;

    // Initial index of merged subarray
    int k = l;

    while (i < n1 && j < n2)
    {
        if (L[i] <= R[j])
        {
            arr[k] = L[i];
            i++;
        }
        else
        {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    // Copy the remaining elements of
    // L[], if there are any
    while (i < n1)
    {
        arr[k] = L[i];
        i++;
        k++;
    }

    // Copy the remaining elements of
    // R[], if there are any
    while (j < n2)
    {
        arr[k] = R[j];
        j++;
        k++;
    }
}

void mergesort(int arr[], int l, int r)
{
    if (l < r)
    {
        if (r - l + 1 <= 4)
        {
            selectionsort(arr, l, r);
            return;
        }
        int pid1;
        int m = l + (r - l) / 2;
        pid1 = fork();
        if (pid1 == 0)
        {
            mergesort(arr, l, m);
            exit(1);
        }
        else
        {
            int pid2;
            pid2 = fork();
            if (pid2 == 0)
            {

                mergesort(arr, m + 1, r);
                exit(1);
            }
            else
            {
                int status;
                waitpid(pid1, &status, 0);
                waitpid(pid2, &status, 0);
                merge(arr, l, m, r);
            }
        }
    }
}
void normal_mergesort(int crr[], int l, int r)
{
    if (l < r)
    {
        if (r - l + 1 > 4)
        {
            int m = l + (r - l) / 2;
            normal_mergesort(crr, l, m);
            normal_mergesort(crr, m + 1, r);
            merge(crr, l, m, r);
        }
        else
        {
            selectionsort(crr, l, r);
        }
    }
}
struct arg
{
    int l;
    int r;
    int *arr;
};
void *threaded_mergesort(void *a)
{
    struct arg *args = (struct arg *)a;

    int l = args->l;
    int r = args->r;
    int m = l + (r - l) / 2;
    int *arr = args->arr;
    if (r - l + 1 < 5)
    {
        selectionsort(arr, l, r);
        return NULL;
    }

    if (l > r)
        return NULL;
    //sort left half array

    struct arg a1;
    a1.l = l;
    a1.r = m;
    a1.arr = arr;
    pthread_t tid1;
    pthread_create(&tid1, NULL, threaded_mergesort, &a1);
    struct arg a2;
    a2.l = m + 1;
    a2.r = r;
    a2.arr = arr;
    pthread_t tid2;
    pthread_create(&tid2, NULL, threaded_mergesort, &a2);
    //wait for two halves get sorted
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    merge(arr, l, m, r);
}
void runsorts(long long int n)
{
    struct timespec ts;
    //getting shared memory
    int *arr = shareMem(sizeof(int) * (n + 1));
    for (int i = 0; i < n; i++)
    {
        scanf("%d", arr + i);
    }
    int brr[n + 1];
    int crr[n + 1];
    for (int i = 0; i < n; i++)
        brr[i] = arr[i];
    for (int i = 0; i < n; i++)
        crr[i] = arr[i];
    printf("running concuurent mergesort for n = %lld\n", n);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double st = ts.tv_nsec / (1e9) + ts.tv_sec;
    //multiprocess mergesort
    mergesort(arr, 0, n - 1);
    for (int i = 0; i < n; i++)
    {
        printf("%d ", arr[i]);
    }
    printf("\n");
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double en = ts.tv_nsec / (1e9) + ts.tv_sec;
    printf("time = %Lf\n", en - st);
    long double t1 = en - st;
    pthread_t tid;
    struct arg a;
    a.l = 0;
    a.r = n - 1;
    a.arr = brr;
    printf("Running threaded_mergesort for n = %lld\n", n);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_nsec / (1e9) + ts.tv_sec;
    //multithreaded mergesort
    pthread_create(&tid, NULL, threaded_mergesort, &a);
    pthread_join(tid, NULL);
    for (int i = 0; i < n; i++)
    {
        printf("%d ", a.arr[i]);
    }
    printf("\n");
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    en = ts.tv_nsec / (1e9) + ts.tv_sec;
    printf("time  = %Lf\n", en - st);
    long double t2 = en - st;
    printf("Running normal merge_sort for n = %lld\n", n);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_nsec / (1e9) + ts.tv_sec;
    //normal mergesort
    normal_mergesort(crr, 0, n - 1);
    for (int i = 0; i < n; i++)
    {
        printf("%d ", crr[i]);
    }
    printf("\n");
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    en = ts.tv_nsec / (1e9) + ts.tv_sec;
    printf("time = %Lf\n", en - st);
    long double t3 = en - st;
    printf("normal_mergesort ran:\n\t[ %Lf ] times faster than concurrent_mergesort\n\t[ %Lf ] times faster than threaded_concurrent_mergesort\n\n\n", t1 / t3, t2 / t3);

    shmdt(arr);
    return;
}

int main()
{
    long long int n;
    scanf("%lld", &n);
    runsorts(n);
    return 0;
}
