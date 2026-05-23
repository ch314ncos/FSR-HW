#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// кормен
int m; // размер хэш-таблицы
double a = 3.0;

int cnt_collisions = 0;


// ___________________________________ХЭШ-ФУНКЦИИ__________________________________________
int hash_substract(int value) {
    return value % m;
}

int hash_bad(int value) {
    return value % 256;
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

        cnt_collisions++;
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


// Поиск простого числа m для размера хэш-таблицы
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
    double insert, search, delete;
} Metrics;



// ________________________________Открытая адресация_________________________________________              
#define EMPTY 0
#define DELETED -1
#define OCCUPIED 1

// a <= 1

typedef struct OAElem {
    int value; 
    int flag; // DELETED = -1, NULL = 0, 
} OAElem;


void lin_probe_insert(OAElem* HTable, int (*hash)(int), int value) {
    int i=0; 
    int k = hash(value);
    while (i<m) {
        int j = (k+i) % m;

        if (HTable[j].flag == OCCUPIED) 
            cnt_collisions++;   // считаем коллизии
        

        if (HTable[j].flag == EMPTY || HTable[j].flag == DELETED) {
            HTable[j].value = value;
            HTable[j].flag = OCCUPIED;
            return;
        }
        i++;
    }

    // printf("INSERTION ERROR: no space to insert");
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

        if (HTable[j].flag == OCCUPIED) 
            cnt_collisions++;   // считаем коллизии
        

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

void print_table(OAElem* HTable) {
    printf("\nTABLE:\n");
    for (int i = 0; i < m; i++) {
        printf("[%d]: ", i);
        if (HTable[i].flag == EMPTY) printf("EMPTY\n");
        else if (HTable[i].flag == DELETED) printf("DELETED\n");
        else printf("%d\n", HTable[i].value);
    }
}

void reset_table(OAElem* HTable) {
    for (int i = 0; i < m; i++) {
        HTable[i].flag = EMPTY;
        HTable[i].value = 0;
    }
}



// ____________________________________________________ТЕСТЫ_________________________________________________________
enum Operation {
    INSERT = 0, SEARCH = 1, DELETE = 2
};

void random_lin_probe_test_series(int (*hash)(int), const char *hash_name) {
    int ops = 50000;

    int tests = 50;

    printf("Random\n");
    printf("Operations: %d\ttests: %d\n", ops, tests);

    for (int n = 10000; n < 50000; n = (int)(n*1.5)) {
        double alphas[] = {0.5, 0.7, 0.9};
        int alphas_count = sizeof(alphas) / sizeof(double);

        for (int ai = 0; ai < alphas_count; ai++) {
            a = alphas[ai];

            m = choose_m(n, a); // a = 3.0
            printf("INPUT:\t n = %d, m = %d, a = %.2lf\n", n, m, a);
    
            OAElem* HTable = malloc(m * sizeof(OAElem));
            reset_table(HTable);
        
            // считаем для общего tests количества, потом амортизируем по tests
            int i_cnt = 0, s_cnt = 0, d_cnt = 0; 
            double i_time = 0.0, s_time = 0.0, d_time = 0.0;
    
            double acc_time = 0.0; // для каждого n считаем суммарное время операций
    
            int total_collisions = 0;
    
            for (int t=0; t<tests; t++) {
                reset_table(HTable);

                // i_cnt = 0, s_cnt = 0, d_cnt = 0; 
    
                clock_t start, end;
                clock_t os, oe; //operation start / end
                start = clock();
                for (int i=0; i< ops; i++) {
                    int value  = (double) rand()/RAND_MAX * n;
        
                    int op = rand() % 3;
            
                    switch (op) {
                        case INSERT:
                            cnt_collisions = 0;
                            os = clock();
                            lin_probe_insert(HTable, hash, value);
                            oe = clock();
                            i_time += (double)(oe - os) / CLOCKS_PER_SEC * 1e6;
                            i_cnt++;
                            total_collisions += cnt_collisions;
                            break;
                        case SEARCH:
                            os = clock();
                            lin_probe_search(HTable, hash, value);
                            oe = clock();
                            s_time += (double)(oe - os) / CLOCKS_PER_SEC * 1e6;
                            s_cnt++;
                            break;
                        case DELETE:
                            os = clock();
                            lin_probe_delete(HTable, hash, value);
                            oe = clock();
                            d_time += (double)(oe - os) / CLOCKS_PER_SEC * 1e6;
                            d_cnt++;
                            break;
                    }
                }
                end = clock();
                acc_time += (double)(end - start) / CLOCKS_PER_SEC *1e6;
            }
    
            acc_time /= tests;
            i_time /= tests;
            s_time /= tests; 
            d_time /= tests;
    
            double avg_collisions = (double)total_collisions / i_cnt;
    
            printf("RESULTS lin probing \thash function %s\ttotal time = %.2lf us\n", hash_name, acc_time);
            printf("Insert time of %d ops\t: %.2lf us\n", i_cnt/tests, i_time);
            printf("Search time of %d ops\t: %.2lf us\n", s_cnt/tests, s_time);
            printf("Delete time of %d ops\t: %.2lf us\n", d_cnt/tests, d_time);
            printf("Collisions number per insert: %.2lf\nEND TESTS\n\n", avg_collisions);
    
            // чистим табличку
            reset_table(HTable);
            free(HTable);

        }
    }
}

void random_quad_probe_test_series(int (*hash)(int), const char *hash_name) {
    int ops = 50000;

    int tests = 50;
    A = 1, B = 1;
    printf("Operations: %d\ttests: %d\n", ops, tests);

    for (int n = 10000; n < 50000; n = (int)(n*1.5)) {
        double alphas[] = {0.5, 0.7, 0.9};
        int alphas_count = sizeof(alphas) / sizeof(double);

        for (int ai = 0; ai < alphas_count; ai++) {
            a = alphas[ai];

            m = choose_m(n, a); // a = 3.0
            printf("INPUT:\t n = %d, m = %d, a = %.2lf\n", n, m, a);
    
            OAElem* HTable = malloc(m * sizeof(OAElem));
        
            // считаем для общего tests количества, потом амортизируем по tests
            int i_cnt = 0, s_cnt = 0, d_cnt = 0; 
            double i_time = 0.0, s_time = 0.0, d_time = 0.0;
    
            double acc_time = 0.0; // для каждого n считаем суммарное время операций
    
            int total_collisions = 0;
    
            for (int t=0; t<tests; t++) {
                reset_table(HTable);

                i_cnt = 0, s_cnt = 0, d_cnt = 0; 
    
                clock_t start, end;
                clock_t os, oe; //operation start / end
                start = clock();
                for (int i=0; i< ops; i++) {
                    int value  = (double) rand()/RAND_MAX * n;
        
                    int op = rand() % 3;
            
                    switch (op) {
                        case INSERT:
                            cnt_collisions = 0;
                            os = clock();
                            quad_probe_insert(HTable, hash, value);
                            oe = clock();
                            i_time += (double)(oe - os) / CLOCKS_PER_SEC * 1e6;
                            i_cnt++;
                            total_collisions += cnt_collisions;
                            break;
                        case SEARCH:
                            os = clock();
                            quad_probe_search(HTable, hash, value);
                            oe = clock();
                            s_time += (double)(oe - os) / CLOCKS_PER_SEC * 1e6;
                            s_cnt++;
                            break;
                        case DELETE:
                            os = clock();
                            quad_probe_delete(HTable, hash, value);
                            oe = clock();
                            d_time += (double)(oe - os) / CLOCKS_PER_SEC * 1e6;
                            d_cnt++;
                            break;
                    }
                }
                end = clock();
                acc_time += (double)(end - start) / CLOCKS_PER_SEC *1e6;
            }
    
            acc_time /= tests;
            i_time /= tests;
            s_time /= tests; 
            d_time /= tests;
    
            double avg_collisions = (double)total_collisions / i_cnt;
    
            printf("RESULTS quad probing \thash function %s\ttotal time = %.2lf us\n", hash_name, acc_time);
            printf("Insert time of %d ops\t: %.2lf us\n", i_cnt/tests, i_time);
            printf("Search time of %d ops\t: %.2lf us\n", s_cnt/tests, s_time);
            printf("Delete time of %d ops\t: %.2lf us\n", d_cnt/tests, d_time);
            printf("Collisions number per insert: %.2lf\nEND TESTS\n\n", avg_collisions);
    
            // чистим табличку
            reset_table(HTable);
            free(HTable);

        }
    }
}

void random_ch_test_series(int (*hash)(int), const char *hash_name) {
    int ops = 100000;

    int tests = 50;

    printf("Random\n");
    printf("Operations: %d\ttests: %d\n", ops, tests);

    for (int n = 10000; n < 50000; n = (int)(n*1.5)) {
        // a = 5.0;
        m = choose_m(n, a); // a = 3.0
        printf("INPUT:\t n = %d, m = %d, a = %.2lf\n", n, m, a);

        List** HTable = malloc(m*sizeof(List*));
        for (int i=0; i<m; i++) 
            HTable[i] = create_list();
    
        // считаем для общего tests количества, потом амортизируем по tests
        int i_cnt = 0, s_cnt = 0, d_cnt = 0; 
        double i_time = 0.0, s_time = 0.0, d_time = 0.0;

        double acc_time = 0.0; // для каждого n считаем суммарное время операций

        int total_collisions = 0;

        for (int t=0; t<tests; t++) {
            // i_cnt = 0, s_cnt = 0, d_cnt = 0; 

            clock_t start, end;
            clock_t os, oe; //operation start / end
            start = clock();
            for (int i=0; i< ops; i++) {
                int value  = (double) rand()/RAND_MAX * n;
    
                int op = rand() % 3;
        
                switch (op) {
                    case INSERT:
                        cnt_collisions = 0;
                        os = clock();
                        chained_hash_insert(HTable, hash, value);
                        oe = clock();
                        i_time += (double)(oe - os) / CLOCKS_PER_SEC * 1e6;
                        i_cnt++;
                        total_collisions += cnt_collisions;
                        break;
                    case SEARCH:
                        os = clock();
                        chained_hash_find(HTable, hash, value);
                        oe = clock();
                        s_time += (double)(oe - os) / CLOCKS_PER_SEC * 1e6;
                        s_cnt++;
                        break;
                    case DELETE:
                        os = clock();
                        chained_hash_delete(HTable, hash, value);
                        oe = clock();
                        d_time += (double)(oe - os) / CLOCKS_PER_SEC * 1e6;
                        d_cnt++;
                        break;
                }
            }
            end = clock();
            acc_time += (double)(end - start) / CLOCKS_PER_SEC *1e6;

            // чистим между тестами
            for (int i=0; i<m; i++) 
                clear_list(HTable[i]);
        }

        acc_time /= tests;
        i_time /= tests;
        s_time /= tests; 
        d_time /= tests;

        double avg_collisions = (double)total_collisions / i_cnt;

        printf("RESULTS chaining \thash function %s\ttotal time = %.2lf us\n", hash_name, acc_time);
        printf("Insert time of %d ops\t: %.2lf us\n", i_cnt/tests, i_time);
        printf("Search time of %d ops\t: %.2lf us\n", s_cnt/tests, s_time);
        printf("Delete time of %d ops\t: %.2lf us\n", d_cnt/tests, d_time);
        printf("Collisions number per insert: %.2lf\nEND TESTS\n\n", avg_collisions);

        // чистим табличку
        for (int i=0; i<m; i++) {
            clear_list(HTable[i]);
            free(HTable[i]);
        }
        free(HTable);
    }
}





void seq_ch_test_series(int (*hash)(int), const char *hash_name){
    printf("Sequently testing.\n");

    int tests = 10000;

   
    for (int n = 10000; n < 50000; n = (int)(n * 1.5)) {
        // a = 5.0;
        m = choose_m(n, a); // a = 3.0
        printf("INPUT:\t n = %d, m = %d, a = %.2lf\n", n, m, a);

        List** HTable = malloc(m*sizeof(List*));
        for (int i=0; i<m; i++) 
            HTable[i] = create_list();
    
        // считаем для общего tests количества, потом амортизируем по tests
        // int i_cnt = 0, s_cnt = 0, d_cnt = 0; 
        double i_time = 0.0, s_time = 0.0, d_time = 0.0;

        double acc_time = 0.0; // для каждого n считаем суммарное время операций

        int max_ch_len = 0; // считаем максимальную длину цепочки и усредним для каждого n
        int sum_maxch = 0;

        int k = n/2; // количество поиска и удалений

        clock_t start, end;
        clock_t os, oe;

        start = clock();
        for (int t=0; t<tests; t++) {
            // n вставок
            os = clock();
            for (int i=0; i<n; i++) {
                int value  = (double) rand()/RAND_MAX * n; // от 0 до n значения формируем
                chained_hash_insert(HTable, hash, value);
            }
            oe = clock();
            // i_cnt = n;
            i_time += (double)(oe - os) / CLOCKS_PER_SEC * 1e6;
    
            // считаем самую длинную цепочку
            max_ch_len = 0;
            for (int i=0; i<m; i++)
                if (HTable[i]->length > max_ch_len)
                    max_ch_len = HTable[i]->length;
            sum_maxch += max_ch_len;
    
            // k поиска
            os = clock();
            for (int i=0; i<k; i++) {
                int value  = (double) rand()/RAND_MAX * n;
                chained_hash_find(HTable, hash, value);
            }
            oe = clock();
            // s_cnt = k;
            s_time += (double)(oe - os) / CLOCKS_PER_SEC * 1e6;
    
            // k удалений
            os = clock();
            for (int i=0; i<k; i++) {
                int value  = (double) rand()/RAND_MAX * n;
                chained_hash_delete(HTable, hash, value);
            }
            oe = clock();
            // d_cnt = k;
            d_time += (double)(oe - os) / CLOCKS_PER_SEC * 1e6;

            // чистим, но тут побочку дает по времени, но должно сгладится 
            for (int i=0; i<m; i++) 
                clear_list(HTable[i]);

        }
        end = clock();

        acc_time = (double)(end - start) / CLOCKS_PER_SEC * 1e6 / tests;
        i_time /= tests;
        s_time /= tests;
        d_time /= tests;
        sum_maxch /= tests;
         

        printf("RESULTS chaining \thash function %s\toperations time = %.2lf us\n", hash_name, acc_time);
        printf("Time for n = %d insert and k = %d search, delete\ti: %.2lf us, s: %.2lf us, d: %.2lf us.\n", n, k, i_time, s_time, d_time);
        printf("Max chain length: %d\nEND TESTS\n\n", max_ch_len);

        // чистим табличку
        for (int i=0; i<m; i++) {
            clear_list(HTable[i]);
            free(HTable[i]);
        }
        free(HTable);
    }
}

void seq_lin_test_series(int (*hash)(int), const char *hash_name) {
    printf("Sequently testing.\n");

    int tests = 100;

    for (int n = 10000; n < 50000; n = (int)(n * 1.5)) {
        double alphas[] = {0.5, 0.7, 0.9};
        int alpahas_count = sizeof(alphas)/sizeof(double);

        for (int ai=0; ai<alpahas_count; ai++) {
            a = alphas[ai];

            m = choose_m(n, a); 
            printf("INPUT:\t n = %d, m = %d, a = %.2lf\n", n, m, a);
    
            OAElem* HTable = malloc(m * sizeof(OAElem));
        
            // считаем для общего tests количества, потом амортизируем по tests
            int i_cnt = 0, s_cnt = 0, d_cnt = 0; 
            double i_time = 0.0, s_time = 0.0, d_time = 0.0;
    
            double acc_time = 0.0; // для каждого n считаем суммарное время операций
    
            int k = n/2; // количество поиска и удалений

            int total_collisions; // считает количество коллизий по всем тестам
    
            clock_t start, end;
            clock_t os, oe;
    
            start = clock();
            for (int t=0; t<tests; t++) {
                reset_table(HTable);

                // n вставок
                os = clock();
                for (int i=0; i<n; i++) {
                    cnt_collisions = 0;
                    int value  = (double) rand()/RAND_MAX * n; // от 0 до n значения формируем
                    lin_probe_insert(HTable, hash, value);
                    total_collisions += cnt_collisions;
                }
                oe = clock();
                i_cnt += n;
                i_time += (double)(oe - os) / CLOCKS_PER_SEC * 1e6;
        
                // k поиска
                os = clock();
                for (int i=0; i<k; i++) {
                    int value  = (double) rand()/RAND_MAX * n;
                    lin_probe_search(HTable, hash, value);
                }
                oe = clock();
                s_cnt += k;
                s_time += (double)(oe - os) / CLOCKS_PER_SEC * 1e6;
        
                // k удалений
                os = clock();
                for (int i=0; i<k; i++) {
                    int value  = (double) rand()/RAND_MAX * n;
                    lin_probe_delete(HTable, hash, value);
                }
                oe = clock();
                d_cnt += k;
                d_time += (double)(oe - os) / CLOCKS_PER_SEC * 1e6;
    
                
            }
            end = clock();
    
            acc_time = (double)(end - start) / CLOCKS_PER_SEC * 1e6 / tests;
            i_time /= tests;
            s_time /= tests;
            d_time /= tests;
            int avg_collisions = total_collisions / i_cnt;
             
    
            printf("RESULTS lin probing \thash function %s\toperations time = %.2lf us\n", hash_name, acc_time);
            printf("Time for n = %d insert and k = %d search, delete\ti: %.2lf us, s: %.2lf us, d: %.2lf us.\n", n, k, i_time, s_time, d_time);
            printf("Collisions number per insert: %d\nEND TESTS\n\n", avg_collisions);
    
            // чистим табличку
            reset_table(HTable);
            free(HTable);
        }
    }
}


void seq_quad_test_series(int (*hash)(int), const char *hash_name) {
    printf("Sequently testing.\n");

    int tests = 100;
    A = 1, B = 1;

    for (int n = 10000; n < 50000; n = (int)(n * 1.5)) {
        double alphas[] = {0.5, 0.7, 0.9};
        int alpahas_count = sizeof(alphas)/sizeof(double);

        for (int ai=0; ai<alpahas_count; ai++) {
            a = alphas[ai];

            m = choose_m(n, a); 
            printf("INPUT:\t n = %d, m = %d, a = %.2lf\n", n, m, a);
    
            OAElem* HTable = malloc(m * sizeof(OAElem));
        
            // считаем для общего tests количества, потом амортизируем по tests
            int i_cnt = 0, s_cnt = 0, d_cnt = 0; 
            double i_time = 0.0, s_time = 0.0, d_time = 0.0;
    
            double acc_time = 0.0; // для каждого n считаем суммарное время операций
    
            int k = n/2; // количество поиска и удалений

            int total_collisions; // считает количество коллизий по всем тестам
    
            clock_t start, end;
            clock_t os, oe;
    
            start = clock();
            for (int t=0; t<tests; t++) {
                reset_table(HTable);

                // n вставок
                os = clock();
                for (int i=0; i<n; i++) {
                    cnt_collisions = 0;
                    int value  = (double) rand()/RAND_MAX * n; // от 0 до n значения формируем
                    quad_probe_insert(HTable, hash, value);
                    total_collisions += cnt_collisions;
                }
                oe = clock();
                i_cnt += n;
                i_time += (double)(oe - os) / CLOCKS_PER_SEC * 1e6;
        
                // k поиска
                os = clock();
                for (int i=0; i<k; i++) {
                    int value  = (double) rand()/RAND_MAX * n;
                    quad_probe_search(HTable, hash, value);
                }
                oe = clock();
                s_cnt += k;
                s_time += (double)(oe - os) / CLOCKS_PER_SEC * 1e6;
        
                // k удалений
                os = clock();
                for (int i=0; i<k; i++) {
                    int value  = (double) rand()/RAND_MAX * n;
                    quad_probe_delete(HTable, hash, value);
                }
                oe = clock();
                d_cnt += k;
                d_time += (double)(oe - os) / CLOCKS_PER_SEC * 1e6;
    
                
            }
            end = clock();
    
            acc_time = (double)(end - start) / CLOCKS_PER_SEC * 1e6 / tests;
            i_time /= tests;
            s_time /= tests;
            d_time /= tests;
            int avg_collisions = total_collisions / i_cnt;
             
    
            printf("RESULTS lin probing \thash function %s\toperations time = %.2lf us\n", hash_name, acc_time);
            printf("Time for n = %d insert and k = %d search, delete\ti: %.2lf us, s: %.2lf us, d: %.2lf us.\n", n, k, i_time, s_time, d_time);
            printf("Collisions number per insert: %d\nEND TESTS\n\n", avg_collisions);
    
            // чистим табличку
            reset_table(HTable);
            free(HTable);
        }
    }
}


// как вариан чтобы не плодить тестовые функции можно сделать функцию с void* на таблицу и потом в функции проверять какого типа, 
// хотя бля там дальше то фукнци-методы вызвать надо, их тогда тоже по указатялеся хуня короче

// RAND_MAX = 32767
int main() {
    srand(time(NULL));

    // seq_lin_test_series(hash_bad, "modulo 2^p");
    // random_ch_test_series(hash_bad, "modulo 2^p");

    random_lin_probe_test_series(hash_substract, "modulo");
    random_lin_probe_test_series(hash_bad, "modulo 2^p");

    // random_lin_probe_test_series(hash_knuth, "knuth");
    // seq_lin_test_series(hash_knuth, "knuth");

    
    // random_ch_test_series(hash_knuth, "knuth");
    // seq_ch_test_series(hash_knuth, "knuth");

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