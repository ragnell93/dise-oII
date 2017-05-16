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

pair <int,vector<int>> localSearch(int dim, vector<int> dist, vector<int> flujo, int tipo, int porcen =100){
    
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
    vector<int> mejorSolEncontrada(dim); //mantiene el mejor de la vecindad
    
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

    else if (tipo == 2){//Buscar la mejor solución de la vecindad
        do{
            mejorCostoAnterior = mejorCosto; // para determinar que se llegó a un óptimo local
            mejorSolEncontrada = sol;
            for (int i = 0; i < dim; i++){
                for (int j = i+1; j < dim; j++){
                    solAux = sol; //probablemente haya una mejor manera que no involucre copiar el vector
                    solAux[i] = sol[j];
                    solAux[j] = sol[i]; //intercambiamos dos elementos
                    /*Importante, hay que optimizar el cálculo del costo de los vecinos*/
                    costoActual = funCosto(dim,solAux,dist,flujo);
                    if (costoActual<mejorCosto){
                        mejorCosto = costoActual; //se actualiza el mejor costo
                        mejorSolEncontrada = solAux; //guarda la mejor solución del momento
                    }
                }
            }
            if (mejorCosto < mejorCostoAnterior){
                sol = mejorSolEncontrada; //se mueve al mejor vecino
            }
        } while (mejorCosto < mejorCostoAnterior); //se detiene cuando ya no hay mejoría
    }

    else if (tipo == 3){ //Busca el mejor en un porcentaje dado, por si la vecindad es muy grande

        int totalIter = dim * (dim + 1) / 2; //calcula el tamaño de la vecindad
        int numIter = porcen * totalIter / 100; //division de enteros
        int iterActual = 0; 
        do{
            mejorCostoAnterior = mejorCosto; // para determinar que se llegó a un óptimo local
            mejorSolEncontrada = sol;
            for (int i = 0; i < dim; i++){
                for (int j = i+1; j < dim; j++){
                    solAux = sol; //probablemente haya una mejor manera que no involucre copiar el vector
                    solAux[i] = sol[j];
                    solAux[j] = sol[i]; //intercambiamos dos elementos
                    /*Importante, hay que optimizar el cálculo del costo de los vecinos*/
                    costoActual = funCosto(dim,solAux,dist,flujo);
                    if (costoActual<mejorCosto){
                        mejorCosto = costoActual; //se actualiza el mejor costo
                        mejorSolEncontrada = solAux; //guarda la mejor solución del momento
                    }
                    iterActual++;
                    if (iterActual >= numIter){
                        break; //termina la busqueda
                    }
                }
                if (iterActual >= numIter){
                    break; //termina la busqueda
                }
            }
            if (mejorCosto < mejorCostoAnterior){
                sol = mejorSolEncontrada; //se mueve al mejor vecino
            }
        } while (mejorCosto < mejorCostoAnterior); //se detiene cuando ya no hay mejoría
    }

    else if (tipo == 4){ //elige un vecino aleatoriamente
        srand (time(NULL)); //inicializa la semilla
        int prim, seg;

        do{
            mejorCostoAnterior = mejorCosto; // para determinar que se llegó a un óptimo local

            prim = rand() % dim;
            do{ // asegrarse que el segundo elemento sea distinto al primero
                seg = rand() % dim;
            }while(seg == prim);

            solAux = sol; //probablemente haya una mejor manera que no involucre copiar el vector
            solAux[prim] = sol[seg];
            solAux[seg] = sol[prim]; //intercambiamos dos elementos
            /*Importante, hay que optimizar el cálculo del costo de los vecinos*/
            costoActual = funCosto(dim,solAux,dist,flujo);
            if (costoActual<mejorCosto){
                mejorCosto = costoActual; //se actualiza el mejor costo
                sol = solAux; //guarda la mejor solución del momento
            }
        } while (mejorCosto < mejorCostoAnterior); //se detiene cuando ya no hay mejoría
    }

    pair <int,vector<int>> pairSol = make_pair (mejorCosto,sol);
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
    pairSol = localSearch(dim,loc,suc,2);
    cout << pairSol.first << endl;

    for (int i = 0; i < dim; i++){
        cout << pairSol.second[i] << " ";
    }
    cout << endl;
    
    
    return 0;
}