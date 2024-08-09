#include <cuda_runtime.h>
#include <iostream>

#define N 512  // Assuming matrix size is 512x512

__global__ void multiply_matrices_gpu(const float* A, const float* B, float* C)  
{
    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;
    float sum = 0.0f;

    if (row < N && col < N) {
        for (int k = 0; k < N; ++k) {
            sum += A[row * N + k] * B[k * N + col];
        }
        C[row * N + col] = sum;
    }
}

int main() {
    // Host matrices
    float *h_A, *h_B, *h_C;
    // Device matrices
    float *d_A, *d_B, *d_C;

    size_t bytes = N * N * sizeof(float);

    // Allocate memory on host
    h_A = (float*)malloc(bytes);
    h_B = (float*)malloc(bytes);
    h_C = (float*)malloc(bytes);

    // Initialize matrices A and B
    for (int i = 0; i < N * N; i++) {
        h_A[i] = static_cast<float>(i);
        h_B[i] = static_cast<float>(i);
    }

    // Allocate memory on device
    cudaMalloc(&d_A, bytes);
    cudaMalloc(&d_B, bytes);
    cudaMalloc(&d_C, bytes);

    // Copy data from host to device
    cudaMemcpy(d_A, h_A, bytes, cudaMemcpyHostToDevice);
    cudaMemcpy(d_B, h_B, bytes, cudaMemcpyHostToDevice);

    // Define block size and grid size
    dim3 threadsPerBlock(16, 16);
    dim3 blocksPerGrid((N + threadsPerBlock.x - 1) / threadsPerBlock.x, 
                       (N + threadsPerBlock.y - 1) / threadsPerBlock.y);

    // Launch the kernel
    multiply_matrices_gpu<<<blocksPerGrid, threadsPerBlock>>>(d_A, d_B, d_C);

    // Wait for GPU to finish
    cudaDeviceSynchronize();

    // Copy result back to host
    cudaMemcpy(h_C, d_C, bytes, cudaMemcpyDeviceToHost);

    // Print a portion of the result matrix C
    std::cout << "Result matrix C[0][0]: " << h_C[0] << std::endl;

    // Free memory
    free(h_A);
    free(h_B);
    free(h_C);

    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);

    return 0;
}
