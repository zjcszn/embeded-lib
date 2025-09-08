/* power_fit_lm_mcu.c
 *
 * MCU 友好版本：全局 b 搜索 + Brent 精细化 + LM refine
 * 不使用 malloc，所有缓冲区栈分配
 * 拟合模型：f(x) = a * x^b + c
 */

#include <math.h>
#include <stdio.h>
#include <float.h>

#define MAX_N           ( 16)
#define B_SEARCH_MIN    (-20.0)
#define B_SEARCH_MAX    ( 20.0)
#define B_CLAMP_MIN     (-100.0)
#define B_CLAMP_MAX     ( 100.0)
#define EPS_DET         ( 1e-30)
#define LM_MAX_IT       ( 2000)
#define GOLDEN_RATIO    ( 0.618033988749895)

/* ------------------ 拟合返回状态 ------------------ */
typedef enum
{
    FIT_OK = 0, // 拟合成功
    FIT_PARTIAL = 1, // 达到最大迭代但未收敛
    FIT_ERROR_INVALID_ARG = -1, // 参数非法
    FIT_ERROR_NONPOS_X = -2, // x[i] <= 0，不合法
    FIT_ERROR_SINGULAR = -3, // 矩阵求解奇异
    FIT_ERROR_NO_SOLUTION = -4, // 无解（解析拟合失败）
    FIT_ERROR_OVERFLOW = -5 // 数值溢出
} FitStatus;

/* ------------------ 3x3 线性方程求解 ------------------ */
static int Solve3x3(double A[3][3], double b[3], double x[3])
{
    int i, j, k, pivot;
    for (i = 0; i < 3; ++i) {
        pivot = i;
        double maxv = fabs(A[i][i]);
        for (j = i + 1; j < 3; ++j) {
            double v = fabs(A[j][i]);
            if (v > maxv) {
                maxv = v;
                pivot = j;
            }
        }
        if (maxv < EPS_DET) return -1; // 奇异矩阵
        if (pivot != i) {
            for (k = i; k < 3; ++k) {
                double tmp = A[i][k];
                A[i][k] = A[pivot][k];
                A[pivot][k] = tmp;
            }
            double tb = b[i];
            b[i] = b[pivot];
            b[pivot] = tb;
        }
        double diag = A[i][i];
        for (j = i + 1; j < 3; ++j) {
            double factor = A[j][i] / diag;
            for (k = i; k < 3; ++k) A[j][k] -= factor * A[i][k];
            b[j] -= factor * b[i];
        }
    }
    for (i = 2; i >= 0; --i) {
        double s = b[i];
        for (j = i + 1; j < 3; ++j) s -= A[i][j] * x[j];
        if (fabs(A[i][i]) < EPS_DET) return -2;
        x[i] = s / A[i][i];
    }
    return 0;
}

/* ------------------ SSE 计算 ------------------ */
static double SsePowerWithLnx(const double* lnx, const double* x,
                              const double* y, int n, double a, double b,
                              double c)
{
    double s = 0.0;
    for (int i = 0; i < n; ++i) {
        double xb = exp(b * lnx[i]);
        if (!isfinite(xb)) return INFINITY;
        double fi = a * xb + c;
        if (!isfinite(fi)) return INFINITY;
        double r = y[i] - fi;
        s += r * r;
    }
    return s;
}

/* ------------------ 固定 b，解析求 a,c ------------------ */
static double SseGivenBWithLnxs(const double* lnx, const double* x,
                                const double* y, int n, double b,
                                double* u_buf, double* a_out,
                                double* c_out)
{
    if (!isfinite(b)) return INFINITY;
    if (b < B_CLAMP_MIN) b = B_CLAMP_MIN;
    if (b > B_CLAMP_MAX) b = B_CLAMP_MAX;

    double Suu = 0.0, Su = 0.0, Suy = 0.0, Sy = 0.0;
    for (int i = 0; i < n; ++i) {
        double ui = exp(b * lnx[i]);
        if (!isfinite(ui)) return INFINITY;
        u_buf[i] = ui;
        Suu += ui * ui;
        Su += ui;
        Suy += ui * y[i];
        Sy += y[i];
    }
    double det = Suu * n - Su * Su;
    if (fabs(det) < EPS_DET) return INFINITY;

    double a = (Suy * n - Su * Sy) / det;
    double c = (Suu * Sy - Su * Suy) / det;

    double sse = 0.0;
    for (int i = 0; i < n; ++i) {
        double r = y[i] - (a * u_buf[i] + c);
        sse += r * r;
    }
    if (a_out) *a_out = a;
    if (c_out) *c_out = c;
    return sse;
}

/* ------------------ Brent 方法搜索 b ------------------ */
static void FindBestBByBrent(const double* x, const double* y,
                             const double* lnx, int n, double bmin,
                             double bmax, double* best_b, double* best_a,
                             double* best_c, double* best_sse)
{
    double u_buf[MAX_N];
    int iter;

    double coarse_best = 0.0, coarse_val = INFINITY;
    for (double bb = bmin; bb <= bmax; bb += 1.0) {
        double val = SseGivenBWithLnxs(lnx, x, y, n, bb, u_buf, NULL, NULL);
        if (val < coarse_val) {
            coarse_val = val;
            coarse_best = bb;
        }
    }

    double a = bmin, b = bmax;
    double c = coarse_best, fc = coarse_val;
    double d = 0.0, e = 0.0;
    const double tol = 1e-8;
    const int maxit = 100;

    double fa = SseGivenBWithLnxs(lnx, x, y, n, a, u_buf, NULL, NULL);
    double fb = SseGivenBWithLnxs(lnx, x, y, n, b, u_buf, NULL, NULL);

    for (iter = 0; iter < maxit; ++iter) {
        double xm = 0.5 * (a + b);
        double tol1 = tol * fabs(c) + 1e-10;
        double tol2 = 2.0 * tol1;
        if (fabs(c - xm) <= tol2 - 0.5 * (b - a)) break;

        double p = 0.0, q = 0.0, r = 0.0;
        if (fabs(e) > tol1) {
            r = (c - d) * (fc - fb);
            q = (c - b) * (fc - fa);
            p = (c - b) * q - (c - d) * r;
            q = 2.0 * (q - r);
            if (q > 0) p = -p;
            if (fabs(p) < fabs(0.5 * q * e) && p > q * (a - c) && p < q * (b - c)) {
                d = p / q;
                double u = c + d;
                if (u - a < tol2 || b - u < tol2) d = (xm - c >= 0 ? tol1 : -tol1);
            }
            else {
                e = (c >= xm ? a - c : b - c);
                d = GOLDEN_RATIO * e;
            }
        }
        else {
            e = (c >= xm ? a - c : b - c);
            d = GOLDEN_RATIO * e;
        }

        double u = (fabs(d) >= tol1 ? c + d : c + (d >= 0 ? tol1 : -tol1));
        double fu = SseGivenBWithLnxs(lnx, x, y, n, u, u_buf, NULL, NULL);

        if (fu <= fc) {
            if (u >= c) a = c;
            else b = c;
            fa = fc;
            d = e;
            e = c;
            c = u;
            fc = fu;
        }
        else {
            if (u < c) a = u;
            else b = u;
        }
    }

    printf("find best b  by brent, iter count = %d\r\n", iter);

    double abest = 0.0, cbest = 0.0;
    double ssebest = SseGivenBWithLnxs(lnx, x, y, n, c, u_buf, &abest, &cbest);
    *best_b = c;
    *best_a = abest;
    *best_c = cbest;
    *best_sse = ssebest;
}

/* ------------------ n=3 精确拟合 ------------------ */
static double FFor3(double b, double lnx1, double lnx2, double lnx3, double r)
{
    double u = exp(b * lnx1);
    double v = exp(b * lnx2);
    double w = exp(b * lnx3);
    double denom = u - w;
    if (!isfinite(u) || !isfinite(v) || !isfinite(w) || fabs(denom) < 1e-300) return NAN;
    return (u - v) / denom - r;
}

static int SolvePower3Try(const double* x, const double* y, const double* lnx,
                          double* a, double* b, double* c)
{
    int iter;
    const double x1 = x[0], x2 = x[1], x3 = x[2];
    const double y1 = y[0], y2 = y[1], y3 = y[2];
    if (fabs(y1 - y2) < 1e-15 && fabs(y1 - y3) < 1e-15) {
        *a = 0.0;
        *b = 1.0;
        *c = y1;
        return 0;
    }

    double r = (y1 - y2) / (y1 - y3);
    double lnx1_ = lnx[0], lnx2_ = lnx[1], lnx3_ = lnx[2];
    const double samples[] = {
        -50, -20, -10, -5, -2, -1, -0.5, -0.25, 0, 0.25,
        0.5, 1, 2, 5, 10, 20, 50
    };
    int ns = sizeof(samples) / sizeof(samples[0]);
    int br_l = -1, br_r = -1;
    double fl = 0.0, fr = 0.0;

    for (int i = 0; i < ns - 1; ++i) {
        double flt = FFor3(samples[i], lnx1_, lnx2_, lnx3_, r);
        double frt = FFor3(samples[i + 1], lnx1_, lnx2_, lnx3_, r);
        if (!isfinite(flt) || !isfinite(frt)) continue;
        if (flt * frt < 0.0) {
            br_l = i;
            br_r = i + 1;
            fl = flt;
            fr = frt;
            break;
        }
    }
    if (br_l == -1) return -1;

    double left = samples[br_l], right = samples[br_r];
    for (iter = 0; iter < 200; ++iter) {
        double mid = 0.5 * (left + right);
        double fmid = FFor3(mid, lnx1_, lnx2_, lnx3_, r);
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
        if (fabs(right - left) < 1e-12 * (1 + fabs(left))) break;
    }

    printf("solve power 3 point, iter count: %d\r\n", iter);
    double bb = 0.5 * (left + right);
    double u = pow(x1, bb), v = pow(x2, bb);
    double aa = (y1 - y2) / (u - v);
    double cc = y1 - aa * u;
    *a = aa;
    *b = bb;
    *c = cc;
    return 0;
}

/* ------------------ 主拟合函数 ------------------ */
FitStatus FitPowerLM(const double* x, const double* y, int n, int max_iter,
                     double tol, double* a_out, double* b_out, double* c_out,
                     double* sse_out, int* n_iter_out)
{
    if (!x || !y || !a_out || !b_out || !c_out || n <= 0 || n > MAX_N) return FIT_ERROR_INVALID_ARG;
    if (n_iter_out) *n_iter_out = 0;

    double lnx[MAX_N];
    for (int i = 0; i < n; ++i) {
        if (x[i] <= 0.0) return FIT_ERROR_NONPOS_X;
        lnx[i] = log(x[i]);
    }

    if (n == 3) {
        double aa, bb, cc;
        if (SolvePower3Try(x, y, lnx, &aa, &bb, &cc) == 0) {
            *a_out = aa;
            *b_out = bb;
            *c_out = cc;
            if (sse_out) *sse_out = 0.0;
            return FIT_OK;
        }
    }

    double A0, B0, C0, S0;
    FindBestBByBrent(x, y, lnx, n, B_SEARCH_MIN, B_SEARCH_MAX, &B0, &A0, &C0, &S0);
    if (!isfinite(B0) || !isfinite(A0) || !isfinite(C0)) return FIT_ERROR_OVERFLOW;

    double A = A0, B = B0, C = C0;
    double lambda = 1e-3;
    double prev_sse = SsePowerWithLnx(lnx, x, y, n, A, B, C);
    int it = 0, converged = 0;

    for (it = 0; it < max_iter; ++it) {
        double JTJ[3][3] = {{0.0}}, JTr[3] = {0.0};
        for (int i = 0; i < n; ++i) {
            double xb = exp(B * lnx[i]);
            if (!isfinite(xb)) continue;
            double fi = A * xb + C;
            double ri = y[i] - fi;
            double Ja = xb, Jb = A * xb * lnx[i], Jc = 1.0;
            JTJ[0][0] += Ja * Ja;
            JTJ[0][1] += Ja * Jb;
            JTJ[0][2] += Ja * Jc;
            JTJ[1][0] += Jb * Ja;
            JTJ[1][1] += Jb * Jb;
            JTJ[1][2] += Jb * Jc;
            JTJ[2][0] += Jc * Ja;
            JTJ[2][1] += Jc * Jb;
            JTJ[2][2] += Jc * Jc;
            JTr[0] += Ja * ri;
            JTr[1] += Jb * ri;
            JTr[2] += Jc * ri;
        }

        double d0 = fabs(JTJ[0][0]) > 0 ? fabs(JTJ[0][0]) : 1.0;
        double d1 = fabs(JTJ[1][1]) > 0 ? fabs(JTJ[1][1]) : 1.0;
        double d2 = fabs(JTJ[2][2]) > 0 ? fabs(JTJ[2][2]) : 1.0;
        JTJ[0][0] += lambda * d0;
        JTJ[1][1] += lambda * d1;
        JTJ[2][2] += lambda * d2;

        double delta[3] = {0}, Acpy[3][3];
        for (int r = 0; r < 3; r++) for (int c = 0; c < 3; c++) Acpy[r][c] = JTJ[r][c];

        if (Solve3x3(Acpy, JTr, delta) != 0) {
            lambda *= 10;
            if (lambda > 1e20) break;
            else continue;
        }

        double A_new = A + delta[0], B_new = B + delta[1], C_new = C + delta[2];
        if (!isfinite(B_new)) B_new = B;
        if (B_new < B_CLAMP_MIN) B_new = B_CLAMP_MIN;
        if (B_new > B_CLAMP_MAX) B_new = B_CLAMP_MAX;

        double sse_new = SsePowerWithLnx(lnx, x, y, n, A_new, B_new, C_new);
        double step_norm = sqrt(delta[0] * delta[0] + delta[1] * delta[1] + delta[2] * delta[2]);
        double rel_sse = fabs(prev_sse - sse_new) / (1 + prev_sse);

        if (isfinite(sse_new) && sse_new < prev_sse) {
            A = A_new;
            B = B_new;
            C = C_new;
            prev_sse = sse_new;
            if (rel_sse < 1e-12 || step_norm < tol || rel_sse < 1e-9) {
                converged = 1;
                it++;
                break;
            }
            lambda *= 0.3;
            if (lambda < 1e-20) lambda = 1e-20;
        }
        else {
            lambda *= 10;
            if (lambda > 1e20) break;
        }
    }

    *a_out = A;
    *b_out = B;
    *c_out = C;
    if (sse_out)*sse_out = prev_sse;
    if (n_iter_out)*n_iter_out = it;
    return converged ? FIT_OK : FIT_PARTIAL;
}

/* ------------------ main 测试 ------------------ */
int main(void)
{
    double a, b, c, sse;
    int iters;
    FitStatus ret;

    /* Dataset1 */
    const double x1[] = {300.0, 600.0, 2000.0};
    const double y1[] = {300, 700, 2000};

    printf("Dataset1\n");
    ret = FitPowerLM(x1, y1, 3,LM_MAX_IT, 1e-12, &a, &b, &c, &sse, &iters);
    printf("  ret=%d, a=%.12g, b=%.12g, c=%.12g, SSE=%.12g, iters=%d\n",
           ret, a, b, c, sse, iters);
    for (int i = 0; i < sizeof(x1) / sizeof(x1[0]); i++) {
        double fi = a * pow(x1[i], b) + c;
        printf("    x=%.0f, y=%.12f, f=%.12f, r=%.12f\n",
               x1[i], y1[i], fi, y1[i] - fi);
    }

    /* Dataset2 */
    const double x2[] = {300.0, 600.0, 800.0, 2000.0};
    const double y2[] = {
        -0.005904406213615, -0.002741179358446,
        -0.002156558016386, -0.000645620903348
    };

    printf("Dataset2:\n");
    ret = FitPowerLM(x2, y2, 4,LM_MAX_IT, 1e-12, &a, &b, &c, &sse, &iters);
    printf("  ret=%d, a=%.12g, b=%.12g, c=%.12g, SSE=%.12g, iters=%d\n",
           ret, a, b, c, sse, iters);
    for (int i = 0; i < 4; i++) {
        double fi = a * pow(x2[i], b) + c;
        printf("    x=%.0f, y=%.12f, f=%.12f, r=%.12f\n",
               x2[i], y2[i], fi, y2[i] - fi);
    }
    return 0;
}
