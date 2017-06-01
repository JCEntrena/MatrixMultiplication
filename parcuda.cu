#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <vector>
#include <time.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cmath>
#include <numeric>
#include <cuda_runtime.h>
#include <utility>
//#include "helper_cuda.h"

using namespace std;


// Función para multiplicar matrices cuadradas en CUDA.
__global__ void matrixMul(const float *A, const float *B, float *C, int numRows)
{
    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;

    int numCols = numRows;

    if (row < numRows and col < numCols){
        int aux = 0;
        for (int i = 0; i < numRows; ++i){
          aux += A[row * numCols + i] * B[col + numRows * i];
        }

        C[row * numCols + col] = aux;
    }
}

/**
 * Función de cálculo de tiempos.
 * Usa clock_gettime.
 */
timespec diff(timespec start, timespec end){
     timespec temp;
     if ((end.tv_nsec - start.tv_nsec) < 0) {
       temp.tv_sec = end.tv_sec - start.tv_sec - 1;
       temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
     }
     else{
       temp.tv_sec = end.tv_sec - start.tv_sec;
       temp.tv_nsec = end.tv_nsec - start.tv_nsec;
     }
     return temp;
}

/**
 * Función de lectura de archivos.
 * Lee una matriz de un archivo y la devuelve.
 * Trabajamos con la matriz como un array.
 */
float* readMatrix(string file){
  ifstream ifs;
  ifs.open(file, ifstream::in);

  // Leemos elementos
  int rows, cols;
  ifs >> rows;
  ifs >> cols;

  float* resultado = (float *)malloc(rows * cols *sizeof(float));

  // Lectura.
  for (int i = 0; i < rows; ++i)
    for (int j = 0; j < cols; ++j)
      ifs >> resultado[i * cols + j];

  ifs.close();

  return resultado;
}

// Devuelve las filas y columnas de una matriz en forma de par.
pair<int, int> size(string file){
  ifstream ifs;
  const char* c = file.c_str();
  ifs.open(c, ifstream::in);

  // Leemos tamaño
  int rows, cols;
  ifs >> rows;
  ifs >> cols;

  ifs.close();

  return make_pair(rows, cols);
}
// Manejo de errores
void cudaErrorHandler(cudaError_t e, const int E)
{
    if (e != cudaSuccess) {
        fprintf(stderr, "ERROR(%d): %s\n", E, cudaGetErrorString(e));
        exit(EXIT_FAILURE);
    }
}


int main(int argc, char *argv[])
{

    if (argc != 4){
      cerr << "Necesito tres argumentos, los dos ficheros que contienen las matrices y el nombre de la matriz de salida.\n";
      exit(-1);
    }

    // Error code to check return values for CUDA calls
    cudaError_t err = cudaSuccess;

    // Datos del programa
    string file1, file2, file3;

    // Lectura de argumentos
    file1 = argv[1];
    file2 = argv[2];
    file3 = argv[3];

    // Para medir el tiempo de suma
    timespec start, finish, dif, t_alloc, t_comp;

    cerr << "Leyendo archivos" << endl;

    // Declaración de matrices.
    // Las consideramos como un array.
    float* m1 = readMatrix(file1);
    float* m2 = readMatrix(file2);

    cerr << "Matrices leídas" << endl;

    pair<int, int> size1 = size(file1);
    pair<int, int> size2 = size(file2);

    if (size1.second != size2.first){
      cerr << "No puedo multiplicar las matrices.\n";
      exit(-1);
    }

    // Tamaño
    size_t size = size1.first * size1.second * sizeof(float);
    // Declaración del vector suma.
    float* m3 = (float *)malloc(size);

    // Inicialización del tiempo
    clock_gettime(CLOCK_REALTIME, &start); 

    cerr << "Empezamos cudaMalloc" << endl;

    // Reservaos espacio para las matrices.
    float *d_A = NULL;
    err = cudaMalloc((void **)&d_A, size);

    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to allocate device vector A (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    float *d_B = NULL;
    err = cudaMalloc((void **)&d_B, size);


    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to allocate device vector B (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    float *d_C = NULL;
    err = cudaMalloc((void **)&d_C, size);

    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to allocate device vector C (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    // Copiamos las matrices.
    cerr << "Copia a memoria GPU" << endl;

    err = cudaMemcpy(d_A, m1, size, cudaMemcpyHostToDevice);

    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to copy vector A from host to device (error code %s)!\n", cudaGetErrorString(err));
        exit(-1);
    }

    err = cudaMemcpy(d_B, m2, size, cudaMemcpyHostToDevice);

    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to copy vector B from host to device (error code %s)!\n", cudaGetErrorString(err));
        exit(-1);
    }

    clock_gettime(CLOCK_REALTIME, &finish); 
    t_alloc = diff(start, finish); 

    // Producto
    cerr << "Lanzamos producto" << endl;

    clock_gettime(CLOCK_REALTIME, &start); 

    // Launch the Vector Add CUDA Kernel
    int threadsPerBlock = 256;
    int blocksPerGrid =(size1.first + threadsPerBlock - 1) / threadsPerBlock;
    printf("CUDA kernel launch with %d blocks of %d threads\n", blocksPerGrid, threadsPerBlock);
    matrixMul<<<blocksPerGrid, threadsPerBlock>>>(d_A, d_B, d_C, size2.first);
    err = cudaGetLastError();

    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to launch matrixMul kernel (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
 
    clock_gettime(CLOCK_REALTIME, &finish); 
    t_comp = diff(start, finish); 

    // Copia en memoria principal  

    cerr << "Copia en memoria principal" << endl;

    // Copy the device result vector in device memory to the host result vector
    // in host memory.
    
    err = cudaMemcpy(m3, d_C, size, cudaMemcpyDeviceToHost);

    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to copy vector C from device to host (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    // Finalización del tiempo
    clock_gettime(CLOCK_REALTIME, &finish);
    // Resultado
    dif = diff(start, finish);
    printf("%ld.%09ld, %ld.%09ld, %ld.%09ld\n",
            t_alloc.tv_sec, t_alloc.tv_nsec, t_comp.tv_sec, t_comp.tv_nsec, dif.tv_sec, dif.tv_nsec);


    // Free device global memory
    err = cudaFree(d_A);

    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to free device vector A (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    err = cudaFree(d_B);

    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to free device vector B (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    err = cudaFree(d_C);

    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to free device vector C (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    free(m1);
    free(m2);
    free(m3);

    return 0;
}
