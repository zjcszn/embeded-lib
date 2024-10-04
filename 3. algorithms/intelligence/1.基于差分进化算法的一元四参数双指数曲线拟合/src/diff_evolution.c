#include "diff_evolution.h"
#include "stdint.h"
#include "stdlib.h"
#include "multi_task.h"
#include "math.h"
#include "string.h"
#include "log.h"
#include "float.h"
#include "main.h"

static population_t X[MAX_NP], V[MAX_NP], U[MAX_NP];
static float_t fitnessX[MAX_NP], fitnessU[MAX_NP];


/**
 * @brief 种群初始化
 * 
 * @param bound 解空间边界：MAX MIN
 * @param NP 种群大小
 * @param dim 维度
 */
void diffevo_initialiazation(bound_t *bound, uint16_t NP, uint16_t dim) {
    /* 初始化种群和适应度buffer */
    memset(X, 0, sizeof(X));
    memset(V, 0, sizeof(V));
    memset(U, 0, sizeof(U));
    memset(fitnessX, 0, sizeof(fitnessX));
    memset(fitnessU, 0, sizeof(fitnessU));

    /* 初始化随机数种子 */
    srand(Systick->VAL);

    for (int i = 0; i < NP; i++) {
        for (int j = 0; j < dim; j++) {
            X[i].raw[j] = bound[j].min + (bound[j].max - bound[j].min) * ((float)rand() / (float)RAND_MAX);
        }
    }
}


/**
 * @brief 计算适应度
 * 
 * @param P 种群
 * @param NP 种群大小
 * @param fitness 适应度[数组]
 * @param dataset 样本数据集
 * @param n 数据集大小
 * @return uint16_t 最佳适应度个体的序号
 */
uint16_t diffevo_update_fitness(population_t *P, uint16_t NP, float *fitness, dataset_t *dataset, uint16_t n) {
    float sse,  error;
    float sse_min = FLT_MAX;
    int best_idx;

    for (int i = 0; i < NP; i++) {
        /* 计算残差平方和 */
        sse = 0;
        for (int j = 0; j < n; j++) {
            error = P[i].raw[0] * expf(P[i].raw[1] * dataset[j].x) + P[i].raw[2] * expf(P[i].raw[3] * dataset[j].x) - dataset[j].y;
            sse += error * error;
        }
        fitness[i] = sse;


        if (sse < sse_min) {
            sse_min = sse;
            best_idx = i;
        }
    }

    return best_idx;
}


/**
 * @brief 变异
 * 
 * @param NP 种群大小
 * @param dim 维度
 * @param F 缩放因子
 * @param best_x 目标种群中适应度最高的个体序号
 * @param bound 解空间边界
 * @param mutation_mode 变异策略
 */
void diffevo_mutation(uint16_t NP, uint16_t dim, float F, uint16_t best_x, bound_t *bound, mutation_mode_t mutation_mode) {
    uint8_t r[5], tmp, is_equal;
    float tmpf;

    for (int i = 0; i < NP; i++) {

        /* 取5个不重复且不等于 i 的随机数，用于扰动变异 */
        for (int j = 0; j < 5; j++) {
            do {
                is_equal = 0;
                tmp = rand() % NP;

                if (tmp == i) {
                    continue;
                }
                
                for (int k = 0; k < j; k++) {
                    if (tmp == r[k]) {
                        is_equal = 1;
                        break;        
                    } 
                }
            } while (is_equal || (tmp == i));

            r[j] = tmp;
        }

        /* 变异 */
        switch (mutation_mode) {
            case DE_BEST_1:
                for (int k = 0; k < dim; k++) {
                    V[i].raw[k] = X[best_x].raw[k] + F * (X[r[0]].raw[k] - X[r[1]].raw[k]);
                }
                break;

            case DE_BEST_2:
                for (int k = 0; k < dim; k++) {
                    V[i].raw[k] = X[best_x].raw[k] + F * (X[r[0]].raw[k] - X[r[1]].raw[k]) + F * (X[r[2]].raw[k] - X[r[3]].raw[k]);
                    
                    /* 限定解空间边界会导致拟合效果较差，无法收敛，TODO */
                    #if 0
                    tmpf = X[best_x].raw[k] + F * (X[r[0]].raw[k] - X[r[1]].raw[k]) + F * (X[r[2]].raw[k] - X[r[3]].raw[k]);
                    if ((tmpf > bound[k].max) || (tmpf < bound[k].min)) {
                        tmpf = bound[k].min + ((float)rand() / (float)RAND_MAX) * (bound[k].max - bound[k].min);
                    }
                    V[i].raw[k] = tmpf;
                    #endif
                }
                break;

            case DE_RAND_1:
                for (int k = 0; k < dim; k++) {
                    V[i].raw[k] = X[r[0]].raw[k] + F * (X[r[1]].raw[k] - X[r[2]].raw[k]);
                }
                break;

            case DE_RAND_2:
                for (int k = 0; k < dim; k++) {
                    V[i].raw[k] = X[r[0]].raw[k] + F * (X[r[1]].raw[k] + X[r[2]].raw[k] - X[r[3]].raw[k] - X[r[4]].raw[k]);
                }
                break;

            case DE_TARGET_TO_BEST_1:
                for (int k = 0; k < dim; k++) {
                    V[i].raw[k] = X[i].raw[k] + F * (X[best_x].raw[k] - X[i].raw[k]) + F * (X[r[0]].raw[k] - X[r[1]].raw[k]);
                }
                break;
            
            default:
                break;
        }
    }
}

/**
 * @brief 交叉
 * 
 * @param NP 种群大小
 * @param dim 维度
 * @param CR 交叉概率
 * @param crossover_mode 交叉策略
 */
void diffevo_crossover(uint16_t NP, uint16_t dim, float CR, crossover_mode_t crossover_mode) {
    int j_rand, cross_flag, L;
    float cr_x;

    switch (crossover_mode) {

        case CROSSOVER_BIN:
            for (int i = 0; i < NP; i++) {
                /* 保证至少有一个维度的变异向量可传递给子代 */
                j_rand = rand() % dim;  
                for (int j = 0; j < dim; j++) {
                    cr_x = (float)rand() / (float)RAND_MAX;
                    if (cr_x < CR || j == j_rand) {
                        U[i].raw[j] = V[i].raw[j];
                    } else {
                        U[i].raw[j] = X[i].raw[j];
                    }
                }
            }
            break;

        case CROSSOVER_EXP:
            /* 未验证 */
            for (int i = 0; i < NP; i++) {
                /* 交叉起始点 */
                L = 0;
                j_rand = rand() % dim;  
                cross_flag = 1;

                for (int j = 0; j < dim; j++) {
                    if (cross_flag) {
                        U[i].raw[j_rand] = V[i].raw[j_rand];

                        cr_x = (float)rand() / (float)RAND_MAX;
                        if ((cr_x > CR) || (++L > dim)) {
                            cross_flag = 0;
                        }
                    } else {
                        U[i].raw[j_rand] = X[i].raw[j_rand];
                    }

                    if (++j_rand >= dim) {
                        j_rand = 0;
                    }
                }
            }
            break;

        default:
            break;
    }
}

/**
 * @brief 选择
 * 
 * @param fitnessX 目标种群适应度值
 * @param fitnessU 试验种群适应度值
 * @param NP 种群大小
 * @param dim 维度
 * @param bestX 适应度最高的个体序号
 */
void diffevo_select(float *fitnessX, float *fitnessU, uint16_t NP, uint16_t dim, uint16_t *bestX) {
    for (int i = 0; i < NP; i++) {

        if (fitnessU[i] <= fitnessX[i]) {   
            for (int j = 0; j < dim; j++) {
                X[i].raw[j] = U[i].raw[j];
            }
            fitnessX[i] = fitnessU[i];
            
            if (fitnessX[i] < fitnessX[*bestX]) {
                *bestX = i;
            }
        }
    }
}

/**
 * @brief 基于差分进化算法的一元四参数双指数曲线拟合
 * 
 * @note 模型：f(x) = a1 * exp(b1 * x) + a2 * exp(b2 * x);
 * 
 * @param result 拟合结果，参数解（a1, b1, a2, b2）
 * @param result_fitness 残差平方和
 * @param bound 参数空间边界
 * @param data 样本数据集
 * @param n 样本大小
 * @param param 算法参数
 * @return int 返回1表示拟合结果符合预期， 返回0表示不符合预期
 */
int diffevo_curve_fitting(population_t *result, float *sse, bound_t *bound, dataset_t *data, uint16_t n, diffevo_params_t *param){
    uint16_t dim, bestX, gen_idx;
    int result_ok = 0;

    /* 初始化种群 */
    dim = sizeof(population_t) / sizeof(float);
    diffevo_initialiazation(bound, param->NP, dim);
    
    /* 计算适应度 */
    bestX = diffevo_update_fitness(X, param->NP, fitnessX, data, n);

    for (gen_idx = 0; gen_idx < param->max_times; gen_idx++) {

        if (fitnessX[bestX] < param->target_fitness) {
            result_ok = 1;
            break;
        }

        /* 变异 */
        diffevo_mutation(param->NP, dim, param->F, bestX, bound, param->mutation_mode);

        /* 交叉 */
        diffevo_crossover(param->NP, dim, param->F, param->crossover_mode);

        /* 计算子代个体适应度 */
        diffevo_update_fitness(U, param->NP, fitnessU, data, n);

        /* 选择 */
        diffevo_select(fitnessX, fitnessU, param->NP, dim, &bestX);
    }

    memcpy(result, &X[bestX], sizeof(population_t));
    *result_fitness = fitnessX[bestX];

    LOG("curve fitting %s, times: %u\r\n", result_ok ? "success", "failed", gen_idx);
    LOG("f(x) = %.4f * exp (%.4f * x) + %.4f * exp (%.4f * x)", X[bestX].raw[0], X[bestX].raw[1], X[bestX].raw[2], X[bestX].raw[3]);

    return result_ok;
}

