#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>  //ifstream
#include <string>
#include <sstream>
#include <vector>
#include <utility> //pair
#include <iostream>
#include <algorithm> //shuffle
#include <random> //default_random_engine
#include <chrono> //chrono::system_clock
#include <cstdlib> // rand
#include "localSearch.h"
using namespace std;

int main (int argc, char* argv[]) {
    
    ifstream file(argv[1]);
    int dim;  //dimensiones de las matrices
    file >> dim;
    vector<int> suc(dim*dim); //matriz con los flujos entre las sucursales
    vector<int> loc(dim*dim); //matriz con las distancias de las localidades
    pair <int,vector<int>> pairSol; //tiene el costo de la busqueda y la permutación

    //guardar la matriz de distancia
    for (int i = 0; i < dim; i++){ 
        for (int j = 0; j < dim; j++) {
            file >> suc[dim*i+j];
        }
    }

    //guardar la matriz de flujos
    for (int i = 0; i < dim; i++){ 
        for (int j = 0; j < dim; j++) {
            file >> loc[dim*i+j];
        }
    }


    //mostrar la solución dada por localSearch
    pairSol = localSearch(dim,loc,suc,2);
    cout << pairSol.first << endl;

    for (int i = 0; i < dim; i++){
        cout << pairSol.second[i] << " ";
    }
    cout << endl;
    
    
    return 0;
}