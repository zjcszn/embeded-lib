/* power_fit_lm_mcu.c
 *
 * MCU 友好版本：全局 b 搜索 + Brent 精细化 + LM refine
 * 不使用 malloc，所有缓冲区栈分配
 * 拟合模型：f(x) = a * x^b + c
 */

#include <math.h>
#include <stdio.h>
#include <float.h>

#define MAX_N               (16)
#define B_SEARCH_MIN        (-20.0)
#define B_SEARCH_MAX        (20.0)
#define B_CLAMP_MIN         (-100.0)
#define B_CLAMP_MAX         (100.0)
#define EPS_DET             (1e-30)
#define LM_MAX_IT           (2000)
#define GOLDEN_RATIO        (0.618033988749895)
#define BRENT_TOL           (1e-8)
#define BRENT_MAX_IT        (100)
#define THREE_POINT_TOL     (1e-12)

/* 日志输出控制 */
#define LOG_ENABLED 1
#if LOG_ENABLED
  #define LOG_PRINTF(...) printf(__VA_ARGS__)
#else
  #define LOG_PRINTF(...) ((void)0)
#endif

/* ------------------ 拟合返回状态 ------------------ */
typedef enum {
  FIT_OK = 0,                 // 拟合成功
  FIT_PARTIAL = 1,            // 达到最大迭代但未收敛
  FIT_ERROR_INVALID_ARG = -1, // 参数非法
  FIT_ERROR_INVALID_X = -2,    // x[i] <= 0，不合法
  FIT_ERROR_SINGULAR = -3,    // 矩阵求解奇异
  FIT_ERROR_NO_SOLUTION = -4, // 无解（解析拟合失败）
  FIT_ERROR_OVERFLOW = -5     // 数值溢出
} FitStatus;

/* ------------------ 3x3 线性方程求解 ------------------ */
static FitStatus Solve3x3(double A[3][3], double b[3], double x[3]) {
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

    if (max_val < EPS_DET) {
      return FIT_ERROR_SINGULAR;
    }

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
      for (int k = i; k < 3; ++k) {
        A[j][k] -= factor * A[i][k];
      }
      b[j] -= factor * b[i];
    }
  }

  for (int i = 2; i >= 0; --i) {
    double s = b[i];
    for (int j = i + 1; j < 3; ++j) {
      s -= A[i][j] * x[j];
    }

    if (fabs(A[i][i]) < EPS_DET) {
      return FIT_ERROR_SINGULAR;
    }

    x[i] = s / A[i][i];
  }

  return FIT_OK;
}

/* ------------------ SSE 计算 ------------------ */
static double SsePowerWithLnx(const double* lnx, const double* x,
                              const double* y, int n, double a, double b,
                              double c) {
  double sse = 0.0;
  for (int i = 0; i < n; ++i) {
    double xb = exp(b * lnx[i]);
    if (!isfinite(xb)) {
      return INFINITY;
    }

    double fi = a * xb + c;
    if (!isfinite(fi)) {
      return INFINITY;
    }

    double residual = y[i] - fi;
    sse += residual * residual;
  }
  return sse;
}

/* ------------------ 计算总平方和 SST ------------------ */
static double CalculateSst(const double* y, int n) {
  if (n <= 0) {
    return 0.0;
  }

  double mean = 0.0;
  for (int i = 0; i < n; ++i) {
    mean += y[i];
  }
  mean /= n;

  double sst = 0.0;
  for (int i = 0; i < n; ++i) {
    double deviation = y[i] - mean;
    sst += deviation * deviation;
  }

  return sst;
}

/* ------------------ 计算 R² ------------------ */
static double CalculateRSquared(double sse, double sst) {
  if (sst == 0.0) {
    // 如果所有y值相同，则R²定义为1（完美拟合）
    return 1.0;
  }

  return 1.0 - (sse / sst);
}

/* ------------------ 固定 b，解析求 a,c ------------------ */
/*
 * u_buf 优化分析：
 * 这个缓冲区用于存储中间计算结果 exp(b * lnx[i])，避免重复计算。
 * 由于这个计算在循环中被多次使用，使用缓冲区确实可以提高性能。
 * 我们可以考虑将其作为局部变量而不是参数传递，但这样会增加栈使用。
 * 对于MCU环境，保持当前设计是合理的，因为它避免了动态内存分配。
 */
static double SseGivenBWithLnxs(const double* lnx, const double* x,
                                const double* y, int n, double b,
                                double* u_buf, double* a_out,
                                double* c_out) {
  if (!isfinite(b)) {
    return INFINITY;
  }

  if (b < B_CLAMP_MIN) {
    b = B_CLAMP_MIN;
  }
  if (b > B_CLAMP_MAX) {
    b = B_CLAMP_MAX;
  }

  double sum_uu = 0.0, sum_u = 0.0, sum_uy = 0.0, sum_y = 0.0;

  for (int i = 0; i < n; ++i) {
    double ui = exp(b * lnx[i]);
    if (!isfinite(ui)) {
      return INFINITY;
    }

    u_buf[i] = ui;
    sum_uu += ui * ui;
    sum_u += ui;
    sum_uy += ui * y[i];
    sum_y += y[i];
  }

  double det = sum_uu * n - sum_u * sum_u;
  if (fabs(det) < EPS_DET) {
    return INFINITY;
  }

  double a = (sum_uy * n - sum_u * sum_y) / det;
  double c = (sum_uu * sum_y - sum_u * sum_uy) / det;

  double sse = 0.0;
  for (int i = 0; i < n; ++i) {
    double residual = y[i] - (a * u_buf[i] + c);
    sse += residual * residual;
  }

  if (a_out != NULL) {
    *a_out = a;
  }
  if (c_out != NULL) {
    *c_out = c;
  }

  return sse;
}

/* ------------------ Brent 方法搜索 b ------------------ */
static void FindBestBByBrent(const double* x, const double* y,
                             const double* lnx, int n, double b_min,
                             double b_max, double* best_b, double* best_a,
                             double* best_c, double* best_sse) {
  double u_buf[MAX_N];

  // 粗搜索找到初始点
  double coarse_best = 0.0, coarse_val = INFINITY;
  for (double bb = b_min; bb <= b_max; bb += 1.0) {
    double val = SseGivenBWithLnxs(lnx, x, y, n, bb, u_buf, NULL, NULL);
    if (val < coarse_val) {
      coarse_val = val;
      coarse_best = bb;
    }
  }

  double a = b_min, b_val = b_max, c = coarse_best, fc = coarse_val;
  double d = 0.0, e = 0.0;

  double fa = SseGivenBWithLnxs(lnx, x, y, n, a, u_buf, NULL, NULL);
  double fb = SseGivenBWithLnxs(lnx, x, y, n, b_val, u_buf, NULL, NULL);

  int iter;
  for (iter = 0; iter < BRENT_MAX_IT; ++iter) {
    double xm = 0.5 * (a + b_val);
    double tol1 = BRENT_TOL * fabs(c) + 1e-10;
    double tol2 = 2.0 * tol1;

    if (fabs(c - xm) <= tol2 - 0.5 * (b_val - a)) {
      break;
    }

    double p = 0.0, q = 0.0, r_val = 0.0;

    if (fabs(e) > tol1) {
      r_val = (c - d) * (fc - fb);
      q = (c - b_val) * (fc - fa);
      p = (c - b_val) * q - (c - d) * r_val;
      q = 2.0 * (q - r_val);

      if (q > 0) {
        p = -p;
      }

      if (fabs(p) < fabs(0.5 * q * e) &&
          p > q * (a - c) &&
          p < q * (b_val - c)) {
        d = p / q;
        double u = c + d;

        if (u - a < tol2 || b_val - u < tol2) {
          d = (xm - c >= 0 ? tol1 : -tol1);
        }
      } else {
        e = (c >= xm ? a - c : b_val - c);
        d = GOLDEN_RATIO * e;
      }
    } else {
      e = (c >= xm ? a - c : b_val - c);
      d = GOLDEN_RATIO * e;
    }

    double u = (fabs(d) >= tol1 ? c + d : c + (d >= 0 ? tol1 : -tol1));
    double fu = SseGivenBWithLnxs(lnx, x, y, n, u, u_buf, NULL, NULL);

    if (fu <= fc) {
      if (u >= c) {
        a = c;
      } else {
        b_val = c;
      }
      fa = fc;
      d = e;
      e = c;
      c = u;
      fc = fu;
    } else {
      if (u < c) {
        a = u;
      } else {
        b_val = u;
      }
    }
  }

  LOG_PRINTF("find best b by brent, iter count = %d\r\n", iter);

  double abest = 0.0, cbest = 0.0;
  double ssebest = SseGivenBWithLnxs(lnx, x, y, n, c, u_buf, &abest, &cbest);

  *best_b = c;
  *best_a = abest;
  *best_c = cbest;
  *best_sse = ssebest;
}

/* ------------------ n=3 精确拟合 ------------------ */
static double FFor3(double b, double lnx1, double lnx2, double lnx3, double r) {
  double u = exp(b * lnx1);
  double v = exp(b * lnx2);
  double w = exp(b * lnx3);

  if (!isfinite(u) || !isfinite(v) || !isfinite(w)) {
    return NAN;
  }

  double denom = u - w;
  if (fabs(denom) < 1e-300) {
    return NAN;
  }

  return (u - v) / denom - r;
}

static FitStatus SolvePower3Try(const double* x, const double* y, const double* lnx,
                                double* a, double* b, double* c) {
  const double x1 = x[0], x2 = x[1], x3 = x[2];
  const double y1 = y[0], y2 = y[1], y3 = y[2];

  if (fabs(y1 - y2) < 1e-15 && fabs(y1 - y3) < 1e-15) {
    *a = 0.0;
    *b = 1.0;
    *c = y1;
    return FIT_OK;
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

    if (!isfinite(flt) || !isfinite(frt)) {
      continue;
    }

    if (flt * frt < 0.0) {
      br_l = i;
      br_r = i + 1;
      fl = flt;
      fr = frt;
      break;
    }
  }

  if (br_l == -1) {
    return FIT_ERROR_NO_SOLUTION;
  }

  double left = samples[br_l], right = samples[br_r];
  int iter;

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
    } else {
      left = mid;
      fl = fmid;
    }

    if (fabs(right - left) < THREE_POINT_TOL * (1 + fabs(left))) {
      break;
    }
  }

  LOG_PRINTF("solve power 3 point, iter count: %d\r\n", iter);
  double bb = 0.5 * (left + right);
  double u = pow(x1, bb);
  double v = pow(x2, bb);

  if (fabs(u - v) < 1e-300) {
    return FIT_ERROR_SINGULAR;
  }

  double aa = (y1 - y2) / (u - v);
  double cc = y1 - aa * u;

  *a = aa;
  *b = bb;
  *c = cc;

  return FIT_OK;
}

/* ------------------ 主拟合函数 ------------------ */
FitStatus FitPowerLM(const double* x, const double* y, int n, int max_iter,
                     double tol, double* a_out, double* b_out, double* c_out,
                     double* r_squared_out, int* n_iter_out) {
  if (x == NULL || y == NULL || a_out == NULL ||
      b_out == NULL || c_out == NULL || n <= 0 || n > MAX_N) {
    return FIT_ERROR_INVALID_ARG;
  }

  if (n_iter_out != NULL) {
    *n_iter_out = 0;
  }

  double lnx[MAX_N];
  for (int i = 0; i < n; ++i) {
    if (x[i] <= 0.0) {
      return FIT_ERROR_INVALID_X;
    }
    lnx[i] = log(x[i]);
  }

  // 计算总平方和 SST
  double sst = CalculateSst(y, n);

  // 快速分支判定，线性回归: y = kx+b
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

    if (sse_lin / (sst + 1e-30) < 1e-12 || sse_lin < 1e-24) {
      *a_out = k;
      *b_out = 1.0;
      *c_out = c0;

      if (r_squared_out != NULL) {
        *r_squared_out = CalculateRSquared(sse_lin, sst);
      }

      return FIT_OK;
    }
  }

  /* 三点精准解 */
  if (n == 3) {
    double aa, bb, cc;
    FitStatus status = SolvePower3Try(x, y, lnx, &aa, &bb, &cc);
    if (status == FIT_OK) {
      *a_out = aa;
      *b_out = bb;
      *c_out = cc;

      if (r_squared_out != NULL) {
        // 三点精确拟合，SSE为0，R²为1
        *r_squared_out = 1.0;
      }

      return FIT_OK;
    }
  }

  double A0, B0, C0, S0;
  FindBestBByBrent(x, y, lnx, n, B_SEARCH_MIN, B_SEARCH_MAX,
                   &B0, &A0, &C0, &S0);

  if (!isfinite(B0) || !isfinite(A0) || !isfinite(C0)) {
    return FIT_ERROR_OVERFLOW;
  }

  double A = A0, B = B0, C = C0;
  double lambda = 1e-3;
  double prev_sse = SsePowerWithLnx(lnx, x, y, n, A, B, C);
  int it = 0;
  int converged = 0;

  for (it = 0; it < max_iter; ++it) {
    double JTJ[3][3] = {{0.0}}, JTr[3] = {0.0};

    for (int i = 0; i < n; ++i) {
      double xb = exp(B * lnx[i]);
      if (!isfinite(xb)) {
        continue;
      }

      double fi = A * xb + C;
      double residual = y[i] - fi;
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

      JTr[0] += Ja * residual;
      JTr[1] += Jb * residual;
      JTr[2] += Jc * residual;
    }

    double d0 = (fabs(JTJ[0][0]) > 0) ? fabs(JTJ[0][0]) : 1.0;
    double d1 = (fabs(JTJ[1][1]) > 0) ? fabs(JTJ[1][1]) : 1.0;
    double d2 = (fabs(JTJ[2][2]) > 0) ? fabs(JTJ[2][2]) : 1.0;

    JTJ[0][0] += lambda * d0;
    JTJ[1][1] += lambda * d1;
    JTJ[2][2] += lambda * d2;

    double delta[3] = {0.0};
    double Acpy[3][3];

    for (int r = 0; r < 3; r++) {
      for (int c = 0; c < 3; c++) {
        Acpy[r][c] = JTJ[r][c];
      }
    }

    FitStatus solve_status = Solve3x3(Acpy, JTr, delta);
    if (solve_status != FIT_OK) {
      lambda *= 10.0;
      if (lambda > 1e20) {
        break;
      } else {
        continue;
      }
    }

    double A_new = A + delta[0];
    double B_new = B + delta[1];
    double C_new = C + delta[2];

    if (!isfinite(B_new)) {
      B_new = B;
    }

    if (B_new < B_CLAMP_MIN) {
      B_new = B_CLAMP_MIN;
    }
    if (B_new > B_CLAMP_MAX) {
      B_new = B_CLAMP_MAX;
    }

    double sse_new = SsePowerWithLnx(lnx, x, y, n, A_new, B_new, C_new);
    double step_norm = sqrt(delta[0] * delta[0] +
                           delta[1] * delta[1] +
                           delta[2] * delta[2]);
    double rel_sse = fabs(prev_sse - sse_new) / (1.0 + prev_sse);

    if (isfinite(sse_new) && sse_new < prev_sse) {
      A = A_new;
      B = B_new;
      C = C_new;
      prev_sse = sse_new;

      if (rel_sse < 1e-12 || step_norm < tol) {
        converged = 1;
        it++;
        break;
      }

      lambda *= 0.3;
      if (lambda < 1e-20) {
        lambda = 1e-20;
      }
    } else {
      lambda *= 10.0;
      if (lambda > 1e20) {
        break;
      }
    }
  }

  *a_out = A;
  *b_out = B;
  *c_out = C;

  if (r_squared_out != NULL) {
    *r_squared_out = CalculateRSquared(prev_sse, sst);
  }

  if (n_iter_out != NULL) {
    *n_iter_out = it;
  }

  return converged ? FIT_OK : FIT_PARTIAL;
}

/* ------------------ main 测试 ------------------ */
int main(void) {
  double a, b, c, r_squared;
  int iters;
  FitStatus ret;

  /* Dataset1 */
  const double x1[] = {300.0, 600.0, 2000.0};
  const double y1[] = {300, 600, 2000};

  LOG_PRINTF("Dataset1\n");
  ret = FitPowerLM(x1, y1, 3, LM_MAX_IT, 1e-12, &a, &b, &c, &r_squared, &iters);
  LOG_PRINTF("  ret=%d, a=%.12g, b=%.12g, c=%.12g, R^2=%.12g, iters=%d\n",
         ret, a, b, c, r_squared, iters);

  for (int i = 0; i < sizeof(x1) / sizeof(x1[0]); i++) {
    double fi = a * pow(x1[i], b) + c;
    LOG_PRINTF("    x=%.0f, y=%.12f, f=%.12f, r=%.12f\n",
           x1[i], y1[i], fi, y1[i] - fi);
  }

  /* Dataset2 */
  const double x2[] = {300.0, 600.0, 800.0, 2000.0};
  const double y2[] = {
    -0.005904406213615, -0.002741179358446,
    -0.002156558016386, -0.000645620903348
  };

  LOG_PRINTF("Dataset2:\n");
  ret = FitPowerLM(x2, y2, 4, LM_MAX_IT, 1e-12, &a, &b, &c, &r_squared, &iters);
  LOG_PRINTF("  ret=%d, a=%.12g, b=%.12g, c=%.12g, R^2=%.12g, iters=%d\n",
         ret, a, b, c, r_squared, iters);

  for (int i = 0; i < 4; i++) {
    double fi = a * pow(x2[i], b) + c;
    LOG_PRINTF("    x=%.0f, y=%.12f, f=%.12f, r=%.12f\n",
           x2[i], y2[i], fi, y2[i] - fi);
  }

  return 0;
}
