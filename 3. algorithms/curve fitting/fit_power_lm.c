/* PowerFitLM_full.c
 *
 * 完整实现：包含全局 b 搜索 + 黄金分割精化 + LM 微调
 *
 * 编译：
 *   gcc -O2 -std=c11 PowerFitLM_full.c -lm -o powerfit
 * 运行：
 *   ./powerfit
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

/* ------------------ 基本工具：3x3 高斯消元（带列主元） ------------------ */
static int solve3x3(double A[3][3], double b[3], double x[3])
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
        if (maxv < 1e-30) return -1;
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
            if (factor == 0.0) continue;
            for (k = i; k < 3; ++k) A[j][k] -= factor * A[i][k];
            b[j] -= factor * b[i];
        }
    }
    for (i = 2; i >= 0; --i) {
        double s = b[i];
        for (j = i + 1; j < 3; ++j) s -= A[i][j] * x[j];
        if (fabs(A[i][i]) < 1e-30) return -2;
        x[i] = s / A[i][i];
    }
    return 0;
}

/* ------------------ SSE（标准计算，用于 LM 精化） ------------------ */
static double sse_power(const double* x, const double* y, int n, double a, double b, double c)
{
    double s = 0.0;
    for (int i = 0; i < n; ++i) {
        if (x[i] <= 0.0) return INFINITY;
        double lnx = log(x[i]);
        double xb = exp(b * lnx);
        double fi = a * xb + c;
        if (!isfinite(fi)) return INFINITY;
        double r = y[i] - fi;
        s += r * r;
    }
    return s;
}

/* ------------------ 对固定 b 的闭式解：求 a, c 并返回 SSE ------------------ */
static double sse_given_b(const double* x, const double* y, int n, double b,
                          double* a_out, double* c_out)
{
    const double BMIN = -100.0, BMAX = 100.0;
    if (!isfinite(b)) return INFINITY;
    if (b < BMIN) b = BMIN;
    if (b > BMAX) b = BMAX;

    double Suu = 0.0, Su = 0.0, Suy = 0.0, Sy = 0.0;
    double* u = (double*)malloc(sizeof(double) * n);
    if (!u) return INFINITY;
    for (int i = 0; i < n; ++i) {
        double lnx = log(x[i]);
        double ui = exp(b * lnx);
        u[i] = ui;
        Suu += ui * ui;
        Su += ui;
        Suy += ui * y[i];
        Sy += y[i];
    }
    double det = Suu * n - Su * Su;
    if (fabs(det) < 1e-30) {
        free(u);
        return INFINITY;
    }

    double a = (Suy * n - Su * Sy) / det;
    double c = (Suu * Sy - Su * Suy) / det;

    double sse = 0.0;
    for (int i = 0; i < n; ++i) {
        double r = y[i] - (a * u[i] + c);
        sse += r * r;
    }
    free(u);
    if (a_out) *a_out = a;
    if (c_out) *c_out = c;
    return sse;
}

/* ------------------ b 的粗网格扫描 + 黄金分割精化 ------------------ */
static void find_best_b_by_grid_then_golden(const double* x, const double* y, int n,
                                            double bmin, double bmax,
                                            double* best_b, double* best_a, double* best_c, double* best_sse)
{
    const double coarse_step = 0.5;
    int m = (int)ceil((bmax - bmin) / coarse_step) + 1;
    if (m < 3) m = 3;
    double* bs = (double*)malloc(sizeof(double) * m);
    double* vals = (double*)malloc(sizeof(double) * m);
    if (!bs || !vals) {
        if (bs) free(bs);
        if (vals) free(vals);
        *best_sse = INFINITY;
        return;
    }

    int idx = 0;
    for (double bb = bmin; bb <= bmax + 1e-12 && idx < m; bb += coarse_step, ++idx) {
        bs[idx] = bb;
        vals[idx] = sse_given_b(x, y, n, bb, NULL, NULL);
    }
    int used = idx;
    int im = 0;
    for (int i = 1; i < used; ++i) if (vals[i] < vals[im]) im = i;

    double left = bs[(im - 2 >= 0) ? im - 2 : 0];
    double right = bs[(im + 2 < used) ? im + 2 : used - 1];

    const double phi = (sqrt(5.0) - 1.0) / 2.0;
    double a = left, b = right;
    double c = b - phi * (b - a);
    double d = a + phi * (b - a);
    double fc = sse_given_b(x, y, n, c, NULL, NULL);
    double fd = sse_given_b(x, y, n, d, NULL, NULL);
    int maxit = 80;
    for (int it = 0; it < maxit && (b - a) > 1e-8; ++it) {
        if (fc < fd) {
            b = d;
            d = c;
            fd = fc;
            c = b - phi * (b - a);
            fc = sse_given_b(x, y, n, c, NULL, NULL);
        }
        else {
            a = c;
            c = d;
            fc = fd;
            d = a + phi * (b - a);
            fd = sse_given_b(x, y, n, d, NULL, NULL);
        }
    }
    double bbest = (fc < fd) ? c : d;
    double abest = 0.0, cbest = 0.0;
    double ssebest = sse_given_b(x, y, n, bbest, &abest, &cbest);

    *best_b = bbest;
    *best_a = abest;
    *best_c = cbest;
    *best_sse = ssebest;

    free(bs);
    free(vals);
}

/* ------------------ 三点精确解（若 n==3） ------------------ */
/* 为简洁起见保留之前的 solve_power3 接近实现（尝试解析 b），
   若成功返回 0 并把 sse 设为 0（穿点），否则返回非 0 继续 LM。 */
static double F_for3(double b, double lnx1, double lnx2, double lnx3, double r)
{
    double u = exp(b * lnx1);
    double v = exp(b * lnx2);
    double w = exp(b * lnx3);
    double denom = u - w;
    if (!isfinite(u) || !isfinite(v) || !isfinite(w) || fabs(denom) < 1e-300) return NAN;
    return (u - v) / denom - r;
}

static int solve_power3_try(const double* x, const double* y, double* a, double* b, double* c)
{
    const double x1 = x[0], x2 = x[1], x3 = x[2];
    const double y1 = y[0], y2 = y[1], y3 = y[2];
    if (fabs(y1 - y2) < 1e-15 && fabs(y1 - y3) < 1e-15) {
        *a = 0.0;
        *b = 1.0;
        *c = y1;
        return 0;
    }
    double r = (y1 - y2) / (y1 - y3);
    double lnx1 = log(x1), lnx2 = log(x2), lnx3 = log(x3);
    const double samples[] = {-50, -20, -10, -5, -2, -1, -0.5, -0.25, 0.0, 0.25, 0.5, 1.0, 2.0, 5.0, 10.0, 20.0, 50.0};
    int ns = (int)(sizeof(samples) / sizeof(samples[0]));
    int br_l = -1, br_r = -1;
    double fl = 0, fr = 0;
    for (int i = 0; i < ns - 1; i++) {
        double flt = F_for3(samples[i], lnx1, lnx2, lnx3, r);
        double frt = F_for3(samples[i + 1], lnx1, lnx2, lnx3, r);
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
    double fleft = F_for3(left, lnx1, lnx2, lnx3, r), fright = F_for3(right, lnx1, lnx2, lnx3, r);
    if (!isfinite(fleft) || !isfinite(fright)) return -2;
    int it = 0;
    for (; it < 200; ++it) {
        double mid = 0.5 * (left + right);
        double fmid = F_for3(mid, lnx1, lnx2, lnx3, r);
        if (!isfinite(fmid)) {
            left = left + 0.25 * (mid - left);
            right = right - 0.25 * (right - mid);
            continue;
        }
        if (fabs(fmid) < 1e-14) {
            left = right = mid;
            break;
        }
        if (fleft * fmid <= 0.0) {
            right = mid;
            fright = fmid;
        }
        else {
            left = mid;
            fleft = fmid;
        }
        if (fabs(right - left) < 1e-12 * (1 + fabs(left))) break;
    }
    double bb = 0.5 * (left + right);
    double u = pow(x1, bb);
    double v = pow(x2, bb);
    double denom = u - v;
    if (fabs(denom) < 1e-300) return -3;
    double aa = (y1 - y2) / denom;
    double cc = y1 - aa * u;
    *a = aa;
    *b = bb;
    *c = cc;
    return 0;
}

/* ------------------ 主拟合函数：整合全局搜索 + LM 精化 ------------------ */
int fit_power_lm(const double* x, const double* y, int n, int max_iter, double tol,
                 double* a, double* b, double* c, double* sse_out, int* n_iter_out)
{
    if (!x || !y || !a || !b || !c || n <= 0) return -10;
    if (n_iter_out) *n_iter_out = 0;

    /* 线性快速分支 */
    double Sx = 0, Sy = 0, Sxx = 0, Sxy = 0, Syy = 0;
    for (int i = 0; i < n; i++) {
        Sx += x[i];
        Sy += y[i];
        Sxx += x[i] * x[i];
        Sxy += x[i] * y[i];
    }
    double mean_y = Sy / n;
    for (int i = 0; i < n; i++) {
        double dy = y[i] - mean_y;
        Syy += dy * dy;
    }
    double denom_lin = n * Sxx - Sx * Sx;
    if (fabs(denom_lin) > 1e-24) {
        double k = (n * Sxy - Sx * Sy) / denom_lin;
        double cc = (Sy - k * Sx) / n;
        double sse_lin = 0.0;
        for (int i = 0; i < n; i++) {
            double fi = k * x[i] + cc;
            double r = y[i] - fi;
            sse_lin += r * r;
        }
        double rel = sse_lin / (Syy + 1e-30);
        if (rel < 1e-12 || sse_lin < 1e-24) {
            *a = k;
            *b = 1.0;
            *c = cc;
            if (sse_out) *sse_out = sse_lin;
            return 0;
        }
    }

    for (int i = 0; i < n; i++) if (x[i] <= 0.0) return -60;

    /* n==3 时尝试解析三点解（穿点） */
    if (n == 3) {
        double aa, bb, cc;
        if (solve_power3_try(x, y, &aa, &bb, &cc) == 0) {
            *a = aa;
            *b = bb;
            *c = cc;
            if (sse_out) *sse_out = 0.0;
            return 0;
        }
    }

    /* 全局搜索 b（粗→精） */
    double bmin = -20.0, bmax = 20.0;
    double A0, B0, C0, S0;
    find_best_b_by_grid_then_golden(x, y, n, bmin, bmax, &B0, &A0, &C0, &S0);
    if (!isfinite(B0) || !isfinite(A0) || !isfinite(C0)) {
        C0 = 0.0;
        for (int i = 0; i < n; i++) C0 += y[i];
        C0 /= n;
        A0 = y[0] - C0;
        B0 = 0.0;
    }

    /* LM 精化（以全局结果为初值） */
    double A = A0, B = B0, C = C0;
    double lambda = 1e-3;
    double prev_sse = sse_power(x, y, n, A, B, C);
    int it;
    int converged = 0;
    for (it = 0; it < max_iter; ++it) {
        double JTJ[3][3] = {{0}}, JTr[3] = {0};
        for (int i = 0; i < n; ++i) {
            double xi = x[i], yi = y[i];
            double lnx = log(xi);
            double xb = exp(B * lnx);
            double fi = A * xb + C;
            double ri = yi - fi;
            double Ja = xb, Jb = A * xb * lnx, Jc = 1.0;
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
        JTJ[0][0] += d0 * lambda;
        JTJ[1][1] += d1 * lambda;
        JTJ[2][2] += d2 * lambda;

        double rhs[3] = {JTr[0], JTr[1], JTr[2]}, delta[3] = {0};
        double Acpy[3][3];
        for (int r = 0; r < 3; r++) for (int cc = 0; cc < 3; cc++) Acpy[r][cc] = JTJ[r][cc];
        int sol = solve3x3(Acpy, rhs, delta);
        if (sol != 0) {
            lambda *= 10.0;
            if (lambda > 1e20) break;
            else continue;
        }

        double A_new = A + delta[0], B_new = B + delta[1], C_new = C + delta[2];
        if (!isfinite(B_new)) B_new = B;
        if (B_new < -100.0) B_new = -100.0;
        if (B_new > 100.0) B_new = 100.0;
        double sse_new = sse_power(x, y, n, A_new, B_new, C_new);

        double step_norm = sqrt(delta[0] * delta[0] + delta[1] * delta[1] + delta[2] * delta[2]);
        double rel_sse = fabs(prev_sse - sse_new) / (1.0 + prev_sse);

        if (isfinite(sse_new) && sse_new < prev_sse) {
            A = A_new;
            B = B_new;
            C = C_new;
            if (rel_sse < 1e-12 || step_norm < tol || rel_sse < 1e-9) {
                converged = 1;
                ++it;
                prev_sse = sse_new;
                break;
            }
            prev_sse = sse_new;
            lambda *= 0.3;
            if (lambda < 1e-20) lambda = 1e-20;
        }
        else {
            lambda *= 10.0;
            if (lambda > 1e20) {
                ++it;
                break;
            }
        }
    }

    *a = A;
    *b = B;
    *c = C;
    if (sse_out) *sse_out = prev_sse;
    if (n_iter_out) *n_iter_out = it;
    return converged ? 0 : 1;
}

/* ------------------ main：两个测试集 ------------------ */
int main(void)
{
    /* 数据集1（示例） */
    const double x1[] = {300.0, 600.0, 2000.0};
    const double y1[] = {300, 700, 2000};
    const int n1 = 3;
    double a, b, c, sse;
    int iters;
    int ret = fit_power_lm(x1, y1, n1, 200, 1e-12, &a, &b, &c, &sse, &iters);
    printf("Dataset 1:\n");
    printf("  ret=%d, a=%.12g, b=%.12g, c=%.12g, SSE=%.12g, iters=%d\n", ret, a, b, c, sse, iters);
    for (int i = 0; i < n1; i++) {
        double fi = a * pow(x1[i], b) + c;
        printf("    x=%.0f, y=%.6f, f=%.6f, r=%.6f\n", x1[i], y1[i], fi, y1[i] - fi);
    }

    /* 数据集2（你的问题集） */
    const double x2[] = {300.0, 600.0, 800.0, 2000.0};
    const double y2[] = {-0.005904406213615, -0.002741179358446, -0.002156558016386, -0.000645620903348};
    const int n2 = 4;
    ret = fit_power_lm(x2, y2, n2, 2000, 1e-12, &a, &b, &c, &sse, &iters);
    printf("\nDataset 2:\n");
    printf("  ret=%d, a=%.12g, b=%.12g, c=%.12g, SSE=%.12g, iters=%d\n", ret, a, b, c, sse, iters);
    for (int i = 0; i < n2; i++) {
        double fi = a * pow(x2[i], b) + c;
        printf("    x=%.0f, y=%.12f, f=%.12f, r=%.12f\n", x2[i], y2[i], fi, y2[i] - fi);
    }

    return 0;
}
