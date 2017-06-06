#include <bits/stdc++.h>
#define initial_guess 1
#define P 10
#define n 3

using namespace std;

vector< vector<int> > A;
vector<int> X;
vector<int> B;

//Number of processors on the machine, thus, number of threads.
int threads_number; //N

void *jacobi(int index);

int main() {
    cin >> threads_number;

    vector<thread> threads;
    for (int i = 0; i < threads_number; ++i) {
        threads.push_back ( thread(jacobi, i) );
    }
    return 0;
}

void *jacobi(int index) {
    int k = 0;
    while(k < P){
        for (int i = 0; i < n; ++i) {
            int gama = 0;
            for (int j = 0; j < n; ++j) {
                if(j != i) {
                    gama = gama + (A[i][j] * (X[j] ^ k));
                }
            }
            (X[i] ^ (k + 1)) = (1/A[i][i]) * (B[i] - gama);
        }
        k++;
    }
    return nullptr;
}