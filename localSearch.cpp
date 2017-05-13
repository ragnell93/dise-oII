#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>  //ifstream
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <algorithm> //shuffle
#include <random> //default_random_engine
#include <chrono> //chrono::system_clock
#include <cstdlib> // rand
using namespace std;

int funCosto(int dim,vector<int> sol,vector<int> dist, vector<int> flujo){
    int costo = 0;
    for (int i = 0; i<dim;i++){
        for (int j = 0; j<dim;j++){
            // El costo es la * del valor de los flujos por la permutacion de las distancias 
            costo += flujo[dim*i+j] * dist[dim*(sol[i]-1) + (sol[j]-1)]; 
        }
    }
    return costo;
}

vector<int> localSearch(int dim, vector<int> dist, vector<int> flujo, int tipo){
    
    vector<int> sol(dim);
    //inicializamos el vector con las localidades ordenadas
    for (int i = 1; i <= dim; i++){
        sol[i-1] = i;
    }
    //obtener una semilla basada en el tiempo
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    shuffle(sol.begin(),sol.end(), default_random_engine(seed)); //solución inicial para la búsqueda

    int mejorCosto = funCosto(dim,sol,dist,flujo);
    int costoActual,mejorCostoAnterior;
    vector<int> solAux(dim); //para realizar las pruebas con los vecinos
    
    if (tipo == 1){ // Buscar primera solución que mejore
        
        //generar la vecindad y buscar la primera solución factible
        do{
            mejorCostoAnterior = mejorCosto; // para determinar que se llegó a un óptimo local
            for (int i = 0; i < dim; i++){
                for (int j = i+1; j < dim; j++){
                    solAux = sol; //probablemente haya una mejor manera que no involucre copiar el vector
                    solAux[i] = sol[j];
                    solAux[j] = sol[i]; //intercambiamos dos elementos
                    /*Importante, hay que optimizar el cálculo del costo de los vecinos*/
                    costoActual = funCosto(dim,solAux,dist,flujo);
                    if (costoActual<mejorCosto){
                        break;
                    }
                }
                if (costoActual < mejorCosto){
                    mejorCosto = costoActual; //se actualiza el mejor costo
                    sol = solAux; //se efectua el movimiento
                    break; 
                }
            }
        } while (mejorCosto < mejorCostoAnterior); //se detiene cuando ya no hay mejoría
    }

    cout << mejorCosto << endl;
    return sol;
}


int main (int argc, char* argv[]) {
    
    ifstream file(argv[1]);
    int dim;  //dimensiones de las matrices
    file >> dim;
    vector<int> suc(dim*dim); //matriz con los flujos entre las sucursales
    vector<int> loc(dim*dim); //matriz con las distancias de las localidades

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

    /*probar el almacenado de las matrices
    for (int i = 0; i < dim; i++){ 
        for (int j = 0; j < dim; j++) {
            cout << loc[dim*i+j] << " ";
        }
        cout << endl;
    }
    */

    //mostrar la solución dada por localSearch
    vector<int> solucion(dim);
    solucion = localSearch(dim,loc,suc,1);

    for (int i = 0; i < dim; i++){
        cout << solucion[i] << " ";
    }
    cout << endl;
    
    
    return 0;
}