#include <iostream>

// Kernel function to print "Hello, World!" from the GPU
__global__ void helloWorldKernel() {
    printf("Hello, World from GPU!\n");
}

int main() {
    // Launch the kernel with a single thread
    helloWorldKernel<<<1, 1>>>();

    // Wait for the GPU to finish before accessing on host
    cudaDeviceSynchronize();

    std::cout << "Hello, World from CPU!" << std::endl;

    return 0;
}
