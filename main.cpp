#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
//#include <mpi.h>
#include <time.h>
#include <sstream>
#include <iostream>
#include <fstream>

using namespace std;

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

  // SECUENCIAL. CALCULAMOS EL PRODUCTO Y LO ESCRIBIMOS EN UN ARCHIVO.

  for (int i = 0; i < rows1; ++i)
    for (int k = 0; k < cols1; ++k)
      for (int j = 0; j < cols2; ++j)
        matrixf.at(i).at(j) += matrix1.at(i).at(k) * matrix2.at(k).at(j);


  ofstream ofs("Resultado.txt", ofstream::out);

  ofs << rows1 << " " << cols2 << endl;

  for (int i = 0; i < rows1; ++i){
    for (int j = 0; j < cols2; ++j)
      ofs << matrixf.at(i).at(j) << " ";
    ofs << endl;
  }


/*
  // Iniciamos entorno MPI
  MPI_Init(&argc, &argv);




  // Asignamos rank a cada proceso y obtenemos el tamaño
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);





  MPI_Finalize();

*/
  return(0);

}
