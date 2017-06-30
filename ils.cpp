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
#include <unistd.h>
#include <csignal>
using namespace std;

sig_atomic_t volatile done = 0;
void game_over(int) {done = 1; }

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

pair <int,vector<int>> iteratedLocalSearch(int dim, vector<int> dist, vector<int> flujo){

    done = 0;
    std::signal(SIGALRM, game_over);
    alarm(2); // permite que el la busqueda tabú se realice por cierto tiempo
    
    vector<int> mejorSol(dim);
    //inicializamos el vector con las localidades ordenadas
    for (int i = 1; i <= dim; i++){
        mejorSol[i-1] = i;
    }
    //obtener una semilla basada en el tiempo
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    shuffle(mejorSol.begin(),mejorSol.end(), default_random_engine(seed)); //solución inicial para la búsqueda

    int prim,seg,y;
    vector<int> solActual(dim);
    solActual = mejorSol;
    vector<int> solAnterior(dim);
    int mejorCosto = funCosto(dim,solActual,dist,flujo);
    int costoActual,mejorCostoAnterior,difCostos;
    vector<int> solAux(dim); //para realizar las pruebas con los vecinos
    int sinMejorias = 0;
    int temp = 100000;
    int tempFinal = 1000;
    random_device rd; // obtener un número aleatorio de hardware
    mt19937 eng(rd()); // semilla para el generador
    uniform_int_distribution<> disInt(0,dim-1); // rango permitido en el movivimiento
    uniform_real_distribution<> disReal(0,1); //necesario para la aceptación de una peor solución

    while ((temp > tempFinal) & !done){
        while ((sinMejorias < 20) & !done){
            solAnterior = solActual;
            do{
                mejorCostoAnterior = mejorCosto; // para determinar que se llegó a un óptimo local
                for (int i = 0; i < dim; i++){
                    for (int j = i+1; j < dim; j++){
                        solAux = solActual;
                        solAux[i] = solActual[j];
                        solAux[j] = solActual[i]; //intercambiamos dos elementos
                        /*Importante, hay que optimizar el cálculo del costo de los vecinos*/
                        costoActual = funCosto(dim,solAux,dist,flujo);
                        if (costoActual<mejorCosto){
                            break;
                        }
                    }
                    if (costoActual < mejorCosto){
                        mejorCosto = costoActual; //se actualiza el mejor costo
                        solActual = solAux; //se efectua el movimiento
                        break; 
                    }
                }
            } while (mejorCosto < mejorCostoAnterior); //se detiene cuando ya no hay mejoría

            if (mejorCosto < funCosto(dim,mejorSol,dist,flujo)){
                mejorSol = solActual;
                sinMejorias = 0;
            }

            difCostos = mejorCosto - funCosto(dim,solAnterior,dist,flujo);
            if ((difCostos > 0) && (disReal(eng) > exp(-difCostos/temp))){
                solActual = solAnterior; //rechaza la solucion y regresa a la anterior
            }

            prim = disInt(eng);
            do{ // asegurarse que el segundo elemento sea distinto al primero
                seg = disInt(eng);
            }while(seg == prim);

            solAux = solActual;
            if (prim < seg){
                y = prim;
                for (int x = seg; x >= prim; x--){
                    solActual[y] = solAux[x]; //2-opt
                    y++;
                }
            }
            else {
                y = seg;
                for (int x = prim; x >= seg; x--){
                    solActual[y] = solAux[x]; //2-opt
                    y++;
                }
            }

            sinMejorias++;
        }
        sinMejorias = 0;
        temp = temp*0.80;
    }
    pair <int,vector<int>> pairSol = make_pair (funCosto(dim,mejorSol,dist,flujo),mejorSol);
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

        //mostrar la solución dada por localSearch
        pairSol = iteratedLocalSearch(dim,loc,suc);
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