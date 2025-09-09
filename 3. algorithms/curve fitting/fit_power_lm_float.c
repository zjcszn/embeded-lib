/* power_fit_lm_mcu_float.c
 *
 * MCU 友好版本（float）：全部 double -> float
 * - 全局 b 粗搜索 + 自适应细化 + 改进的 Brent 精细化
 * - LM refine（缓存 exp，中心化 lnx）
 * - 不使用 malloc，所有缓冲区栈分配
 * 拟合模型：f(x) = a * x^b + c
 */

#include <math.h>
#include <stdio.h>
#include <float.h>

#define MAX_N                       (16)
#define B_SEARCH_MIN_F              (-20.0f)
#define B_SEARCH_MAX_F              (20.0f)
#define B_CLAMP_MIN_F               (-100.0f)
#define B_CLAMP_MAX_F               (100.0f)
#define EPS_DET_F                   (1e-20f)      // float 容差
#define LM_MAX_IT                   (2000)
#define GOLDEN_RATIO_F              (0.618033988749895f)
#define TOL_BRENT_F                 (1e-6f)
#define MAX_IT_BRENT                (100)
#define TOL_3POINT_F                (1e-7f)
#define COARSE_SEARCH_POINTS        (21) // 初始粗采样点数
#define COARSE_REFINE_POINTS        (11) // 在最小点邻域的细化采样点数
#define INF_F                      (HUGE_VALF)

/* 日志输出控制 */
#define LOG_ENABLED 1
#if LOG_ENABLED
#define LOG_PRINTF(...) printf(__VA_ARGS__)
#else
#define LOG_PRINTF(...) ((void)0)
#endif

/* ------------------ 拟合返回状态 ------------------ */
typedef enum
{
    FIT_OK = 0, // 拟合成功
    FIT_PARTIAL = 1, // 达到最大迭代但未收敛
    FIT_ERROR_INVALID_ARG = -1, // 参数非法
    FIT_ERROR_INVALID_X = -2, // x[i] <= 0，不合法
    FIT_ERROR_SINGULAR = -3, // 矩阵求解奇异
    FIT_ERROR_NO_SOLUTION = -4, // 无解（解析拟合失败）
    FIT_ERROR_OVERFLOW = -5 // 数值溢出
} FitStatus;

/* ------------------ 线性方程求解状态 ------------------ */
typedef enum
{
    SOLVE_OK = 0, // 求解成功
    SOLVE_ERROR_SINGULAR = -1, // 矩阵奇异
    SOLVE_ERROR_INVALID = -2 // 无效输入
} SolveStatus;

/* ------------------ 3x3 线性方程求解（带主元） - float 版 ------------------ */
static SolveStatus Solve3x3_f(float A[3][3], float b[3], float x[3])
{
    for (int i = 0; i < 3; ++i) {
        int pivot = i;
        float max_val = fabsf(A[i][i]);

        for (int j = i + 1; j < 3; ++j) {
            float val = fabsf(A[j][i]);
            if (val > max_val) {
                max_val = val;
                pivot = j;
            }
        }

        if (max_val < EPS_DET_F) { return SOLVE_ERROR_SINGULAR; }

        if (pivot != i) {
            for (int k = i; k < 3; ++k) {
                float tmp = A[i][k];
                A[i][k] = A[pivot][k];
                A[pivot][k] = tmp;
            }
            float tmp_b = b[i];
            b[i] = b[pivot];
            b[pivot] = tmp_b;
        }

        float diag = A[i][i];
        for (int j = i + 1; j < 3; ++j) {
            float factor = A[j][i] / diag;
            for (int k = i; k < 3; ++k) { A[j][k] -= factor * A[i][k]; }
            b[j] -= factor * b[i];
        }
    }

    for (int i = 2; i >= 0; --i) {
        float s = b[i];
        for (int j = i + 1; j < 3; ++j) { s -= A[i][j] * x[j]; }

        if (fabsf(A[i][i]) < EPS_DET_F) { return SOLVE_ERROR_SINGULAR; }

        x[i] = s / A[i][i];
    }

    return SOLVE_OK;
}

/* ------------------ SSE 计算（使用中心化 lnx，float 版） ------------------ */
static float SsePowerWithCenteredLnx_f(const float* lnx_center, const float* x,
                                       const float* y, int n, float a_local,
                                       float b, float c)
{
    float sse = 0.0f;
    for (int i = 0; i < n; ++i) {
        float xb = expf(b * lnx_center[i]); // 已减均值，数值更稳定
        if (!isfinite(xb)) { return INF_F; }

        float fi = a_local * xb + c;
        if (!isfinite(fi)) { return INF_F; }

        float residual = y[i] - fi;
        sse += residual * residual;
        if (!isfinite(sse)) { return INF_F; } // 溢出检测
    }
    return sse;
}

/* ------------------ 计算总平方和 SST（float） ------------------ */
static float CalculateSst_f(const float* y, int n)
{
    if (n <= 0) { return 0.0f; }

    float mean = 0.0f;
    for (int i = 0; i < n; ++i) { mean += y[i]; }
    mean /= (float)n;

    float sst = 0.0f;
    for (int i = 0; i < n; ++i) {
        float deviation = y[i] - mean;
        sst += deviation * deviation;
    }

    return sst;
}

/* ------------------ 计算 R²（float） ------------------ */
static float CalculateRSquared_f(float sse, float sst)
{
    if (sst == 0.0f) {
        return 1.0f;
    }
    return 1.0f - (sse / sst);
}

/* ------------------ 固定 b，解析求 a_local,c（使用已中心化 lnx） float 版 ------------------ */
/* u_buf: 用于缓存 exp(b * lnx_center[i])，避免重复计算 */
static float SseGivenBCenteredLnx_f(const float* lnx_center, const float* x,
                                    const float* y, int n, float b,
                                    float* u_buf, float* a_local_out,
                                    float* c_out)
{
    if (!isfinite(b)) { return INF_F; }

    if (b < B_CLAMP_MIN_F) { b = B_CLAMP_MIN_F; }
    if (b > B_CLAMP_MAX_F) { b = B_CLAMP_MAX_F; }

    float sum_uu = 0.0f, sum_u = 0.0f, sum_uy = 0.0f, sum_y = 0.0f;

    for (int i = 0; i < n; ++i) {
        float ui = expf(b * lnx_center[i]);
        if (!isfinite(ui)) { return INF_F; }

        u_buf[i] = ui;
        sum_uu += ui * ui;
        sum_u += ui;
        sum_uy += ui * y[i];
        sum_y += y[i];
    }

    float det = sum_uu * (float)n - sum_u * sum_u;
    if (fabsf(det) < EPS_DET_F) { return INF_F; }

    float a_local = (sum_uy * (float)n - sum_u * sum_y) / det;
    float c = (sum_uu * sum_y - sum_u * sum_uy) / det;

    float sse = 0.0f;
    for (int i = 0; i < n; ++i) {
        float residual = y[i] - (a_local * u_buf[i] + c);
        sse += residual * residual;
        if (!isfinite(sse)) { return INF_F; }
    }

    if (a_local_out != NULL) { *a_local_out = a_local; }
    if (c_out != NULL) { *c_out = c; }

    return sse;
}

/* ------------------ 改进的 Brent（三点实现：x,w,v） float 版 ------------------ */
static void FindBestBByBrent_f(const float* x, const float* y,
                               const float* lnx_center, int n, float b_min,
                               float b_max, float* best_b, float* best_a_local,
                               float* best_c, float* best_sse)
{
    float u_buf[MAX_N];
    int iterator = 0;

    // 第一阶段：粗采样（均匀）
    float coarse_best = b_min;
    float coarse_val = INF_F;
    float step = (b_max - b_min) / (float)(COARSE_SEARCH_POINTS - 1);

    for (int i = 0; i < COARSE_SEARCH_POINTS; ++i) {
        float bb = b_min + i * step;
        float val = SseGivenBCenteredLnx_f(lnx_center, x, y, n, bb, u_buf, NULL, NULL);
        if (val < coarse_val) {
            coarse_val = val;
            coarse_best = bb;
        }
    }

    // 第二阶段：在 coarse_best 邻域再细化采样（自适应半宽）
    float half_width = step * 2.0f;
    float refine_left = coarse_best - half_width;
    float refine_right = coarse_best + half_width;
    if (refine_left < b_min) refine_left = b_min;
    if (refine_right > b_max) refine_right = b_max;

    float refine_step = (refine_right - refine_left) / (float)(COARSE_REFINE_POINTS - 1);
    for (int i = 0; i < COARSE_REFINE_POINTS; ++i) {
        float bb = refine_left + i * refine_step;
        float val = SseGivenBCenteredLnx_f(lnx_center, x, y, n, bb, u_buf, NULL, NULL);
        if (val < coarse_val) {
            coarse_val = val;
            coarse_best = bb;
        }
    }

    // 现在以 coarse_best 为初始 xk，准备执行标准 Brent（三点）
    float a = b_min;
    float b = b_max;
    float xk = coarse_best; // x
    float w = xk;           // w
    float v = xk;           // v
    float fx = SseGivenBCenteredLnx_f(lnx_center, x, y, n, xk, u_buf, NULL, NULL);
    float fw = fx;
    float fv = fx;

    float d = 0.0f, e = 0.0f;
    for (iterator = 0; iterator < MAX_IT_BRENT; ++iterator) {
        float xm = 0.5f * (a + b);
        float tol1 = TOL_BRENT_F * fabsf(xk) + 1e-7f;
        float tol2 = 2.0f * tol1;

        // 终止条件
        if (fabsf(xk - xm) <= (tol2 - 0.5f * (b - a))) { break; }

        float p = 0.0f, q = 0.0f, r = 0.0f;
        if (fabsf(e) > tol1) {
            // 拟合抛物线插值（基于 xk,w,v）
            r = (xk - w) * (fx - fv);
            q = (xk - v) * (fx - fw);
            p = (xk - v) * q - (xk - w) * r;
            q = 2.0f * (q - r);
            if (q > 0.0f) p = -p;
            q = fabsf(q);
            float min1 = e;
            if (fabsf(p) < fabsf(0.5f * q * min1) && p > q * (a - xk) && p < q * (b - xk)) {
                d = p / q;
                float u = xk + d;
                if (u - a < tol2 || b - u < tol2) {
                    d = (xm - xk >= 0.0f) ? tol1 : -tol1;
                }
            } else {
                e = (xk >= xm) ? (a - xk) : (b - xk);
                d = GOLDEN_RATIO_F * e;
            }
        } else {
            e = (xk >= xm) ? (a - xk) : (b - xk);
            d = GOLDEN_RATIO_F * e;
        }

        float u = (fabsf(d) >= tol1) ? xk + d : xk + ((d > 0.0f) ? tol1 : -tol1);
        float fu = SseGivenBCenteredLnx_f(lnx_center, x, y, n, u, u_buf, NULL, NULL);

        if (fu <= fx) {
            if (u >= xk) a = xk; else b = xk;
            v = w; fv = fw;
            w = xk; fw = fx;
            xk = u; fx = fu;
        } else {
            if (u < xk) a = u; else b = u;
            if (fu <= fw || w == xk) {
                v = w; fv = fw;
                w = u; fw = fu;
            } else if (fu <= fv || v == xk || v == w) {
                v = u; fv = fu;
            }
        }
    }

    LOG_PRINTF("find best b by brent (float), iterator count = %d\r\n", iterator);

    float a_local = 0.0f, c_local = 0.0f;
    float ssebest = SseGivenBCenteredLnx_f(lnx_center, x, y, n, xk, u_buf, &a_local, &c_local);

    *best_b = xk;
    *best_a_local = a_local;
    *best_c = c_local;
    *best_sse = ssebest;
}

/* ------------------ n=3 精确拟合（使用中心化 lnx） float 版 ------------------ */
static float FFor3Centered_f(float b, float l1, float l2, float l3, float r)
{
    float u = expf(b * l1);
    float v = expf(b * l2);
    float w = expf(b * l3);

    if (!isfinite(u) || !isfinite(v) || !isfinite(w)) { return NAN; }

    float denom = u - w;
    if (fabsf(denom) < 1e-30f) { return NAN; }

    return (u - v) / denom - r;
}

static FitStatus SolvePower3TryCentered_f(const float* x, const float* y, const float* lnx_center,
                                          const float lnx_mean, float* a_local, float* b, float* c)
{
    const float x1 = x[0], x2 = x[1], x3 = x[2];
    const float y1 = y[0], y2 = y[1], y3 = y[2];

    if (fabsf(y1 - y2) < 1e-7f && fabsf(y1 - y3) < 1e-7f) {
        *a_local = 0.0f;
        *b = 1.0f;
        *c = y1;
        return FIT_OK;
    }

    float r = (y1 - y2) / (y1 - y3);
    float l1 = lnx_center[0], l2 = lnx_center[1], l3 = lnx_center[2];

    const float samples[] = {
        -50.0f, -20.0f, -10.0f, -5.0f, -2.0f, -1.0f, -0.5f, -0.25f, 0.0f, 0.25f,
         0.5f, 1.0f, 2.0f, 5.0f, 10.0f, 20.0f, 50.0f
    };

    int ns = (int)(sizeof(samples) / sizeof(samples[0]));
    int br_l = -1, br_r = -1;
    float fl = 0.0f, fr = 0.0f;

    for (int i = 0; i < ns - 1; ++i) {
        float flt = FFor3Centered_f(samples[i], l1, l2, l3, r);
        float frt = FFor3Centered_f(samples[i + 1], l1, l2, l3, r);

        if (!isfinite(flt) || !isfinite(frt)) { continue; }

        if (flt * frt < 0.0f) {
            br_l = i;
            br_r = i + 1;
            fl = flt;
            fr = frt;
            break;
        }
    }

    if (br_l == -1) { return FIT_ERROR_NO_SOLUTION; }

    float left = samples[br_l], right = samples[br_r];
    int iterator;

    for (iterator = 0; iterator < 200; ++iterator) {
        float mid = 0.5f * (left + right);
        float fmid = FFor3Centered_f(mid, l1, l2, l3, r);

        if (!isfinite(fmid)) {
            left += 0.25f * (mid - left);
            right -= 0.25f * (right - mid);
            continue;
        }

        if (fabsf(fmid) < 1e-10f) {
            left = right = mid;
            break;
        }

        if (fl * fmid <= 0.0f) {
            right = mid;
            fr = fmid;
        }
        else {
            left = mid;
            fl = fmid;
        }

        if (fabsf(right - left) < TOL_3POINT_F * (1.0f + fabsf(left))) { break; }
    }

    LOG_PRINTF("solve power 3 point (float), iterator count: %d\r\n", iterator);
    float bb = 0.5f * (left + right);
    float u = powf(x1, bb);
    float v = powf(x2, bb);

    if (fabsf(u - v) < 1e-30f) { return FIT_ERROR_SINGULAR; }

    float aa = (y1 - y2) / (u - v);
    float cc = y1 - aa * u;

    float a_local_val = aa * expf(bb * lnx_mean); // 将 a_orig 转成 a_local（以便保持内部一致）
    *a_local = a_local_val;
    *b = bb;
    *c = cc;

    return FIT_OK;
}

/* ------------------ 主拟合函数（float 版） ------------------ */
FitStatus FitPowerLM_f(const float* x, const float* y, int n, int max_iterator,
                       float tol, float* a_out, float* b_out, float* c_out,
                       float* r_squared_out, int* n_iterator_out)
{
    if (x == NULL || y == NULL || a_out == NULL ||
        b_out == NULL || c_out == NULL || n <= 0 || n > MAX_N) { return FIT_ERROR_INVALID_ARG; }

    if (n_iterator_out != NULL) { *n_iterator_out = 0; }

    float lnx[MAX_N];
    for (int i = 0; i < n; ++i) {
        if (x[i] <= 0.0f) { return FIT_ERROR_INVALID_X; }
        lnx[i] = logf(x[i]);
    }

    // 计算 lnx 的均值并做中心化，降低数值范围
    float lnx_mean = 0.0f;
    for (int i = 0; i < n; ++i) lnx_mean += lnx[i];
    lnx_mean /= (float)n;

    float lnx_center[MAX_N];
    for (int i = 0; i < n; ++i) lnx_center[i] = lnx[i] - lnx_mean;

    // 计算总平方和 SST
    float sst = CalculateSst_f(y, n);

    // 快速分支判定，线性回归: y = kx+b（作为特殊情况）
    float sum_x = 0.0f, sum_y = 0.0f, sum_xx = 0.0f, sum_xy = 0.0f;
    for (int i = 0; i < n; i++) {
        sum_x += x[i];
        sum_y += y[i];
        sum_xx += x[i] * x[i];
        sum_xy += x[i] * y[i];
    }

    float denom = sum_xx * (float)n - sum_x * sum_x;
    if (fabsf(denom) > 1e-12f) {
        float k = ((float)n * sum_xy - sum_x * sum_y) / denom;
        float c0 = (sum_y - k * sum_x) / (float)n;
        float sse_lin = 0.0f;

        for (int i = 0; i < n; i++) {
            float residual = y[i] - (k * x[i] + c0);
            sse_lin += residual * residual;
        }

        // 放宽阈值以避免遗漏明显的线性情形
        if (sse_lin / (sst + 1e-30f) < 1e-6f || sse_lin < 1e-12f) {
            *a_out = k;
            *b_out = 1.0f;
            *c_out = c0;
            if (r_squared_out != NULL) { *r_squared_out = CalculateRSquared_f(sse_lin, sst); }
            return FIT_OK;
        }
    }

    /* 三点精准解（使用中心化 lnx） */
    if (n == 3) {
        float a_local, bb, cc;
        FitStatus status = SolvePower3TryCentered_f(x, y, lnx_center, lnx_mean, &a_local, &bb, &cc);
        if (status == FIT_OK) {
            // 将局部 a_local 转换回原始尺度 a_orig = a_local * exp(-b * lnx_mean)
            float a_orig = a_local * expf(-bb * lnx_mean);
            *a_out = a_orig;
            *b_out = bb;
            *c_out = cc;
            if (r_squared_out != NULL) { *r_squared_out = 1.0f; }
            return FIT_OK;
        }
    }

    float A_local0, B0, C0, S0;
    FindBestBByBrent_f(x, y, lnx_center, n, B_SEARCH_MIN_F, B_SEARCH_MAX_F,
                       &B0, &A_local0, &C0, &S0);

    if (!isfinite(B0) || !isfinite(A_local0) || !isfinite(C0)) { return FIT_ERROR_OVERFLOW; }

    float A_local = A_local0, B = B0, C = C0;
    float lambda = 1e-3f;
    float prev_sse = SsePowerWithCenteredLnx_f(lnx_center, x, y, n, A_local, B, C);
    if (!isfinite(prev_sse)) return FIT_ERROR_OVERFLOW;

    int it = 0;
    int converged = 0;

    float u_buf[MAX_N];

    for (it = 0; it < max_iterator; ++it) {
        // 在每次迭代开始时缓存 u_buf = exp(B * lnx_center[i])
        int valid_count = 1;
        for (int i = 0; i < n; ++i) {
            float ui = expf(B * lnx_center[i]);
            if (!isfinite(ui)) { valid_count = 0; break; }
            u_buf[i] = ui;
        }
        if (!valid_count) {
            // 如果 u_buf 无效，缩小步长并继续
            lambda *= 2.0f;
            if (lambda > 1e20f) break;
            continue;
        }

        float JTJ[3][3] = {{0.0f}}, JTr[3] = {0.0f};

        for (int i = 0; i < n; ++i) {
            float xb = u_buf[i]; // 已缓存
            float fi = A_local * xb + C;
            float residual = y[i] - fi;
            float Ja = xb;
            float Jb = A_local * xb * lnx_center[i];
            float Jc = 1.0f;

            JTJ[0][0] += Ja * Ja;
            JTJ[0][1] += Ja * Jb;
            JTJ[0][2] += Ja * Jc;
            JTJ[1][0] += Jb * Ja;
            JTJ[1][1] += Jb * Jb;
            JTJ[1][2] += Jb * Jc;
            JTJ[2][0] += Jc * Ja;
            JTJ[2][1] += Jc * Jb;
            JTJ[2][2] += Jc * Jc;

            JTr[0] += Ja * residual;
            JTr[1] += Jb * residual;
            JTr[2] += Jc * residual;
        }

        float d0 = (fabsf(JTJ[0][0]) > 0.0f) ? fabsf(JTJ[0][0]) : 1.0f;
        float d1 = (fabsf(JTJ[1][1]) > 0.0f) ? fabsf(JTJ[1][1]) : 1.0f;
        float d2 = (fabsf(JTJ[2][2]) > 0.0f) ? fabsf(JTJ[2][2]) : 1.0f;

        // 加上阻尼项
        JTJ[0][0] += lambda * d0;
        JTJ[1][1] += lambda * d1;
        JTJ[2][2] += lambda * d2;

        float delta[3] = {0.0f};
        float Acpy[3][3];
        for (int r = 0; r < 3; r++) for (int c = 0; c < 3; c++) Acpy[r][c] = JTJ[r][c];

        SolveStatus solve_status = Solve3x3_f(Acpy, JTr, delta);
        if (solve_status != SOLVE_OK) {
            lambda *= 2.0f;
            if (lambda > 1e20f) { break; }
            else { continue; }
        }

        float A_new_local = A_local + delta[0];
        float B_new = B + delta[1];
        float C_new = C + delta[2];

        if (!isfinite(B_new)) { B_new = B; }

        if (B_new < B_CLAMP_MIN_F) { B_new = B_CLAMP_MIN_F; }
        if (B_new > B_CLAMP_MAX_F) { B_new = B_CLAMP_MAX_F; }

        // 计算 sse_new：注意要使用中心化 lnx
        float sse_new = SsePowerWithCenteredLnx_f(lnx_center, x, y, n, A_new_local, B_new, C_new);
        if (!isfinite(sse_new)) {
            lambda *= 2.0f;
            if (lambda > 1e20f) break;
            continue;
        }

        float step_norm = sqrtf(delta[0] * delta[0] +
            delta[1] * delta[1] +
            delta[2] * delta[2]);
        float rel_sse = fabsf(prev_sse - sse_new) / (1.0f + prev_sse);

        if (sse_new < prev_sse) {
            // 接受更新，且降低 lambda（更平滑）
            A_local = A_new_local;
            B = B_new;
            C = C_new;
            prev_sse = sse_new;

            if (rel_sse < 1e-7f || step_norm < tol) {
                converged = 1;
                it++;
                break;
            }

            lambda *= 0.7f;
            if (lambda < 1e-20f) lambda = 1e-20f;
        } else {
            // 未改善，增加 lambda
            lambda *= 2.0f;
            if (lambda > 1e20f) { break; }
        }
    }

    // 将本地 a_local 转换回原始尺度 a_orig = a_local * exp(-B * lnx_mean)
    float a_final = A_local * expf(-B * lnx_mean);
    *a_out = a_final;
    *b_out = B;
    *c_out = C;

    if (r_squared_out != NULL) { *r_squared_out = CalculateRSquared_f(prev_sse, sst); }
    if (n_iterator_out != NULL) { *n_iterator_out = it; }

    return converged ? FIT_OK : FIT_PARTIAL;
}

/* ------------------ main 测试（float 版） ------------------ */
int main(void)
{
    float a, b, c, r_squared;
    int iterators;
    FitStatus ret;

    LOG_PRINTF("================= Fitting Power Test (float) ================= \n");

    /* Dataset1 */
    const float x1[] = {300.0f, 600.0f, 2000.0f};
    const float y1[] = {300.0f, 700.0f, 2000.0f};

    LOG_PRINTF("Dataset.1\n");
    ret = FitPowerLM_f(x1, y1, (int)(sizeof(x1) / sizeof(x1[0])), LM_MAX_IT, 1e-6f, &a, &b, &c, &r_squared, &iterators);
    LOG_PRINTF("\tret=%d, f(x) = %.7g * x ^ %.7g + (%.7g),  R^2 = %.7g, iterator count = %d\n", ret, a, b, c, r_squared, iterators);

    for (int i = 0; i < (int)(sizeof(x1) / sizeof(x1[0])); i++) {
        float fi = a * powf(x1[i], b) + c;
        LOG_PRINTF("\t\tx = %.6f, y = %.6f, f(x) = %.9f, r = %.9f\n", x1[i], y1[i], fi, y1[i] - fi);
    }

    LOG_PRINTF("\r\n");
    /* Dataset2 */
    const float x2[] = {300.0f, 600.0f, 800.0f, 2000.0f};
    const float y2[] = {-0.005904406213615f, -0.002741179358446f, -0.002156558016386f, -0.000645620903348f};

    LOG_PRINTF("Dataset.2\n");
    ret = FitPowerLM_f(x2, y2, (int)(sizeof(x2) / sizeof(x2[0])), LM_MAX_IT, 1e-6f, &a, &b, &c, &r_squared, &iterators);
    LOG_PRINTF("\tret=%d, f(x) = %.7g * x ^ %.7g + (%.7g),  R^2 = %.7g, iterator count = %d\n", ret, a, b, c, r_squared, iterators);

    for (int i = 0; i < (int)(sizeof(x2) / sizeof(x2[0])); i++) {
        float fi = a * powf(x2[i], b) + c;
        LOG_PRINTF("\t\tx = %.6f, y = %.6f, f(x) = %.9f, r = %.9f\n", x2[i], y2[i], fi, y2[i] - fi);
    }

    /* Dataset3 */
    const float x3[] = {300.0f, 600.0f, 2000.0f};
    const float y3[] = {300.0f, 600.0f, 2000.0f};

    LOG_PRINTF("Dataset.3\n");
    ret = FitPowerLM_f(x3, y3, (int)(sizeof(x3) / sizeof(x3[0])), LM_MAX_IT, 1e-6f, &a, &b, &c, &r_squared, &iterators);
    LOG_PRINTF("\tret=%d, f(x) = %.7g * x ^ %.7g + (%.7g),  R^2 = %.7g, iterator count = %d\n", ret, a, b, c, r_squared, iterators);

    for (int i = 0; i < (int)(sizeof(x3) / sizeof(x3[0])); i++) {
        float fi = a * powf(x3[i], b) + c;
        LOG_PRINTF("\t\tx = %.6f, y = %.6f, f(x) = %.9f, r = %.9f\n", x3[i], y3[i], fi, y3[i] - fi);
    }

    LOG_PRINTF("======================= End ======================= \n");

    return 0;
}
