---
title: Práctica 3
subtitle: Producto de matrices
author:
  - José Carlos Entrena Jiménez

toc: true
toc-depth: 2
numbersections: true

mainfont: Droid Serif
monofont: Source Code Pro

geometry: "a4paper, top=2.5cm, bottom=2.5cm, left=3cm, right=3cm"

header-includes:
  - \usepackage{graphicx}
  - \usepackage[spanish]{babel}
  - \usepackage{float}
---
\pagebreak

# Introducción.

En esta práctica, vamos a ver las ventajas de usar directivas MPI a la hora de calcular el producto de dos matrices cuadradas. A la hora de paralelizar este algoritmo he utilizado un reparto de tareas siguiendo un esquema _maestro-esclavo_, en el cual el maestro envía a cada esclavo la tarea que debe realizar, que en este caso son las filas que la matriz producto que debe calcular (o, equivalentemente, las filas de la primera matriz que va a utilizar). Una vez repartidas las tareas, el maestro también realiza cómputo, y se encarga de recibir los datos de los esclavos, agruparlos en una estructura de datos única y escribirlos en un fichero.

El producto de las matrices se ha calculado mediante un bucle que anida tres índices, lo que hace que tenga una complejidad computacional de $\mathcal{O}(n^3)$.

# Consideraciones

Debido a que la parte computacional es muy simple, no he intentado hacer ningún cambio para mejorar el código. La única prueba que hice fue alternar el orden de los bucles, para explotar el principio de localidad espacial de los datos. Aun así, las diferencias de tiempo de cómputo no eran significativas, por lo que finalmente decidí dejar la versión más eficiente teóricamente.

A la hora de resolver el problema, he realizado dos implementaciones: una completamente secuencial, contenida en el archivo _seq.cpp_, que no utiliza ningún tipo de directivas MPI, y otra paralela, en el archivo _main.cpp_. La versión paralela puede utilizarse como una versión secuencial si se utiliza un único proceso, aunque inicializará el entorno MPI, lo que causa un pequeño coste en cuanto al tiempo. Debido a esto, para el cálculo de la ganancia utilizaré los tiempos del programa secuencial para cuando tenga un solo proceso. El resto de tiempos serán del programa paralelo, y tendrá en cuenta la suma del tiempo de cómputo, de inicialización del entorno paralelo y de recepción de datos.

Las matrices utilizadas por el programa han sido generadas mediante un _script_ en **Ruby**, con nombre _script.rb_. Se usa mediante el comando de terminal _\`ruby script.rb ARG1 ARG2 ARG3 ARG4\`_ donde _ARG1_ y _ARG2_ son las filas y columnas de la matriz, respectivamente, _ARG3_ es el número tope para los valores de la matriz (_float_ entre $0$ y _ARG3_) y _ARG4_ es el nombre del fichero de salida. Las matrices contienen también dos números al principio que indican su tamaño.

## Cargas de trabajo

Las cargas de trabajo utilizadas han sido tres: matrices de tamaño $300$, $500$ y $700$. En cuanto al número de procesos, en ATCGRID se han hecho pruebas para $1$, $2$ y $3$ procesos.




# Datos recopilados en ATCGRID

\begin{table}[H]
\centering
\resizebox{\textwidth}{!}{
\begin{tabular}{|c|c|c|c|c|c|c|}

\hline

Procesos  & Tamaño & Tiempo creación & Tiempo cálculos     & Tiempo recepción & Tiempo total   & Ganancia           \\ \hline
1         &  300   &  -              &  2.584025317        &  -               &  2.584025317   &  1                 \\ \hline
2         &  300   &  0.128622314    &  1.336316280        &  0.013574667     &  1.478513261   &  1.74771872878048  \\ \hline
3         &  300   &  0.130662112    &  0.866368306        &  0.016662779     &  1.013693197   &  2.54911971851775  \\ \hline
1         &  500   &  -              &  11.929682498       &  -               &  11.929682498  &  1                 \\ \hline
2         &  500   &  0.129480718    &  6.152394482        &  0.028446932     &  6.310322132   &  1.89050293288577  \\ \hline
3         &  500   &  0.121061173    &  3.988587801        &  0.057451886     &  4.16710086    &  2.86282547478344  \\ \hline
1         &  700   &  -              &  32.851394920       &  -               &  32.851394920  &  1                 \\ \hline
2         &  700   &  0.120542498    &  16.548675473       &  0.101917300     &  16.771135271  &  1.95880567350771  \\ \hline
3         &  700   &  0.130824683    &  10.984513951       &  0.001329321     &  11.116667955  &  2.95514762633746  \\ \hline


\end{tabular}}
\end{table}

## Gráficas

### Datos ATCGRID

Vemos la ganancia para 2 procesos:

\begin{figure}[H]
\begin{centering}
\includegraphics[width=0.8\textwidth]{Ganancia2ATC.png}
\caption{Ganancia ATC para 2 procesos}
\end{centering}
\end{figure}

Y para 3 procesos:

\begin{figure}[H]
\begin{centering}
\includegraphics[width=0.8\textwidth]{Ganancia3ATC.png}
\caption{Ganancia ATC para 3 procesos}
\end{centering}
\end{figure}

# Conclusiones

Para los cálculos realizados, hemos visto que paralelizar obtiene ganancias en relación directa con el número de procesos cuando las matrices tienen un tamaño elevado, creciendo la ganancia conforme crece el tamaño de las matrices. Esto se debe, principalmente, a que los tiempos de inicialización y de recepción de datos pierden relevancia con respecto a los cálculos. Mientras que en las matrices de tamaño $300$, el tiempo de cómputo representa alrededor del $90\%$ del tiempo total, el porcentaje aumenta cuanto más aumenta la matriz, sin que el tiempo de transmisión de datos llegue a ser relevante en ninguno de los casos estudiados.
