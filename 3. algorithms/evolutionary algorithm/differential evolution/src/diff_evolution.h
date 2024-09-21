#ifndef __DIFF_EVOLUTION_H__
#define __DIFF_EVOLUTION_H__

#include "stdint.h"

#define MAX_NP       40


typedef union {
    struct {
        float a1;
        float b1;
        float a2;
        float b2;
    } data;

    float raw[4];

} population_t;

typedef struct {
    float max;
    float min;
} bound_t;

typedef struct {
    float x;
    float y;
} dataset_t;

typedef enum {
    DE_RAND_1,
    DE_BEST_1,
    DE_RAND_TO_BEST_1,
    DE_BEST_2,
    DE_RAND_2,
} mutation_mode_t;

typedef enum {
    CROSSOVER_BIN,  /* 二项式交叉 */
    CROSSOVER_EXP,  /* 指数交叉 */
} crossover_mode_t;

typedef struct {
    uint16_t NP;
    float CR;
    float F;
    uint16_t max_times;
    mutation_mode_t mutation_mode;
    crossover_mode_t crossover_mode;
    float target_fitness;
} diffevo_params_t;

int diffevo_curve_fitting(population_t *result, float *sse, bound_t *bound, dataset_t *data, uint16_t n, diffevo_params_t *param);



#endif