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
  // Tiempos
  timespec start, finish, dif, ini;

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

  // Inicializamos entorno paralelo, midiendo tiempos.
  clock_gettime(CLOCK_REALTIME, &start);

  MPI_Init(&argc, &argv);
  // Asignamos rank y obtenemos el tamaño.
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  /*
    Cada proceso utilizará las filas que le correspondan de la primera matriz y la segunda matriz completa.
    Así, cada proceso calculará tantas filas de la matriz resultado como filas haya tomado de la primera matriz.
  */

    // Maestro. Envía el número de filas que va a calcular cada proceso, y recibe los datos.
    // Realiza sus cálculos y luego recibe los datos de los otros procesos.

  if (rank == 0){
    clock_gettime(CLOCK_REALTIME, &finish);
    ini = diff(start, finish);
    printf("Tiempo de inicialización: %ld.%09ld\n",
         ini.tv_sec, ini.tv_nsec);


    // Para el trabajo
    // Tendremos n trabajadores: el maestro y los n-1 esclavos.
    int workers = size;
    int workers_left = workers;
    int rows_left = rows1;
    int row_count = 0;
    int rows_to_send;

    // Para el trabajo del Maestro: empieza siempre en la fila 0.
    // Almacenamos el número de filas que va a calcular.
    int my_rows = rows_left / workers_left;
    rows_left -= my_rows;

    // Quitamos un trabajador restante y actualizamos la fila por la que vamos.
    workers_left--;
    row_count += my_rows;

    // cout << "El maestro reparte el trabajo." << endl;

    // Para el reparto de trabajo al resto de procesos.
    for (int i = 1; i < workers; ++i){
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


    // Para el tiempo de cómputo
    clock_gettime(CLOCK_REALTIME, &start);

    // Matriz producto. Inicializada a 0
    vector< vector<float>> matrixf(rows1, vector<float>(cols2, 0));

    // Cálculos
    //cout << "El maestro hace sus cálculos" << endl;

    for (int i = 0; i < my_rows; ++i)
      for (int k = 0; k < cols1; ++k)
        for (int j = 0; j < cols2; ++j)
          matrixf.at(i).at(j) += matrix1.at(i).at(k) * matrix2.at(k).at(j);

    // Tiempos
    clock_gettime(CLOCK_REALTIME, &finish);

    dif = diff(start, finish);
    printf("Tiempo de cómputo: %ld.%09ld\n",
         dif.tv_sec, dif.tv_nsec);

    // Recepción de datos
    //cout << "Esperando recepción" << endl;

    // Recepción de datos
    for (int i = 1; i < workers; ++i){
      int first_row, number_rows;
      // Recibimos primera fila
      MPI_Recv(&first_row, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
      // Recibimos número de filas
      MPI_Recv(&number_rows, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &status);

      for (int j = 0; j < number_rows; ++j)
        MPI_Recv(&matrixf.at(first_row + j).front(), cols2, MPI_FLOAT, i, j + 2, MPI_COMM_WORLD, &status);

    }

    //cout << "Todo recibido" << endl;

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
