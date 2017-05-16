#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>  //ifstream
#include <vector>
#include <utility> //pair
#include <algorithm> //shuffle
#include <random> //default_random_engine
#include <chrono> //chrono::system_clock
#include <cstdlib> // rand
#include <cmath> //exp
using namespace std;

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

pair <int,vector<int>> simAnnealing(int dim, vector<int> dist, vector<int> flujo){

    vector<int> sol(dim); //mejor solucion encontrada
    //inicializamos el vector con las localidades ordenadas
    for (int i = 1; i <= dim; i++){
        sol[i-1] = i;
    }
    //obtener una semilla basada en el tiempo
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    shuffle(sol.begin(),sol.end(), default_random_engine(seed)); //solución inicial para la búsqueda

    vector<int> solActual(dim);
    solActual = sol;
    vector<int> solAux(dim); //para evaluar los vecinos
    int temp = 100000; //temperatura inicial, este es uno de los parámetros que hay que ajustar constantemente
    int tempFinal = 10000; //temperatura final, otro parámetro que hay que ajustar
    int cambioTemp = 0; // al llegar a cierto numero efectua el enfriamiento
    int prim, seg, acepta;
    int difCostos; //almacena la diferencia de costos entre la sol actual y la candidata
    srand (time(NULL)); //inicializa la semilla
    
    while (temp > tempFinal){  
        while (cambioTemp < 100){ //este número también es para ajustar, puede usarse un preceso de enfriamiento más complicado

            prim = rand() % dim;
            do{ // asegurarse que el segundo elemento sea distinto al primero
                seg = rand() % dim;
            }while(seg == prim);
        
            solAux = solActual; //probablemente haya una mejor manera que no involucre copiar el vector
            solAux[prim] = solActual[seg];
            solAux[seg] = solActual[prim]; //intercambiamos dos elementos

            difCostos = funCosto(dim,solAux,dist,flujo) - funCosto(dim,solActual,dist,flujo);
            if (difCostos <= 0){
                //si es mejor solución que la actual se mueve a ella
                solActual = solAux;
                if (funCosto(dim,solActual,dist,flujo) < funCosto(dim,sol,dist,flujo)){
                    //verifica si es la mejor respuesta encontrada hasta el momento
                    sol = solActual;
                }
            }
            else if (rand() < exp(-difCostos/temp)){ //acepta con probabilidad e^(difCosts/temp)
                solActual = solAux;
            }
            cambioTemp++;
        }
        temp = 0.9*temp; //enfriamiento como función geométrica
    }
    pair <int,vector<int>> pairSol = make_pair (funCosto(dim,sol,dist,flujo),sol);
    return pairSol;

}

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
    pairSol = simAnnealing(dim,loc,suc);
    cout << pairSol.first << endl;

    for (int i = 0; i < dim; i++){
        cout << pairSol.second[i] << " ";
    }
    cout << endl;
    
    
    return 0;
}