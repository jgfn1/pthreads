/***
	*
	* @author victoraurelio.com
	* @since 13/11/16
	*
*/
#include <bits/stdc++.h>
#define MAX 1000000

using namespace std;

typedef pair<int,int> sparseElement;
vector< vector<sparseElement> > sparseMatrix;
int** denseMatrix;
int** resultMatrix;

int lengthSparseX, lengthSparseY,
    lengthDenseX, lengthDenseY,
    lengthResultX, lengthResultY;// dX is length on X orientation of dense matrix;  dY is length on Y orientation of dense matrix;



void readData();
void clearMatrix(int** m, int x, int y);
void multiplySparseWithDense();

int main(){
    readData();
    multiplySparseWithDense();
    for(int i=0; i<lengthResultX; i++){
        for(int j=0; j<lengthResultY; j++){
            printf("%d ",resultMatrix[i][j]);
        }
        printf("\n");
    }
}



void multiplySparseWithDense(){
    int tmpIndice;
    for (int i=0; i<sparseMatrix.size(); i++) {
        for (int j = 0; j < sparseMatrix[i].size(); j++) {
            tmpIndice = sparseMatrix[i][j].first;
            for (int k = 0; k < lengthResultY; k++) {
                resultMatrix[i][k] += (sparseMatrix[i][j].second * denseMatrix[tmpIndice][k]); // first is the index, and second is the value.
            }
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
  resultMatrix = (int**) malloc(lengthResultX * sizeof(int**));

  // Reading values of sparse matrix
  for(int i = 0; i < lengthSparseX; i++){
      // Allocating result matrix columns.
      resultMatrix[i] = (int*) malloc(lengthResultY * sizeof(int*));

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
  denseMatrix = (int**) malloc(lengthDenseX * sizeof(int**));
  // Reading values of dense matrix
  for(int i = 0; i < lengthDenseX; i++){
      // Allocating dense matrix columns.
      denseMatrix[i] = (int*) malloc(lengthDenseY * sizeof(int*));

      // Reading values:
      for(int j = 0; j < lengthDenseY; j++){
          cin >> a;
          denseMatrix[i][j] = a;
      }
  }

  clearMatrix(resultMatrix, lengthResultX, lengthResultY);
}

void clearMatrix(int** m, int x, int y){
    for(int i=0; i<x; i++)
      for(int j=0; j<y; j++)
          m[i][j] = 0;
}