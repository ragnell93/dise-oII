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
using namespace std;

int partition(vector<pair<int,int>>* costos,int p,int r){
    int x = (*costos)[r].second;
    int i = p-1;
    pair<int,int> aux;
    for (int j = p; j < r; j++){
        if ((*costos)[j].second <= x){
            i++;
            aux = (*costos)[i];
            (*costos)[i] = (*costos)[j];
            (*costos)[j] = aux;
        }
    }
    aux = (*costos)[i+1];
    (*costos)[i+1] = (*costos)[r];
    (*costos)[r] = aux;
    return i+1; 
}

void quicksort(vector<pair<int,int>>* costos, int p,int r){
    if (p < r){
        int q = partition(costos,p,r);
        quicksort(costos,p,q-1);
        quicksort(costos,q+1,r);
    }
}

int funCosto(int dim,vector<int> sol,vector<int> dist, vector<int> flujo){
    int costo = 0;
    for (int i = 0; i<dim;i++){
        for (int j = i+1; j<dim;j++){
            // El costo es la * del valor de los flujos por la permutacion de las distancias 
            costo += flujo[dim*i+j] * dist[dim*(sol[i]-1) + (sol[j]-1)]; 
        }
    }
    return costo*2; //La matriz es simétrica
}

int geneticAlgorithm(int tamPob,int dim, vector<int> dist, vector<int> flujo){

    vector<vector<int>> poblacion(tamPob, vector<int>(dim));
    vector<vector<int>> hijos(tamPob, vector<int>(dim));
    vector<vector<int>> auxPob(tamPob, vector<int>(dim)); //Auxiliar para el reordenamiento de elementos
    vector<pair<int,int>> costos(tamPob);
    random_device rd; // obtener un número aleatorio de hardware
    mt19937 eng(rd()); // semilla para el generador
    uniform_int_distribution<> disInt(0,dim-1); // rango permitido en el movivimiento
    uniform_real_distribution<> disReal(0,1); //necesario para la aceptación de una peor solución
    int sinMejoria = 0;
    int indice,z;

    double gamma = 1.5; // presión de selección = P(elegir mejor respuesta)/P(elegir media)
    double alpha = (2*tamPob - gamma*(tamPob+1))/(tamPob*(tamPob-1));
    double beta = (2 * (gamma-1))/(tamPob * (tamPob-1));
 
    for (int i = 1; i <= dim; i++){
        poblacion[0][i-1] = i;
    }
    for (int j = 1; j < tamPob; j++){
        poblacion[j] = poblacion[0];
    }
    //obtener una semilla basada en el tiempo
    for (int k = 0; k < tamPob; k++){
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        shuffle(poblacion[k].begin(),poblacion[k].end(), default_random_engine(seed));
    } 

    /*
    for (int l = 0; l < tamPob; l++){
        costos[l] = make_pair(l,funCosto(dim,poblacion[l],dist,flujo));
    }

    quicksort(&costos,0,tamPob-1);
    auxPob = poblacion;
    int z=0;
    for (int i = tamPob-1; i>=0 ; i--){
        poblacion[i] = auxPob[costos[z].first]; //Ordenados los elementos de menor a mayor costo
        z++;
    }*/

    while (sinMejoria < 2){
        //consutruye el arreglo de costos para la población actual
        for (int l = 0; l < tamPob; l++){
            costos[l] = make_pair(l,funCosto(dim,poblacion[l],dist,flujo));
        }
        //ordena los elementos de peor a mejor
        quicksort(&costos,0,tamPob-1);
        auxPob = poblacion;
        z=0;
        for (int i = tamPob-1; i>=0 ; i--){
            poblacion[i] = auxPob[costos[z].first]; //Ordenados los elementos de menor a mayor costo
            z++;
        }

        /*Proceso de selección de los cromosomas que se van a reproducir. Tiene mayor oportunidad aquellos
          que tienen mejor función de costos*/
        indice = (-(2*alpha+beta)+sqrt(((2*alpha+beta)*(2*alpha+beta)) + (4*beta*disReal(eng))))/(2*beta);
        
        cout << indice << endl;
        sinMejoria++;
        
    }

    /* NO ESTA DIVERSIFICANDO CORRECTAMENTE
    //Inicializamos la población de manera diversificada
    for (int i = 1; i < 10; i++){
        for (int j = 0; j < dim; j++){
            poblacion[i][j] = poblacion[i-1][poblacion[i-1][j]-1];
        }
    }
    */
   
    /*
    for (int i = 0; i < tamPob; i++){
        for (int j = 0; j < dim; j++){
            cout << poblacion[i][j] << " ";
        }
        cout <<endl;
    }*/

    return 1;
}

int main (int argc, char* argv[]) {
  
    clock_t startTime = clock();
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

    geneticAlgorithm(10,dim,loc,suc);
    return 0;
}