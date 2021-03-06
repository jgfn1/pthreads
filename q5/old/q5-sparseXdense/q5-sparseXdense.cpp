/***
	*
	* @author victoraurelio.com
	* @since 13/11/16
	*
*/
#include <bits/stdc++.h>
#include <thread>
#define THREADS 3

using namespace std;

// TO COMPILE:
    // g++ q5.cpp -std=c++0x -lpthread -o e
    // g++ q5.cpp -std=c++0x -pthread -o e

typedef pair<int,double> sparseElement;
vector< vector<sparseElement> > sparseMatrix;
double** denseMatrix;
double** resultMatrix;

int lengthSparseX, lengthSparseY,
    lengthDenseX, lengthDenseY,
    lengthResultX, lengthResultY;// dX is length on X orientation of dense matrix;  dY is length on Y orientation of dense matrix;



void readData();
void showResultMatrix();
void clearMatrix(double** m, int x, int y);
void *multiplySparseWithDense(int id);

int main(){
    readData();

    // Allocating one thread for each line of matrix
    vector< thread > threads;
    for(int i=0; i<lengthResultX; i++){
        threads.push_back( thread(multiplySparseWithDense, i) );
    }

    // Waiting each thread stop
    for(int i=0; i<lengthResultX; i++){
      threads[i].join();
    }

    // Show the result
    showResultMatrix();

}



void *multiplySparseWithDense(int id){
    int tmpIndice;
    printf("Hi, I'm thread %d going calculate %d line\n", id, id);
    for (int j = 0; j < sparseMatrix[id].size(); j++) {
        tmpIndice = sparseMatrix[id][j].first;
        for (int k = 0; k < lengthResultY; k++) {
            resultMatrix[id][k] += (sparseMatrix[id][j].second * denseMatrix[tmpIndice][k]); // first is the index, and second is the value.
        }
    }
}

void readData(){
  int elements, a,b;
  // Read sparse matrix dimensions
  cin >> lengthSparseX >> lengthSparseY;
  // Read dense matrix dimensions
  cin >> lengthDenseX >> lengthDenseY;
  // Set the result matrix dimensions
  lengthResultX = lengthSparseX;  // result has the same number of lines wich the first matrix
  lengthResultY = lengthDenseY;   // result has the same number of columns wich the second matrix

  // Allocating result matrix lines.
  resultMatrix = (double**) malloc(lengthResultX * sizeof(double**));

  // Reading values of sparse matrix
  for(int i = 0; i < lengthSparseX; i++){
      // Allocating result matrix columns.
      resultMatrix[i] = (double*) malloc(lengthResultY * sizeof(double*));

      // Reading values:

      cin >> elements; // Reading quantity of itens in this line of sparse matrix.
      vector<sparseElement> tmp;// Declaring new vector.
      for (int j = 0; j < elements; j++){
          cin >> a >> b;
          //(to_unprofessional_debug) printf(" [%d][%d] == %d ", i,a,b);
          tmp.push_back(make_pair(a,b));// Adding indice and value on new vector.
      }
      //(to_unprofessional_debug)printf("\n");
      sparseMatrix.push_back(tmp); // Adding our new vector into sparseMatrix that is a vector. (or better, that is a matrix!)
  }

  // Allocating dense matrix lines.
  denseMatrix = (double**) malloc(lengthDenseX * sizeof(double**));
  // Reading values of dense matrix
  for(int i = 0; i < lengthDenseX; i++){
      // Allocating dense matrix columns.
      denseMatrix[i] = (double*) malloc(lengthDenseY * sizeof(double*));

      // Reading values:
      for(int j = 0; j < lengthDenseY; j++){
          cin >> a;
          denseMatrix[i][j] = a;
      }
  }

  clearMatrix(resultMatrix, lengthResultX, lengthResultY);
}

void clearMatrix(double** m, int x, int y){
    for(int i=0; i<x; i++)
      for(int j=0; j<y; j++)
          m[i][j] = 0;
}

void showResultMatrix(){
    for(int i=0; i<lengthResultX; i++){
        for(int j=0; j<lengthResultY; j++){
            printf("%.1lf\t",resultMatrix[i][j]);
        }
        printf("\n");
    }
}