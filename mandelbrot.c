#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "timer.h"

#define fpow(x) (x) * (x)
#define square_norm(x, y) fpow(x) + fpow(y)
#define random_double(a, b) a + rand_r(&rand_state) / ((double)RAND_MAX) * (b - a);
#define write_to_buf_next() COMPLEX* c = &global_points_buf[index]; c->real = c_real; c->img = c_img; ++i;

long nthreads;
long npoints;

const int iterations_to_include = 1500;
const double real_min = -2.;
const double real_max = 1.;
const double img_min = -1.;
const double img_max = 1.;

struct thread_args {
    int start;
    int end;
} typedef ARGS;

struct complex {
    double real;
    double img;
} typedef COMPLEX;

COMPLEX* global_points_buf;

double ro(double x, double y) {
    return sqrt(fpow(x - 1. / 4.) + fpow(y));
}

double ro_c(double x, double y) {
    double theta = atan2(y, x - 1. / 4.);
    return 1. / 2. - 1. / 2. * cos(theta);
}

int check_main_cardiod(double x, double y) {
    double r = ro(x, y);
    double r_c = ro_c(x, y);
    return r <= r_c;
}

void* mandelbrot(void* arg) {
    ARGS* my_args = (ARGS*)arg;
    long points = my_args->end - my_args->start;

    unsigned int rand_state = rand();

    for (long i = 0; i < points;) {
        // rand() is actually not thread-safe
        double c_real = random_double(real_min, real_max);
        double c_img = random_double(img_min, img_max);
        double zn_real = 0;
        double zn_img = 0;

        int index = my_args->start + i;

        if (check_main_cardiod(c_real, c_img)) {
            write_to_buf_next()
            continue;
        }

        // temporary variables
        double t_r;
        double t_i;
        for (long j = 0; j < iterations_to_include; ++j) {
            t_r = zn_real * zn_real - zn_img * zn_img + c_real;
            t_i = 2 * zn_real * zn_img + c_img;
            zn_real = t_r;
            zn_img = t_i;

            // calculating x * x is faster than pow(x, 2)
            if (square_norm(zn_real, zn_img) >= 4) {
                break;
            }
        }

        if (square_norm(zn_real, zn_img) < 4) {
            write_to_buf_next()
        }
    }
    free(my_args);
}

ARGS* new_args(int start, int end) {
    ARGS* args = malloc(sizeof(ARGS));
    args->start = start;
    args->end = end;
    return args;
}

pthread_t* create_threads() {
    long points_per_thread = npoints / nthreads;

    pthread_t* threads = malloc(nthreads * sizeof(pthread_t));
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    for (long i = 0; i < nthreads - 1; i++) {
        ARGS* args = new_args(i * points_per_thread, (i + 1) * points_per_thread);
        pthread_create(&threads[i], &attr, mandelbrot, args);
    }

    ARGS* args = new_args((nthreads - 1) * points_per_thread, npoints);
    pthread_create(&threads[nthreads - 1], &attr, mandelbrot, args);

    return threads;
}

void join_threads(pthread_t* threads) {
    for (long i = 0; i < nthreads; i++) {
        pthread_join(threads[i], NULL);
    }
    free(threads);
}

int check_input(int argc, char* argv[]) {
    if (argc != 3) 
    {
        printf("Использование: %s <количество потоков> <количество попыток>\n",
               argv[0]);
        return 1;
    }
    if (argv[1] <= 0 || argv[2] <= 0) 
    {
        printf("Ошибка: количество потоков и попыток должно быть положительным!\n");
        return 1;
    }

    return 0;
}

void write_to_csv(char* filename) {
    FILE* csv;
    csv = fopen(filename, "w");
    fprintf(csv, "real,img\n");
    for (int i = 0; i < npoints; ++i) {
        COMPLEX c = global_points_buf[i];
        fprintf(csv, "%.10f,%.10f\n", c.real, c.img);
    }
    fclose(csv);
}

int main(int argc, char* argv[]) {
    if(check_input(argc, argv)) {
        return 1;
    }

    srand(time(NULL));

    nthreads = atoi(argv[1]);
    npoints = atoi(argv[2]);

    global_points_buf = malloc(npoints * sizeof(COMPLEX));

    double start, end;

    GET_TIME(start);
    pthread_t* threads = create_threads();
    join_threads(threads);
    GET_TIME(end);

    printf("%.5f\n", end - start);
    write_to_csv("mandelbrot.csv");

    free(global_points_buf);

    return 0;
}
