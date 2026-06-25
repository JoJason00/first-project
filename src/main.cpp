#include <chrono>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

#if defined(_MSC_VER)
#define NOINLINE __declspec(noinline)
#else
#define NOINLINE __attribute__((noinline))
#endif

using Matrix = std::vector<double>;

static inline std::size_t idx(int row, int col, int n)
{
    return static_cast<std::size_t>(row) * n + col;
}

NOINLINE void init_matrix(Matrix& m, int n, double seed)
{
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            m[idx(i, j, n)] = seed + i * 0.001 + j * 0.0001;
        }
    }
}

NOINLINE void clear_matrix(Matrix& c)
{
    for (std::size_t i = 0; i < c.size(); ++i) {
        c[i] = 0.0;
    }
}

NOINLINE double checksum_matrix(const Matrix& c)
{
    double sum = 0.0;

    for (std::size_t i = 0; i < c.size(); i += 97) {
        sum += c[i];
    }

    return sum;
}

// 方法 A：i -> j -> k
// 缺点：最内层访问 b[k * n + j]，对行主序矩阵来说是跨行跳跃访问
NOINLINE void matmul_ijk_kernel(
    const double* a,
    const double* b,
    double* c,
    int n
)
{
    for (int i = 0; i < n; ++i) {
        const std::size_t row_i = static_cast<std::size_t>(i) * n;

        for (int j = 0; j < n; ++j) {
            double sum = 0.0;

            for (int k = 0; k < n; ++k) {
                sum += a[row_i + k] * b[static_cast<std::size_t>(k) * n + j];
            }

            c[row_i + j] = sum;
        }
    }
}

// 方法 B：i -> k -> j
// 优点：最内层连续访问 b[k * n + j] 和 c[i * n + j]
// 对 cache 和自动 SIMD 更友好
NOINLINE void matmul_ikj_kernel(
    const double* a,
    const double* b,
    double* c,
    int n
)
{
    for (int i = 0; i < n; ++i) {
        const std::size_t row_i = static_cast<std::size_t>(i) * n;

        for (int k = 0; k < n; ++k) {
            const double aik = a[row_i + k];
            const std::size_t row_k = static_cast<std::size_t>(k) * n;

            for (int j = 0; j < n; ++j) {
                c[row_i + j] += aik * b[row_k + j];
            }
        }
    }
}

NOINLINE double run_ijk_version(
    const Matrix& a,
    const Matrix& b,
    Matrix& c,
    int n,
    int repeat
)
{
    double total = 0.0;

    for (int r = 0; r < repeat; ++r) {
        clear_matrix(c);
        matmul_ijk_kernel(a.data(), b.data(), c.data(), n);
        total += checksum_matrix(c);
    }

    return total;
}

NOINLINE double run_ikj_version(
    const Matrix& a,
    const Matrix& b,
    Matrix& c,
    int n,
    int repeat
)
{
    double total = 0.0;

    for (int r = 0; r < repeat; ++r) {
        clear_matrix(c);
        matmul_ikj_kernel(a.data(), b.data(), c.data(), n);
        total += checksum_matrix(c);
    }

    return total;
}

NOINLINE double max_abs_diff(const Matrix& x, const Matrix& y)
{
    double max_diff = 0.0;

    for (std::size_t i = 0; i < x.size(); ++i) {
        const double diff = std::abs(x[i] - y[i]);

        if (diff > max_diff) {
            max_diff = diff;
        }
    }

    return max_diff;
}

int main()
{
    const int n = 600;
    const int repeat = 5;

    Matrix a(static_cast<std::size_t>(n) * n);
    Matrix b(static_cast<std::size_t>(n) * n);
    Matrix c_ijk(static_cast<std::size_t>(n) * n);
    Matrix c_ikj(static_cast<std::size_t>(n) * n);

    init_matrix(a, n, 1.0);
    init_matrix(b, n, 2.0);

    std::cout << "matrix size = " << n << " x " << n << '\n';
    std::cout << "repeat      = " << repeat << "\n\n";

    // ------------------------------------------------------------
    // 方法 A：i-j-k
    // ------------------------------------------------------------
    const auto t0 = std::chrono::high_resolution_clock::now();

    const double result_ijk = run_ijk_version(a, b, c_ijk, n, repeat);

    const auto t1 = std::chrono::high_resolution_clock::now();

    const double time_ijk =
        std::chrono::duration<double>(t1 - t0).count();

    // ------------------------------------------------------------
    // 方法 B：i-k-j
    // ------------------------------------------------------------
    const auto t2 = std::chrono::high_resolution_clock::now();

    const double result_ikj = run_ikj_version(a, b, c_ikj, n, repeat);

    const auto t3 = std::chrono::high_resolution_clock::now();

    const double time_ikj =
        std::chrono::duration<double>(t3 - t2).count();

    // ------------------------------------------------------------
    // 校验结果
    // ------------------------------------------------------------
    const double diff = max_abs_diff(c_ijk, c_ikj);

    std::cout << "[A] matmul_ijk_kernel\n";
    std::cout << "    result = " << result_ijk << '\n';
    std::cout << "    time   = " << time_ijk << " seconds\n\n";

    std::cout << "[B] matmul_ikj_kernel\n";
    std::cout << "    result = " << result_ikj << '\n';
    std::cout << "    time   = " << time_ikj << " seconds\n\n";

    std::cout << "max abs diff = " << diff << '\n';

    if (time_ikj > 0.0) {
        std::cout << "speedup      = " << time_ijk / time_ikj << "x\n";
    }

    return 0;
}