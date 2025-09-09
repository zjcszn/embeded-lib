/* power_fit_lm_mcu.c
 *
 * MCU 友好版本（改进版）：
 * - 全局 b 粗搜索 + 自适应细化 + 改进的 Brent 精细化
 * - LM refine（缓存 exp，中心化 lnx）
 * - 不使用 malloc，所有缓冲区栈分配
 * 拟合模型：f(x) = a * x^b + c
 */

#include <math.h>
#include <stdio.h>
#include <float.h>

#define MAX_N                       (16)
#define B_SEARCH_MIN                (-20.0)
#define B_SEARCH_MAX                (20.0)
#define B_CLAMP_MIN                 (-100.0)
#define B_CLAMP_MAX                 (100.0)
#define EPS_DET                     (1e-30)
#define LM_MAX_IT                   (2000)
#define GOLDEN_RATIO                (0.618033988749895)
#define TOL_BRENT                   (1e-8)
#define MAX_IT_BRENT                (100)
#define TOL_3POINT                  (1e-12)
#define COARSE_SEARCH_POINTS        (21) // 初始粗采样点数
#define COARSE_REFINE_POINTS        (11) // 在最小点邻域的细化采样点数

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

/* ------------------ 3x3 线性方程求解（带主元） ------------------ */
static SolveStatus Solve3x3(double A[3][3], double b[3], double x[3])
{
    for (int i = 0; i < 3; ++i) {
        int pivot = i;
        double max_val = fabs(A[i][i]);

        for (int j = i + 1; j < 3; ++j) {
            double val = fabs(A[j][i]);
            if (val > max_val) {
                max_val = val;
                pivot = j;
            }
        }

        if (max_val < EPS_DET) { return SOLVE_ERROR_SINGULAR; }

        if (pivot != i) {
            for (int k = i; k < 3; ++k) {
                double tmp = A[i][k];
                A[i][k] = A[pivot][k];
                A[pivot][k] = tmp;
            }
            double tmp_b = b[i];
            b[i] = b[pivot];
            b[pivot] = tmp_b;
        }

        double diag = A[i][i];
        for (int j = i + 1; j < 3; ++j) {
            double factor = A[j][i] / diag;
            for (int k = i; k < 3; ++k) { A[j][k] -= factor * A[i][k]; }
            b[j] -= factor * b[i];
        }
    }

    for (int i = 2; i >= 0; --i) {
        double s = b[i];
        for (int j = i + 1; j < 3; ++j) { s -= A[i][j] * x[j]; }

        if (fabs(A[i][i]) < EPS_DET) { return SOLVE_ERROR_SINGULAR; }

        x[i] = s / A[i][i];
    }

    return SOLVE_OK;
}

/* ------------------ SSE 计算（使用中心化 lnx） ------------------ */
static double SsePowerWithCenteredLnx(const double* lnx_center, const double* x,
                                      const double* y, int n, double a_local,
                                      double b, double c)
{
    double sse = 0.0;
    for (int i = 0; i < n; ++i) {
        double xb = exp(b * lnx_center[i]); // 已减均值，数值更稳定
        if (!isfinite(xb)) { return INFINITY; }

        double fi = a_local * xb + c;
        if (!isfinite(fi)) { return INFINITY; }

        double residual = y[i] - fi;
        sse += residual * residual;
        if (!isfinite(sse)) { return INFINITY; } // 溢出检测
    }
    return sse;
}

/* ------------------ 计算总平方和 SST ------------------ */
static double CalculateSst(const double* y, int n)
{
    if (n <= 0) { return 0.0; }

    double mean = 0.0;
    for (int i = 0; i < n; ++i) { mean += y[i]; }
    mean /= n;

    double sst = 0.0;
    for (int i = 0; i < n; ++i) {
        double deviation = y[i] - mean;
        sst += deviation * deviation;
    }

    return sst;
}

/* ------------------ 计算 R² ------------------ */
static double CalculateRSquared(double sse, double sst)
{
    if (sst == 0.0) {
        return 1.0;
    }
    return 1.0 - (sse / sst);
}

/* ------------------ 固定 b，解析求 a_local,c（使用已中心化 lnx） ------------------ */
/* u_buf: 用于缓存 exp(b * lnx_center[i])，避免重复计算 */
static double SseGivenBCenteredLnx(const double* lnx_center, const double* x,
                                  const double* y, int n, double b,
                                  double* u_buf, double* a_local_out,
                                  double* c_out)
{
    if (!isfinite(b)) { return INFINITY; }

    if (b < B_CLAMP_MIN) { b = B_CLAMP_MIN; }
    if (b > B_CLAMP_MAX) { b = B_CLAMP_MAX; }

    double sum_uu = 0.0, sum_u = 0.0, sum_uy = 0.0, sum_y = 0.0;

    for (int i = 0; i < n; ++i) {
        double ui = exp(b * lnx_center[i]);
        if (!isfinite(ui)) { return INFINITY; }

        u_buf[i] = ui;
        sum_uu += ui * ui;
        sum_u += ui;
        sum_uy += ui * y[i];
        sum_y += y[i];
    }

    double det = sum_uu * n - sum_u * sum_u;
    if (fabs(det) < EPS_DET) { return INFINITY; }

    double a_local = (sum_uy * n - sum_u * sum_y) / det;
    double c = (sum_uu * sum_y - sum_u * sum_uy) / det;

    double sse = 0.0;
    for (int i = 0; i < n; ++i) {
        double residual = y[i] - (a_local * u_buf[i] + c);
        sse += residual * residual;
        if (!isfinite(sse)) { return INFINITY; }
    }

    if (a_local_out != NULL) { *a_local_out = a_local; }
    if (c_out != NULL) { *c_out = c; }

    return sse;
}

/* ------------------ 改进的 Brent（三点实现：x,w,v） ------------------ */
static void FindBestBByBrent(const double* x, const double* y,
                             const double* lnx_center, int n, double b_min,
                             double b_max, double* best_b, double* best_a_local,
                             double* best_c, double* best_sse)
{
    double u_buf[MAX_N];
    int iterator = 0;

    // 第一阶段：粗采样（均匀）
    double coarse_best = b_min;
    double coarse_val = INFINITY;
    double step = (b_max - b_min) / (COARSE_SEARCH_POINTS - 1);

    for (int i = 0; i < COARSE_SEARCH_POINTS; ++i) {
        double bb = b_min + i * step;
        double val = SseGivenBCenteredLnx(lnx_center, x, y, n, bb, u_buf, NULL, NULL);
        if (val < coarse_val) {
            coarse_val = val;
            coarse_best = bb;
        }
    }

    // 第二阶段：在 coarse_best 邻域再细化采样（自适应半宽）
    double half_width = step * 2.0;
    double refine_left = coarse_best - half_width;
    double refine_right = coarse_best + half_width;
    if (refine_left < b_min) refine_left = b_min;
    if (refine_right > b_max) refine_right = b_max;

    double refine_step = (refine_right - refine_left) / (COARSE_REFINE_POINTS - 1);
    for (int i = 0; i < COARSE_REFINE_POINTS; ++i) {
        double bb = refine_left + i * refine_step;
        double val = SseGivenBCenteredLnx(lnx_center, x, y, n, bb, u_buf, NULL, NULL);
        if (val < coarse_val) {
            coarse_val = val;
            coarse_best = bb;
        }
    }

    // 现在以 coarse_best 为初始 c，准备执行标准 Brent（三点）
    double a = b_min;
    double b = b_max;
    double xk = coarse_best; // x
    double w = xk;           // w
    double v = xk;           // v
    double fx = SseGivenBCenteredLnx(lnx_center, x, y, n, xk, u_buf, NULL, NULL);
    double fw = fx;
    double fv = fx;

    double d = 0.0, e = 0.0;
    for (iterator = 0; iterator < MAX_IT_BRENT; ++iterator) {
        double xm = 0.5 * (a + b);
        double tol1 = TOL_BRENT * fabs(xk) + 1e-10;
        double tol2 = 2.0 * tol1;

        // 终止条件
        if (fabs(xk - xm) <= (tol2 - 0.5 * (b - a))) { break; }

        double p = 0.0, q = 0.0, r = 0.0;
        if (fabs(e) > tol1) {
            // 拟合抛物线插值（基于 xk,w,v）
            r = (xk - w) * (fx - fv);
            q = (xk - v) * (fx - fw);
            p = (xk - v) * q - (xk - w) * r;
            q = 2.0 * (q - r);
            if (q > 0.0) p = -p;
            q = fabs(q);
            double min1 = e;
            if (fabs(p) < fabs(0.5 * q * min1) && p > q * (a - xk) && p < q * (b - xk)) {
                d = p / q;
                double u = xk + d;
                if (u - a < tol2 || b - u < tol2) {
                    d = (xm - xk >= 0) ? tol1 : -tol1;
                }
            } else {
                e = (xk >= xm) ? (a - xk) : (b - xk);
                d = GOLDEN_RATIO * e;
            }
        } else {
            e = (xk >= xm) ? (a - xk) : (b - xk);
            d = GOLDEN_RATIO * e;
        }

        double u = (fabs(d) >= tol1) ? xk + d : xk + ((d > 0) ? tol1 : -tol1);
        double fu = SseGivenBCenteredLnx(lnx_center, x, y, n, u, u_buf, NULL, NULL);

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

    LOG_PRINTF("find best b by brent, iterator count = %d\r\n", iterator);

    double a_local = 0.0, c_local = 0.0;
    double ssebest = SseGivenBCenteredLnx(lnx_center, x, y, n, xk, u_buf, &a_local, &c_local);

    *best_b = xk;
    *best_a_local = a_local;
    *best_c = c_local;
    *best_sse = ssebest;
}

/* ------------------ n=3 精确拟合（使用中心化 lnx） ------------------ */
/* 保持和原来一样，但使用 lnx_center（即 ln(x)-mean）以提高数值稳定性 */
static double FFor3Centered(double b, double l1, double l2, double l3, double r)
{
    double u = exp(b * l1);
    double v = exp(b * l2);
    double w = exp(b * l3);

    if (!isfinite(u) || !isfinite(v) || !isfinite(w)) { return NAN; }

    double denom = u - w;
    if (fabs(denom) < 1e-300) { return NAN; }

    return (u - v) / denom - r;
}

static FitStatus SolvePower3TryCentered(const double* x, const double* y, const double* lnx_center,
                                        const double lnx_mean, double* a_local, double* b, double* c)
{
    const double x1 = x[0], x2 = x[1], x3 = x[2];
    const double y1 = y[0], y2 = y[1], y3 = y[2];

    if (fabs(y1 - y2) < 1e-15 && fabs(y1 - y3) < 1e-15) {
        *a_local = 0.0;
        *b = 1.0;
        *c = y1;
        return FIT_OK;
    }

    double r = (y1 - y2) / (y1 - y3);
    double l1 = lnx_center[0], l2 = lnx_center[1], l3 = lnx_center[2];

    const double samples[] = {
        -50, -20, -10, -5, -2, -1, -0.5, -0.25, 0, 0.25,
        0.5, 1, 2, 5, 10, 20, 50
    };

    int ns = sizeof(samples) / sizeof(samples[0]);
    int br_l = -1, br_r = -1;
    double fl = 0.0, fr = 0.0;

    for (int i = 0; i < ns - 1; ++i) {
        double flt = FFor3Centered(samples[i], l1, l2, l3, r);
        double frt = FFor3Centered(samples[i + 1], l1, l2, l3, r);

        if (!isfinite(flt) || !isfinite(frt)) { continue; }

        if (flt * frt < 0.0) {
            br_l = i;
            br_r = i + 1;
            fl = flt;
            fr = frt;
            break;
        }
    }

    if (br_l == -1) { return FIT_ERROR_NO_SOLUTION; }

    double left = samples[br_l], right = samples[br_r];
    int iterator;

    for (iterator = 0; iterator < 200; ++iterator) {
        double mid = 0.5 * (left + right);
        double fmid = FFor3Centered(mid, l1, l2, l3, r);

        if (!isfinite(fmid)) {
            left += 0.25 * (mid - left);
            right -= 0.25 * (right - mid);
            continue;
        }

        if (fabs(fmid) < 1e-14) {
            left = right = mid;
            break;
        }

        if (fl * fmid <= 0.0) {
            right = mid;
            fr = fmid;
        }
        else {
            left = mid;
            fl = fmid;
        }

        if (fabs(right - left) < TOL_3POINT * (1 + fabs(left))) { break; }
    }

    LOG_PRINTF("solve power 3 point, iterator count: %d\r\n", iterator);
    double bb = 0.5 * (left + right);
    double u = pow(x1, bb);
    double v = pow(x2, bb);

    if (fabs(u - v) < 1e-300) { return FIT_ERROR_SINGULAR; }

    double aa = (y1 - y2) / (u - v);
    double cc = y1 - aa * u;

    // 注意：这里求得的是原始 a（适用于原始 x），需要把它转换成 center 表示 a_local：
    // 我们用中心化表示，最终要使用 a_local such that:
    // a_local * exp(b * (lnx - mean)) = a_orig * exp(b * lnx)
    // => a_local = a_orig * exp(b * mean)
    double a_local_val = aa * exp(bb * lnx_mean); // 将 a_orig 转成 a_local（以便保持内部一致）
    *a_local = a_local_val;
    *b = bb;
    *c = cc;

    return FIT_OK;
}

/* ------------------ 主拟合函数 ------------------ */
FitStatus FitPowerLM(const double* x, const double* y, int n, int max_iterator,
                     double tol, double* a_out, double* b_out, double* c_out,
                     double* r_squared_out, int* n_iterator_out)
{
    if (x == NULL || y == NULL || a_out == NULL ||
        b_out == NULL || c_out == NULL || n <= 0 || n > MAX_N) { return FIT_ERROR_INVALID_ARG; }

    if (n_iterator_out != NULL) { *n_iterator_out = 0; }

    double lnx[MAX_N];
    for (int i = 0; i < n; ++i) {
        if (x[i] <= 0.0) { return FIT_ERROR_INVALID_X; }
        lnx[i] = log(x[i]);
    }

    // 计算 lnx 的均值并做中心化，降低数值范围
    double lnx_mean = 0.0;
    for (int i = 0; i < n; ++i) lnx_mean += lnx[i];
    lnx_mean /= n;

    double lnx_center[MAX_N];
    for (int i = 0; i < n; ++i) lnx_center[i] = lnx[i] - lnx_mean;

    // 计算总平方和 SST
    double sst = CalculateSst(y, n);

    // 快速分支判定，线性回归: y = kx+b（作为特殊情况）
    double sum_x = 0.0, sum_y = 0.0, sum_xx = 0.0, sum_xy = 0.0;
    for (int i = 0; i < n; i++) {
        sum_x += x[i];
        sum_y += y[i];
        sum_xx += x[i] * x[i];
        sum_xy += x[i] * y[i];
    }

    double denom = sum_xx * n - sum_x * sum_x;
    if (fabs(denom) > 1e-24) {
        double k = (n * sum_xy - sum_x * sum_y) / denom;
        double c0 = (sum_y - k * sum_x) / n;
        double sse_lin = 0.0;

        for (int i = 0; i < n; i++) {
            double residual = y[i] - (k * x[i] + c0);
            sse_lin += residual * residual;
        }

        // 放宽阈值以避免遗漏明显的线性情形
        if (sse_lin / (sst + 1e-30) < 1e-8 || sse_lin < 1e-16) {
            *a_out = k;
            *b_out = 1.0;
            *c_out = c0;
            if (r_squared_out != NULL) { *r_squared_out = CalculateRSquared(sse_lin, sst); }
            return FIT_OK;
        }
    }

    /* 三点精准解（使用中心化 lnx） */
    if (n == 3) {
        double a_local, bb, cc;
        FitStatus status = SolvePower3TryCentered(x, y, lnx_center, lnx_mean, &a_local, &bb, &cc);
        if (status == FIT_OK) {
            // 将局部 a_local 转换回原始尺度 a_orig = a_local * exp(-b * lnx_mean)
            double a_orig = a_local * exp(-bb * lnx_mean);
            *a_out = a_orig;
            *b_out = bb;
            *c_out = cc;
            if (r_squared_out != NULL) { *r_squared_out = 1.0; }
            return FIT_OK;
        }
    }

    double A_local0, B0, C0, S0;
    FindBestBByBrent(x, y, lnx_center, n, B_SEARCH_MIN, B_SEARCH_MAX,
                     &B0, &A_local0, &C0, &S0);

    if (!isfinite(B0) || !isfinite(A_local0) || !isfinite(C0)) { return FIT_ERROR_OVERFLOW; }

    double A_local = A_local0, B = B0, C = C0;
    double lambda = 1e-3;
    double prev_sse = SsePowerWithCenteredLnx(lnx_center, x, y, n, A_local, B, C);
    if (!isfinite(prev_sse)) return FIT_ERROR_OVERFLOW;

    int it = 0;
    int converged = 0;

    double u_buf[MAX_N];

    for (it = 0; it < max_iterator; ++it) {
        // 在每次迭代开始时缓存 u_buf = exp(B * lnx_center[i])
        int valid_count = 1;
        for (int i = 0; i < n; ++i) {
            double ui = exp(B * lnx_center[i]);
            if (!isfinite(ui)) { valid_count = 0; break; }
            u_buf[i] = ui;
        }
        if (!valid_count) {
            // 如果 u_buf 无效，缩小步长并继续
            lambda *= 2.0;
            if (lambda > 1e20) break;
            continue;
        }

        double JTJ[3][3] = {{0.0}}, JTr[3] = {0.0};

        for (int i = 0; i < n; ++i) {
            double xb = u_buf[i]; // 已缓存
            double fi = A_local * xb + C;
            double residual = y[i] - fi;
            double Ja = xb;
            double Jb = A_local * xb * lnx_center[i];
            double Jc = 1.0;

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

        double d0 = (fabs(JTJ[0][0]) > 0) ? fabs(JTJ[0][0]) : 1.0;
        double d1 = (fabs(JTJ[1][1]) > 0) ? fabs(JTJ[1][1]) : 1.0;
        double d2 = (fabs(JTJ[2][2]) > 0) ? fabs(JTJ[2][2]) : 1.0;

        // 加上阻尼项
        JTJ[0][0] += lambda * d0;
        JTJ[1][1] += lambda * d1;
        JTJ[2][2] += lambda * d2;

        double delta[3] = {0.0};
        double Acpy[3][3];
        for (int r = 0; r < 3; r++) for (int c = 0; c < 3; c++) Acpy[r][c] = JTJ[r][c];

        SolveStatus solve_status = Solve3x3(Acpy, JTr, delta);
        if (solve_status != SOLVE_OK) {
            lambda *= 2.0;
            if (lambda > 1e20) { break; }
            else { continue; }
        }

        double A_new_local = A_local + delta[0];
        double B_new = B + delta[1];
        double C_new = C + delta[2];

        if (!isfinite(B_new)) { B_new = B; }

        if (B_new < B_CLAMP_MIN) { B_new = B_CLAMP_MIN; }
        if (B_new > B_CLAMP_MAX) { B_new = B_CLAMP_MAX; }

        // 计算 sse_new：注意要使用中心化 lnx
        double sse_new = SsePowerWithCenteredLnx(lnx_center, x, y, n, A_new_local, B_new, C_new);
        if (!isfinite(sse_new)) {
            lambda *= 2.0;
            if (lambda > 1e20) break;
            continue;
        }

        double step_norm = sqrt(delta[0] * delta[0] +
            delta[1] * delta[1] +
            delta[2] * delta[2]);
        double rel_sse = fabs(prev_sse - sse_new) / (1.0 + prev_sse);

        if (sse_new < prev_sse) {
            // 接受更新，且降低 lambda（更平滑）
            A_local = A_new_local;
            B = B_new;
            C = C_new;
            prev_sse = sse_new;

            if (rel_sse < 1e-9 || step_norm < tol) {
                converged = 1;
                it++;
                break;
            }

            lambda *= 0.7;
            if (lambda < 1e-20) lambda = 1e-20;
        } else {
            // 未改善，增加 lambda
            lambda *= 2.0;
            if (lambda > 1e20) { break; }
        }
    }

    // 将本地 a_local 转换回原始尺度 a_orig = a_local * exp(-B * lnx_mean)
    double a_final = A_local * exp(-B * lnx_mean);
    *a_out = a_final;
    *b_out = B;
    *c_out = C;

    if (r_squared_out != NULL) { *r_squared_out = CalculateRSquared(prev_sse, sst); }
    if (n_iterator_out != NULL) { *n_iterator_out = it; }

    return converged ? FIT_OK : FIT_PARTIAL;
}

/* ------------------ main 测试 ------------------ */
int main(void)
{
    double a, b, c, r_squared;
    int iterators;
    FitStatus ret;

    LOG_PRINTF("================= Fitting Power Test ================= \n");


    /* Dataset1 */
    const double x1[] = {300.0, 600.0, 2000.0};
    const double y1[] = {300.0, 700.0, 2000.0};

    LOG_PRINTF("Dataset.1\n");
    ret = FitPowerLM(x1, y1, sizeof(x1) / sizeof(x1[0]), LM_MAX_IT, 1e-12, &a, &b, &c, &r_squared, &iterators);
    LOG_PRINTF("\tret=%d, f(x) = %.12g * x ^ %.12g + (%.12g),  R^2 = %.12g, iteratorator count = %d\n", ret, a, b, c, r_squared, iterators);

    for (int i = 0; i < sizeof(x1) / sizeof(x1[0]); i++) {
        double fi = a * pow(x1[i], b) + c;
        LOG_PRINTF("\t\tx = %.6f, y = %.6f, f(x) = %.12f, r = %.12f\n", x1[i], y1[i], fi, y1[i] - fi);
    }

    LOG_PRINTF("\r\n");
    /* Dataset2 */
    const double x2[] = {300.0, 600.0, 800.0, 2000.0};
    const double y2[] = {-0.005904406213615, -0.002741179358446, -0.002156558016386, -0.000645620903348};

    LOG_PRINTF("Dataset.2\n");
    ret = FitPowerLM(x2, y2, sizeof(x2) / sizeof(x2[0]), LM_MAX_IT, 1e-12, &a, &b, &c, &r_squared, &iterators);
    LOG_PRINTF("\tret=%d, f(x) = %.12g * x ^ %.12g + (%.12g),  R^2 = %.12g, iteratorator count = %d\n", ret, a, b, c, r_squared, iterators);

    for (int i = 0; i < sizeof(x2) / sizeof(x2[0]); i++) {
        double fi = a * pow(x2[i], b) + c;
        LOG_PRINTF("\t\tx = %.6f, y = %.6f, f(x) = %.12f, r = %.12f\n", x2[i], y2[i], fi, y2[i] - fi);
    }

    /* Dataset3 */
    const double x3[] = {300.0, 600.0, 2000.0};
    const double y3[] = {300.0, 600.0, 2000.0};

    LOG_PRINTF("Dataset.3\n");
    ret = FitPowerLM(x3, y3, sizeof(x3) / sizeof(x3[0]), LM_MAX_IT, 1e-12, &a, &b, &c, &r_squared, &iterators);
    LOG_PRINTF("\tret=%d, f(x) = %.12g * x ^ %.12g + (%.12g),  R^2 = %.12g, iteratorator count = %d\n", ret, a, b, c, r_squared, iterators);

    for (int i = 0; i < sizeof(x3) / sizeof(x3[0]); i++) {
        double fi = a * pow(x3[i], b) + c;
        LOG_PRINTF("\t\tx = %.6f, y = %.6f, f(x) = %.12f, r = %.12f\n", x3[i], y3[i], fi, y3[i] - fi);
    }


    LOG_PRINTF("======================= End ======================= \n");


    return 0;
}
