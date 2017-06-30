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
#include <limits> //max
#include <unistd.h>
#include <csignal>
using namespace std;

sig_atomic_t volatile done = 0;
void game_over(int) { done = 1; }

int busquedaListaTabu(vector<pair<int,int>> tabu,int i, int j){
    //Busca si una permutación entre i y j es tabu
    vector<pair<int,int>>::iterator it;
    for (it = tabu.begin(); it != tabu.end(); it++){
        if (((it->first == i) && (it->second == j)) || ((it->first == j) && (it->second == i))){
            return 1; //fue encontrado
        }
    }
    return 0; //no se encontró
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

pair <int,vector<int>> tabuSearch(int dim, vector<int> dist, vector<int> flujo){

    done = 0;
    std::signal(SIGALRM, game_over);
    alarm(10); // permite que el la busqueda tabú se realice por cierto tiempo
    
    vector<int> solActual(dim);
    //inicializamos el vector con las localidades ordenadas
    for (int i = 1; i <= dim; i++){
        solActual[i-1] = i;
    }

    vector<pair<int,int>> tabu(10); //lista con las permutaciones prohibidas. Es circular
    int contador = 0; //contador para ubicar cada nuevo par de posiciones en la lista tabú usando mod.

    //obtener una semilla basada en el tiempo
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    shuffle(solActual.begin(),solActual.end(), default_random_engine(seed)); //solución inicial para la búsqueda

    vector<int> mejorSol(dim); //mejor solución encontrada
    mejorSol = solActual; //Se comienza con la inicial
    vector<int> solAux(dim); //para realizar las pruebas con los vecinos
    vector<int> mejorVecindad(dim); //mantiene el mejor de la vecindad
    int sinMejoria = 0; //contador que determina la condición de parada de la búsqueda tabú.
    int costoAux; //costo de solAux
    int mejorCosto = funCosto(dim,mejorSol,dist,flujo);
    int mejorCostoVecindad; //mejor de la vecindad, no necesariamente tiene que mejorar la solución final.
    int primero, segundo, flag; //indices del movimiento a agregar en la lista tabu.

    while ((sinMejoria < 1000) & !done){ //condición de parada. Ajustar
        mejorCostoVecindad = std::numeric_limits<int>::max(); //de tal manera que el primer vecino lo reemplace
        flag = 0;
        for (int i = 0; i < dim; i++){
            for (int j = i+1; j < dim; j++){
                solAux = solActual; //probablemente haya una mejor manera que no involucre copiar el vector
                solAux[i] = solActual[j];
                solAux[j] = solActual[i]; //intercambiamos dos elementos
                costoAux = funCosto(dim,solAux,dist,flujo);

                if (costoAux<mejorCosto){ //si es menor a la mejor solución no importa que sea tabú, se mueve a él
                    mejorSol = solAux; //actualiza la mejor solución encontrada
                    mejorCosto = costoAux;
                    sinMejoria = 0; //resetea el contador
                    mejorVecindad = solAux;
                    mejorCostoVecindad = costoAux;
                    primero = i;
                    segundo = j;
                    flag = 1;
                    break;
                }
                else if ((busquedaListaTabu(tabu,i,j) == 0) && (costoAux < mejorCostoVecindad)){
                    //si el movimiento es permitido y es el mejor encontrado por el momento de la vecindad
                    mejorVecindad = solAux;
                    mejorCostoVecindad = costoAux; 
                    primero = i;
                    segundo = j;
                }
            }
            if (flag == 1){break;}
        }
        solActual = mejorVecindad; //se mueve al mejor vecino encontrado permitido
        tabu[contador%10] = make_pair(primero,segundo); //lista circular que actualiza las entradas mas viejas
        contador++;
        sinMejoria++; 
    }

    pair <int,vector<int>> pairSol = make_pair (mejorCosto,mejorSol);
    return pairSol;
}

int main (int argc, char* argv[]) {
    
    vector<int> resultados(10);

    for (int i = 0; i < 10; i++){

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

        //mostrar la solución dada por ltabuSearch
        pairSol = tabuSearch(dim,loc,suc);
        resultados[i] = pairSol.first;
        
        cout << pairSol.first << endl;

        for (int i = 0; i < dim; i++){
            cout << pairSol.second[i] << " ";
        }
        cout << endl;
        
        cout << double( clock() - startTime ) / (double)CLOCKS_PER_SEC<< " seconds." << endl;

    }

    int total = 0;
    for (int j = 0; j<10; j++){
        total += resultados[j];
    }

    cout << endl << "El promedio de de las soluciones es: " <<endl;
    cout << total/10 << endl;
    return 0;
}