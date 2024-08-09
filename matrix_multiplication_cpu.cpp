#include <iostream>
#include <vector>
#include <chrono>

#define N 512 // Size of the matrix

void multiply_matrices_cpu(const std::vector<std::vector<float>>& A, const std::vector<std::vector<float>>& B, std::vector<std::vector<float>>& C) {
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            C[i][j] = 0;
            for (int k = 0; k < N; ++k) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

int main() {
    std::vector<std::vector<float>> A(N, std::vector<float>(N, 1.0f));
    std::vector<std::vector<float>> B(N, std::vector<float>(N, 1.0f));
    std::vector<std::vector<float>> C(N, std::vector<float>(N, 0.0f));

    auto start = std::chrono::high_resolution_clock::now();
    
    multiply_matrices_cpu(A, B, C);
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    
    std::cout << "Matrix multiplication on CPU took " << elapsed.count() << " seconds.\n";

    return 0;
}
