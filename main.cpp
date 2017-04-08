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

  // Datos MPI
  int rank, size;
  MPI_Status status;

  // Inicializamos entorno paralelo.
  MPI_Init(&argc, &argv);

  // Asignamos rank y obtenemos el tamaño.
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  /*
    Cada proceso utilizará las filas que le correspondan de la primera matriz y la segunda matriz completa.
    Así, cada proceso calculará tantas filas de la matriz resultado como filas haya tomado de la primera matriz.
  */

    // Maestro. Envía el número de filas que va a calcular cada proceso, y recibe los datos.
    // No realiza cálculos.

  if (rank == 0){

    int workers = size - 1;
    int workers_left = workers;
    int rows_left = rows1;
    int row_count = 0;
    int rows_to_send;

    for (int i = 1; i <= workers; ++i){
      // Repartimos filas según las que queden
      rows_to_send = rows_left / workers_left;
      rows_left -= rows_to_send;
      workers_left--;

      // Enviamos fila de inicio
      MPI_Send(&row_count, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
      // Enviamos número de filas
      MPI_Send(&rows_to_send, 1, MPI_INT, i, 1, MPI_COMM_WORLD);

      // Actualizamos la fila que tenemos que enviar.
      row_count += rows_to_send;
    }

    // Matriz producto. Inicializada a 0
    vector< vector<float>> matrixf(rows1, vector<float>(cols2, 0));

    // Recepción de datos
    for (int i = 1; i <= workers; ++i){
      int first_row, number_rows;
      // Recibimos primera fila
      MPI_Recv(&first_row, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
      // Recibimos número de filas
      MPI_Recv(&number_rows, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &status);

      for (int j = 0; j < number_rows; ++j){
        float aux[cols2];
        MPI_Recv(&matrixf.at(first_row + j).front(), cols2, MPI_FLOAT, i, j + 2, MPI_COMM_WORLD, &status);
      }

    }

    // Escritura de la matriz en un fichero
    ofstream ofs("Resultado.txt", ofstream::out);

    ofs << rows1 << " " << cols2 << endl;

    for (int i = 0; i < rows1; ++i){
      for (int j = 0; j < cols2; ++j)
        ofs << matrixf.at(i).at(j) << " ";
      ofs << endl;
    }
  }

  // Esclavo.

  if (rank != 0){

    int row_init;
    int n_rows;

    // Recibimos primera fila
    MPI_Recv(&row_init, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    // Recibimos número de filas
    MPI_Recv(&n_rows, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);

    // cout << "Hola, soy " << rank << ", empiezo en " << row_init << " y acabo en " << row_init + n_rows << endl;

    vector< vector<float>> matrix_parcial(n_rows, vector<float>(cols2, 0));

    for (int i = row_init; i < row_init + n_rows; ++i)
      for (int k = 0; k < cols1; ++k)
        for (int j = 0; j < cols2; ++j){
          //cout << "I: " << i << "\nK: " << k << "\nElem: " << matrix1.at(i).at(k) << endl;
          matrix_parcial.at(i - row_init).at(j) += matrix1.at(i).at(k) * matrix2.at(k).at(j);
        }

    // Enviamos los datos
    // Enviamos primera fila y número de filas
    MPI_Send(&row_init, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    MPI_Send(&n_rows, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);

    for (int i = 0; i < n_rows; ++i)
      MPI_Send(&matrix_parcial[i].front(), cols2, MPI_FLOAT, 0, i + 2, MPI_COMM_WORLD);


  }


  MPI_Finalize();

  return(0);

}
