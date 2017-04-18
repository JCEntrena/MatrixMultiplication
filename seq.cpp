#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <time.h>
#include <sstream>
#include <iostream>
#include <fstream>

using namespace std;

// VERSIÓN SECUENCIAL DEL PRODUCTO DE MATRICES. PARA COMPARAR TIEMPOS.


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
 */
vector< vector<float> > readMatrix(string file){
  ifstream ifs;
  ifs.open(file, ifstream::in);

  // Leemos elementos
  int rows, cols;
  ifs >> rows;
  ifs >> cols;

  vector< vector<float> > resultado(rows, vector<float>(cols, 0));

  // Lectura.
  for (int i = 0; i < rows; ++i)
    for (int j = 0; j < cols; ++j)
      ifs >> resultado.at(i).at(j);

  ifs.close();

  return resultado;
}


int main(int argc, char *argv[]){

  if (argc != 3){
    cerr << "Necesito dos argumentos, los ficheros que contienen las matrices.\n";
    exit(-1);
  }
  // Datos del programa
  string file1, file2;

  // Lectura de argumentos
  file1 = argv[1];
  file2 = argv[2];

  // Declaración
  vector< vector<float>> matrix1 = readMatrix(file1);
  vector< vector<float>> matrix2 = readMatrix(file2);

  // Tomamos las filas y columnas. Las necesitamos para definir la matrix producto.
  int rows1 = matrix1.size();
  int cols1 = matrix1.at(0).size();
  int rows2 = matrix2.size();
  int cols2 = matrix2.at(0).size();

  if (cols1 != rows2){
    cerr << "Las matrices no se pueden multiplicar." << endl;
    exit(-1);
  }

  // Matriz producto. Inicializada a 0.
  vector< vector<float>> matrixf(rows1, vector<float>(cols2, 0));

  // Para la medida de tiempos
  timespec start, finish, dif;
  clock_gettime(CLOCK_REALTIME, &start);

  // SECUENCIAL. CALCULAMOS EL PRODUCTO Y LO ESCRIBIMOS EN UN ARCHIVO.

  for (int i = 0; i < rows1; ++i)
    for (int k = 0; k < cols1; ++k)
      for (int j = 0; j < cols2; ++j)
        matrixf.at(i).at(j) += matrix1.at(i).at(k) * matrix2.at(k).at(j);

  // Tiempos
  clock_gettime(CLOCK_REALTIME, &finish);

  ofstream ofs("Resultado2.txt", ofstream::out);

  ofs << rows1 << " " << cols2 << endl;

  for (int i = 0; i < rows1; ++i){
    for (int j = 0; j < cols2; ++j)
      ofs << matrixf.at(i).at(j) << " ";
    ofs << endl;
  }


  dif = diff(start, finish);
  printf("Tiempo de cómputo: %ld.%09ld\n",
          dif.tv_sec, dif.tv_nsec);


  return(0);

}
