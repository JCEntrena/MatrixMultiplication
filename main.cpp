#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <mpi.h>
#include <time.h>
#include <sstream>
#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char *argv[]){

  if (argc != 3){
    cerr << "Necesito dos argumentos, los ficheros que contienen las matrices.\n";
    exit(-1);
  }
  // Datos del programa
  string file1, file2;

  // Lectura de argumentos
  istringstream iss( argv[1] );
  iss >> file1;
  iss >> file2;

  // Abrir los ficheros y leer las matrices
  ifstream ifs;
  // Matriz 1
  ifs.open(file1, ifstream::in);

  // Leemos filas y columnas.
  int rows1, cols1;
  ifs >> rows1;
  ifs >> cols1;

  // Declaración
  vector< vector<float>> matrix1 (rows1, vector<float>(cols1, 0));

  // Lectura.
  for (int i = 0; i < rows1; ++i)
    for (int j = 0; j < cols1; ++j)
      ifs >> matrix1.at(i).at(j);

  fs.close();

  // Matriz 2.
  fs.open(file2, ifstream::in);

  // Leemos filas y columnas.
  int rows2, cols2;
  ifs >> rows2;
  ifs >> cols2;

  // Declaración
  vector< vector<float>> matrix2 (rows2, vector<float>(cols2, 0));
  // Lectura.
  for (int i = 0; i < rows2; ++i)
    for (int j = 0; j < cols2; ++j)
      ifs >> matrix2.at(i).at(j);

  fs.close();

  // Matriz producto.
  vector< vector<float>> matrixf (rows1, vector<float>(cols2, 0));

  // Iniciamos entorno MPI
  MPI_Init(&argc, &argv);




  // Asignamos rank a cada proceso y obtenemos el tamaño
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);





  MPI_Finalize();
  return(0);

}
