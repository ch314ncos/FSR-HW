#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// кормен
int m; // размер хэш-таблицы
double a = 3.0;


int hash_substract(int value) {
    return value % m;
}

int hash_multiply(int value) {
    int w = 32; // 32 бит
    double A = (pow(5, 0.5) - 1)/2; // 0,6180339887

    return (int)(m*(value*A - (int)(value*A)));
} 

int hash_xor(int value) {
    value ^= (value >> 16);
    value ^= (value << 10);
    value ^= (value >> 5);
    return value % m;
}

int hash_knuth(int value) {
    return (value * 2654435761u) % m;
}




// ________________________________МЕТОД ЦЕПОЧЕК_________________________________________
typedef struct Elem {
    int value;
    struct Elem* next, *prev;
} Elem;

typedef struct { 
    Elem* head, *tail;
    int length;
} List; 

Elem* create_elem(int value) { 
    Elem* elem = malloc(sizeof(Elem));
    elem->value = value;
    elem->next = NULL;
    elem->prev = NULL;

    return elem;
}


List* create_list() {
    List* list = malloc(sizeof(List));
    list->head = NULL;
    list->tail = NULL;
    list->length = 0;
    return list;
}

void list_pushfront(List* list, int value) {
    Elem* elem = create_elem(value);

    if (!list->length) {
        list->head = elem;
        list->tail = elem;
    }
    else {
        list->head->prev = elem; 
        elem->next = list->head;
        list->head = elem;
    }

    list->length++;
}

int list_find(List* list, int value) { // будем возвращать позицию в списке от 0 индексации
    Elem* tmp = list->head;
    int idx=0;
    int f=0;
    while (!f && tmp) {
        if (tmp->value == value) 
            f = 1;
        tmp = tmp->next;
        idx++;
    }

    return f ? idx : -1;
}

void list_delete(List* list, int value) {
    Elem* tmp = list->head;

    int d=0;
    while (!d && tmp) {
        if (tmp->value == value) {
            d=1;
            
            if (tmp->prev) 
                tmp->prev->next = tmp->next; 
            else // нет предыд значит первый
                list->head = tmp->next;
            if(tmp->next) 
                tmp->next->prev = tmp->prev;
            else // нет следующего значит последний
                list->tail = tmp->prev; 
        }
        else 
            tmp = tmp->next;
    }

    if (d) {
        free(tmp);
        list->length--;
    }
}


void print_list(List* list) {
    Elem* tmp = list->head;

    printf("List: ");
    while (tmp) {
        printf("%d ", tmp->value);
        tmp = tmp->next;
    }
    printf("\n");
}

void clear_list(List* list) {
    Elem* tmp = list->head;
    Elem* next = NULL;

    while (tmp) {
        next = tmp->next;
        free(tmp);
        tmp = next;
    }

    list->head = NULL;
    list->tail = NULL;
    list->length = 0;
}


void chained_hash_insert(List** HTable, int (*hash)(int), int value) {
    int k = hash(value);
    list_pushfront(HTable[k], value);
}

int chained_hash_find(List** HTable, int(*hash)(int), int value) {
    int k = hash(value);
    return list_find(HTable[k], value) != -1; // мб еще k возвращать а не "какой то индекс" то
}

void chained_hash_delete(List** HTable, int (*hash)(int), int value) {
    int k = hash(value);
    list_delete(HTable[k], value);
}


// поиск простого числа m для заданого n и a
int is_prime(int x) {
    if (x < 2) return 0;
    for (int i = 2; i * i <= x; i++)
        if (x % i == 0) return 0;
    return 1;
}

int next_prime(int x) {
    while (!is_prime(x)) x++;
    return x;
}

int choose_m(int n, double a) {
    int approx = (int)(n / a);
    return next_prime(approx);
}

typedef struct {
    double insert, search, del_success, del_fail;
} Metrics;

Metrics test_chained_hash(int (*hash)(int), int n) {
    // будем считать что коэфф заполнения a = n/m.
    // как вариант можно добавить разные a от 0.5 до 5 

    int reps=10000; // так как время операций все равно очень быстрое раздуем количество тестов

    List** HTable = malloc(m*sizeof(List*));
    for (int i=0; i<m; i++) 
        HTable[i] = create_list();

    // получаем n рандомных значений от 0 до n
    int* test_values = malloc(n*sizeof(int));
    for(int i=0; i<n; i++)
        test_values[i] = (double) rand()/RAND_MAX * n;

    // INSERT
    double insert_time = 0;
    clock_t start, end;
    for (int r=0; r<reps; r++) {

        start = clock();
        for (int i = 0; i < n; i++)
            chained_hash_insert(HTable, hash, test_values[i]);
        end = clock();

        insert_time += (double)(end - start) / CLOCKS_PER_SEC;
        for (int i=0; i<m; i++) 
            clear_list(HTable[i]);
    }
    
    insert_time /= (n*reps);

    // SEARCH
    for (int i = 0; i < n; i++)
        chained_hash_insert(HTable, hash, test_values[i]);

    // создаем массив рандомных значений, которые будем искать в таблице
    int k = n / 2;
    int* to_search = malloc(k * sizeof(int));
    for (int i = 0; i < k; i++)
        to_search[i] = rand() % (10 * n);

    start = clock();
    for (int r = 0; r < reps; r++)
        for (int i = 0; i < k; i++)
            chained_hash_find(HTable, hash, to_search[i]);
    end = clock();

    double search_time =
    (double)(end - start) / CLOCKS_PER_SEC / (reps * k);

    for (int i=0; i<m; i++) 
        clear_list(HTable[i]);

    
    // DELETE только успешные
    int* successful_delete = test_values; 
    double delete_success_time=0;

    for (int r=0; r<reps; r++) {
        for (int i = 0; i < n; i++)
        chained_hash_insert(HTable, hash, test_values[i]);
        
        start = clock();
        for (int i=0; i<k; i++) {
            chained_hash_delete(HTable, hash, successful_delete[i]);
        }
        end = clock();
        
        delete_success_time += (double) (end-start) / CLOCKS_PER_SEC;

        for (int i=0; i<m; i++) 
            clear_list(HTable[i]);
    }
    
    delete_success_time /= (reps*k);
   
    // DELETE только неуспешные
    int *unsuccessful_delete = malloc(k*sizeof(int));
    for (int i=0; i<k; i++) 
        unsuccessful_delete[i] = (double) rand()/RAND_MAX * n + 2*n;
    double delete_unsuccess_time = 0;
    for (int r=0; r<reps; r++) {
        for (int i = 0; i < n; i++)
        chained_hash_insert(HTable, hash, test_values[i]);
        
        start = clock();
        for (int i=0; i<k; i++) {
            chained_hash_delete(HTable, hash, unsuccessful_delete[i]);
        }
        end = clock();
        
        delete_unsuccess_time += (double) (end-start) / CLOCKS_PER_SEC;

        for (int i=0; i<m; i++) 
            clear_list(HTable[i]);
    }
    
    delete_unsuccess_time /= (reps*k);
    

    for (int i=0; i<m; i++) {
        clear_list(HTable[i]);
        free(HTable[i]);
    }
    free(HTable);
    free(unsuccessful_delete);
    //free(successful_delete);
    successful_delete = NULL;
    free(to_search);
    free(test_values);

    
    return (Metrics){insert_time, search_time, delete_success_time, delete_unsuccess_time};
}

void chained_test_series (
    Metrics (*test_func) (int (*hash)(int), int), 
    int (*hash)(int), 
    const char *test_func_name, 
    const char *hash_name
) {
    FILE *RESULTS;
    RESULTS = fopen("results.txt", "a");
    if (RESULTS) {
        fprintf(RESULTS, 
            "Метод разрешения коллизий %s. Хэш-функция %s\n", 
            test_func_name, 
            hash_name);
        fclose(RESULTS);
    }

    int tests = 10; // количествов тестов
    
    for (int n=100; n<=10000; n*=2) {
        m = choose_m(n, a);

        RESULTS = fopen("results.txt", "a");
            if (RESULTS) {
                fprintf(RESULTS, "Количество элементов в таблице n = %d. ", n);
                fprintf(RESULTS, "Значения m = %d и a = %lf.\n", m, a);
                fprintf(RESULTS, "Для поиска и удаления используем k = n/2 = %d прозвольных значений.\n", n/2);
                fclose(RESULTS);
            } 

        Metrics sum = {0, 0, 0, 0};
        for (int t=0; t<tests; t++) {
            Metrics metr = test_func(hash, n);

            sum.insert += metr.insert;
            sum.search += metr.search;
            sum.del_success += metr.del_success;
            sum.del_fail += metr.del_fail;
        }

        Metrics avg = {
            sum.insert / tests *1e9, 
            sum.search / tests *1e9, 
            sum.del_success / tests *1e9, 
            sum.del_fail / tests *1e9};
        
        // сделаем файлик и туда будем запиывать результаты для каждого n
        RESULTS = fopen("results.txt", "a");
        if (RESULTS) {
            fprintf(RESULTS,
                    "Среднее время вставки = %.2lf ns, поиска = %.2lf ns, успешного удаления = %.2lf ns, неудачного удаления = %.2lf ns.\n\n", 
                    avg.insert, avg.search, avg.del_success, avg.del_fail);
            fclose(RESULTS);
        } //  else return 1; чето вернуть если файлик не открылся
    }

    RESULTS = fopen("results.txt", "a");
        if (RESULTS) {
            fprintf(RESULTS,
                "..............................Конец теста..................................................... \n\n");
            fclose(RESULTS);
        } //  else return 1; чето вернуть если файлик не открылся
}




// ________________________________Открытая адресация_________________________________________              
#define EMPTY 0
#define DELETED -1
#define OCCUPIED 1

// a <= 1

typedef struct OAElem {
    int value; 
    int flag; // DELETED = -1, NULL = 0, 
} OAElem;


// int hash_lin_probe(int (*hash)(int), int value, int i) {
//     return (hash(value) + i) % m;
// } // если с ней делать, то доп параметры везде пихать надо

void lin_probe_insert(OAElem* HTable, int (*hash)(int), int value) {
    int i=0; 
    int k = hash(value);
    while (i<m) {
        int j = (k+i) % m;
        if (HTable[j].flag == EMPTY || HTable[j].flag == DELETED) {
            HTable[j].value = value;
            HTable[j].flag = OCCUPIED;
            return;
        }
        i++;
    }

    printf("INSERTION ERROR: no space to insert");
    return;
}

int lin_probe_search(OAElem* HTable, int (*hash)(int), int value) {
    int i=0;
    int k = hash(value);
    while (i<m) {
        int j = (k+i) % m;
        if (HTable[j].flag == EMPTY)
            return -1; 

        if (HTable[j].flag == OCCUPIED && HTable[j].value == value) 
            return j;
    
        i++;        
    }
    
    return -1;
}

void lin_probe_delete(OAElem* HTable, int (*hash)(int), int value) {
    int j = lin_probe_search(HTable, hash, value);

    if (j != -1) 
        HTable[j].flag = DELETED;
}

int A, B; // положительные константы для квадратичного пробирования пускай 1 и 1

void quad_probe_insert(OAElem* HTable, int(*hash)(int), int value) {
    int i=0; 
    int k = hash(value);
    while (i<m) {
        int j = (k + A*i + B*i*i) % m;
        if (HTable[j].flag == EMPTY || HTable[j].flag == DELETED) {
            HTable[j].value = value;
            HTable[j].flag = OCCUPIED;
            return;
        }
        i++;
    }

    printf("INSERTION ERROR: no space to insert");
    return;
}

int quad_probe_search(OAElem* HTable, int (*hash)(int), int value) {
    int i=0;
    int k = hash(value);
    while (i<m) {
        int j = (k + A*i + B*i*i) % m;
        
        if (HTable[j].flag == EMPTY)
            return -1; 

        if (HTable[j].flag == OCCUPIED && HTable[j].value == value) 
            return j;
    
        i++;        
    }
    
    return -1;
}

void quad_probe_delete(OAElem* HTable, int (*hash)(int), int value) {
    int j = quad_probe_search(HTable, hash, value);

    if (j != -1) 
        HTable[j].flag = DELETED;
}

// void print_table(OAElem* HTable) {
//     printf("\nTABLE:\n");
//     for (int i = 0; i < m; i++) {
//         printf("[%d]: ", i);
//         if (HTable[i].flag == EMPTY) printf("EMPTY\n");
//         else if (HTable[i].flag == DELETED) printf("DELETED\n");
//         else printf("%d\n", HTable[i].value);
//     }
// }

void reset_table(OAElem* HTable) {
    for (int i = 0; i < m; i++) {
        HTable[i].flag = EMPTY;
        HTable[i].value = 0;
    }
}


Metrics test_lin_probe(int (*hash)(int), int n) {
    int reps = 10000;

    OAElem* HTable = malloc(m * sizeof(OAElem));
    reset_table(HTable);

    int* test_values = malloc(n * sizeof(int));
    for(int i=0; i<n; i++)
        test_values[i] = (double) rand()/RAND_MAX * n;

    int k = n / 2;


    // INSERT
    double insert_time = 0;
    clock_t start, end;

    for (int r=0; r<reps; r++) {
        start = clock();
        for (int i=0; i<n; i++) 
            lin_probe_insert(HTable, hash, test_values[i]);
        end = clock();

        insert_time += (double)(end - start) / CLOCKS_PER_SEC;
        
        reset_table(HTable);
    }

    insert_time /= (reps*n);


    // SEARCH
    for (int i=0; i<n; i++) 
            lin_probe_insert(HTable, hash, test_values[i]);

    int* to_search = malloc(k*sizeof(int));
    for(int i=0; i<k; i++)
        to_search[i] = (double) rand()/RAND_MAX * n;

    double search_time = 0;
    start = clock();
    for (int r=0; r<reps; r++) {
        for (int i=0; i<k; i++)
            lin_probe_search(HTable, hash, to_search[i]);
    }
    end = clock();

    search_time = (double)(end - start) / CLOCKS_PER_SEC / (reps * k);

    reset_table(HTable);


    // DELETE успешные
    double del_success_time = 0;
    for (int r = 0; r < reps; r++) {

        for (int i = 0; i < n; i++)
            lin_probe_insert(HTable, hash, test_values[i]);

        start = clock();
        for (int i = 0; i < k; i++)
            lin_probe_delete(HTable, hash, test_values[i]);
        end = clock();

        del_success_time += (double)(end - start) / CLOCKS_PER_SEC;

        reset_table(HTable);
    }

    del_success_time /= (reps * k);


    // DELETE провальные
    double del_fail_time = 0;
    int *bad_delete = malloc(k*sizeof(int));
    for (int i=0; i<k; i++) 
        bad_delete[i] = (double) rand()/RAND_MAX * n + 2*n;

     for (int r = 0; r < reps; r++) {

        for (int i = 0; i < n; i++)
            lin_probe_insert(HTable, hash, test_values[i]);

        start = clock();
        for (int i = 0; i < k; i++)
            lin_probe_delete(HTable, hash, bad_delete[i]);
        end = clock();

        del_fail_time += (double)(end - start) / CLOCKS_PER_SEC;

        reset_table(HTable);
    }

    del_fail_time /= (reps * k);



    // чистим чистим чистим
    free(HTable);
    free(test_values);
    free(to_search);
    free(bad_delete);

    return (Metrics) {insert_time, search_time, del_success_time, del_fail_time};
}


Metrics test_quad_probe(int (*hash)(int), int n) {

    int reps = 10000;

    OAElem* HTable = malloc(m * sizeof(OAElem));
    reset_table(HTable);

    int* test_values = malloc(n * sizeof(int));
    for(int i=0; i<n; i++)
        test_values[i] = (double) rand()/RAND_MAX * n;

    int k = n / 2;


    // INSERT
    double insert_time = 0;
    clock_t start, end;

    for (int r=0; r<reps; r++) {
        start = clock();
        for (int i=0; i<n; i++) 
            quad_probe_insert(HTable, hash, test_values[i]);
        end = clock();

        insert_time += (double)(end - start) / CLOCKS_PER_SEC;
        
        reset_table(HTable);
    }

    insert_time /= (reps*n);


    // SEARCH
    for (int i=0; i<n; i++) 
            quad_probe_insert(HTable, hash, test_values[i]);

    int* to_search = malloc(k*sizeof(int));
    for(int i=0; i<k; i++)
        to_search[i] = (double) rand()/RAND_MAX * n;

    double search_time = 0;
    start = clock();
    for (int r=0; r<reps; r++) {
        for (int i=0; i<k; i++)
            quad_probe_search(HTable, hash, to_search[i]);
    }
    end = clock();

    search_time = (double)(end - start) / CLOCKS_PER_SEC / (reps * k);

    reset_table(HTable);


    // DELETE успешные
    double del_success_time = 0;
    for (int r = 0; r < reps; r++) {

        for (int i = 0; i < n; i++)
            quad_probe_insert(HTable, hash, test_values[i]);

        start = clock();
        for (int i = 0; i < k; i++)
            quad_probe_delete(HTable, hash, test_values[i]);
        end = clock();

        del_success_time += (double)(end - start) / CLOCKS_PER_SEC;

        reset_table(HTable);
    }

    del_success_time /= (reps * k);


    // DELETE провальные
    double del_fail_time = 0;
    int *bad_delete = malloc(k*sizeof(int));
    for (int i=0; i<k; i++) 
        bad_delete[i] = (double) rand()/RAND_MAX * n + 2*n;

     for (int r = 0; r < reps; r++) {

        for (int i = 0; i < n; i++)
            quad_probe_insert(HTable, hash, test_values[i]);

        start = clock();
        for (int i = 0; i < k; i++)
            quad_probe_delete(HTable, hash, bad_delete[i]);
        end = clock();

        del_fail_time += (double)(end - start) / CLOCKS_PER_SEC;

        reset_table(HTable);
    }

    del_fail_time /= (reps * k);



    // чистим чистим чистим
    free(HTable);
    free(test_values);
    free(to_search);
    free(bad_delete);

    return (Metrics) {insert_time, search_time, del_success_time, del_fail_time};
}


void lin_quad_test_series(
    Metrics (*test_func)(int (*hash)(int), int),
    int (*hash)(int),
    const char *test_func_name,
    const char *hash_name
) {
    FILE *RESULTS;
    RESULTS = fopen("results.txt", "a");
    if (RESULTS) {
        fprintf(RESULTS, 
            "Метод разрешения коллизий %s. Хэш-функция %s\n", 
            test_func_name, 
            hash_name);
        fclose(RESULTS);
    }

    int tests = 10; // количествов тестов

    double alphas[] = {0.5, 0.7, 0.9};
    int alpahas_count = sizeof(alphas)/sizeof(double);

    for (int ai=0; ai<alpahas_count; ai++) {
        a = alphas[ai];

        RESULTS = fopen("results.txt", "a");
        if (RESULTS) {
            fprintf(RESULTS,
            "Используемый коэффициент заполнения a = %.2lf\n", 
            a
            );
            fclose(RESULTS);
        }

        for (int n=100; n<=5000; n*=2) {
            A = 1, B = 1;
            m = choose_m(n, a);

            RESULTS = fopen("results.txt", "a");
            if (RESULTS) {
                fprintf(RESULTS, "Количество элементов в таблице n = %d. ", n);
                fprintf(RESULTS, "Значение m = %d\n", m);
                fprintf(RESULTS, "Для поиска и удаления используем k = n/2 = %d прозвольных значений.\n", n/2);
                if (test_func == test_quad_probe) fprintf(RESULTS, "Используем A = %d, B = %d\n", A, B);
                fclose(RESULTS);
            } 

            Metrics sum = {0, 0, 0, 0};

            for (int t = 0; t < tests; t++) {

                Metrics metr = test_func(hash, n);

                sum.insert += metr.insert;
                sum.search += metr.search;
                sum.del_success += metr.del_success;
                sum.del_fail += metr.del_fail;
            }

            Metrics avg = {
                sum.insert / tests * 1e9,
                sum.search / tests * 1e9,
                sum.del_success / tests * 1e9,
                sum.del_fail / tests * 1e9
            };

            RESULTS = fopen("results.txt", "a");
            if (RESULTS) {
                fprintf(RESULTS,
                        "Среднее время вставки = %.2lf ns, поиска = %.2lf ns, успешного удаления = %.2lf ns, неудачного удаления = %.2lf ns.\n\n", 
                        avg.insert, avg.search, avg.del_success, avg.del_fail);
                fclose(RESULTS);
            } 
        }
    }

    RESULTS = fopen("results.txt", "a");
    if (RESULTS) {
        fprintf(RESULTS,
            "..............................Конец теста..................................................... \n\n");
        fclose(RESULTS);
    } //  else return 1; чето вернуть если файлик не открылся
}


// RAND_MAX = 32767
int main() {
    srand(time(NULL));

    // printf(".........BENCHMARK........\n");
    // chained_test_series(test_chained_hash, hash_substract,
    //             "chaining", "modulo");

    // printf(".........BENCHMARK........\n");
    // chained_test_series(test_chained_hash, hash_multiply,
    //             "chaining", "multiplication");
    // 


    // printf(".........BENCHMARK........\n");
    // lin_quad_test_series(test_quad_probe, hash_substract,
    //             "quadratic probing", "modulo");

    // printf(".........BENCHMARK........\n");
    // chained_test_series(test_chained_hash, hash_xor,
    //             "chaining", "xor");

    // printf(".........BENCHMARK........\n");
    // lin_quad_test_series(test_lin_probe, hash_xor,
    //             "linear", "xor");

    // printf(".........BENCHMARK........\n");
    // lin_quad_test_series(test_quad_probe, hash_xor,
    //             "quad", "xor");

    // printf(".........BENCHMARK........\n");
    // chained_test_series(test_chained_hash, hash_knuth,
    //             "chaining", "knuth");

    // printf(".........BENCHMARK........\n");
    // lin_quad_test_series(test_lin_probe, hash_knuth,
    //             "linear", "knuth");

    // printf(".........BENCHMARK........\n");
    // lin_quad_test_series(test_quad_probe, hash_knuth,
    //             "quad", "knuth");

    // printf(".........BENCHMARK........\n");
    // lin_quad_test_series(test_lin_probe, hash_multiply,
    //             "linear", "multiplication");

    // printf(".........BENCHMARK........\n");
    // lin_quad_test_series(test_quad_probe, hash_multiply,
    //             "quad", "multiplication");

    return 0;
}



/* 
ОБЪЯСНЕНИЕ ОТЛИЧИЙ РЕЗУЛЬТАТОВ
Положим a коэф заполненности хэш-таблицы
Для метода цепочек количество операций для вставки O(1), поиска как удачного, так и неудачного O(1+a), а значит и время удалений O(1+a)
в нашем случае a = 3, поэтому малое время получалось

В линейном пробировании удачный поиск O(1 + 1/(1-a)), неудачный поиск O(1 + 1/(1-a)^2). С учетом времени проверок и тд получаются такие большие числа >1000ns

В квадратичном пробировании так как не образуются большие кластеры, количество операций сокращается в разы
*/

















// рандомно генерить операции много штук 50к и их рандомно вызвать
// считать общее время каждой операции