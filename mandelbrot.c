#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "timer.h"

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
    return sqrt(pow(x - 1. / 4., 2) + pow(y, 2));
}

double ro_c(double x, double y) {
    double teta = atan2(y, x - 1. / 4.);
    return 1. / 2. - 1. / 2. * cos(teta);
}

// https://ru.wikipedia.org/wiki/%D0%9C%D0%BD%D0%BE%D0%B6%D0%B5%D1%81%D1%82%D0%B2%D0%BE_%D0%9C%D0%B0%D0%BD%D0%B4%D0%B5%D0%BB%D1%8C%D0%B1%D1%80%D0%BE%D1%82%D0%B0#%D0%9E%D0%BF%D1%82%D0%B8%D0%BC%D0%B8%D0%B7%D0%B0%D1%86%D0%B8%D1%8F
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
        double c_real = real_min + rand_r(&rand_state) / ((double)RAND_MAX) *
                                       (real_max - real_min);
        double c_img = img_min + rand_r(&rand_state) / ((double)RAND_MAX) *
                                     (img_max - img_min);
        double real_part = 0;
        double img_part = 0;

        if (check_main_cardiod(c_real, c_img)) {
            int index = my_args->start + i;
            COMPLEX* c = &global_points_buf[index];
            c->real = c_real;
            c->img = c_img;
            ++i;
            continue;
        }
        for (long j = 0; j < iterations_to_include; ++j) {
            double r = real_part * real_part - img_part * img_part + c_real;
            double i = 2 * real_part * img_part + c_img;
            real_part = r;
            img_part = i;

            if (pow(real_part, 2) + pow(img_part, 2) >= 4) {
                break;
            }
        }

        if (real_part * real_part + img_part * img_part < 4) {
            int index = my_args->start + i;
            COMPLEX* c = &global_points_buf[index];
            c->real = c_real;
            c->img = c_img;
            ++i;
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
        ARGS* args =
            new_args(i * points_per_thread, (i + 1) * points_per_thread);
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
    if (argc != 3) {
        printf("Использование: %s <количество потоков> <количество попыток>\n",
               argv[0]);
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
