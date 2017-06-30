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
#include <unistd.h>
#include <csignal>
using namespace std;

sig_atomic_t volatile done = 0;
void game_over(int) { done = 1; }

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

    done = 0;
    std::signal(SIGALRM, game_over);
    alarm(2); // permite que el la busqueda tabú se realice por cierto tiempo

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
    int tempFinal = 1000; //temperatura final, otro parámetro que hay que ajustar
    int sinCambio; // al llegar a cierto numero efectua el enfriamiento
    int prim, seg, acepta;
    int difCostos; //almacena la diferencia de costos entre la sol actual y la candidata
    random_device rd; // obtener un número aleatorio de hardware
    mt19937 eng(rd()); // semilla para el generador
    uniform_int_distribution<> disInt(0,dim-1); // rango permitido en el movivimiento
    uniform_real_distribution<> disReal(0,1); //necesario para la aceptación de una peor solución
    
    while ((temp > tempFinal) & !done){  
        sinCambio = 0;
        while ((sinCambio < 1000) & !done){ //este número también es para ajustar, puede usarse un preceso de enfriamiento más complicado

            prim = disInt(eng);
            do{ // asegurarse que el segundo elemento sea distinto al primero
                seg = disInt(eng);
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
                    sinCambio = 0; //reinicia el contador
                }
            }

            if (disReal(eng) < exp(-difCostos/temp)){ //acepta con probabilidad e^(difCosts/temp)
                solActual = solAux;
            }
            sinCambio++;
        }
        /*for (int i = 0; i < dim; i++){
            cout << solActual[i] << " ";
        }
        cout << endl;*/
        temp = 0.95*temp; //enfriamiento como función geométrica
    }
    pair <int,vector<int>> pairSol = make_pair (funCosto(dim,sol,dist,flujo),sol);
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

        //mostrar la solución dada por simulated Annealing
        pairSol = simAnnealing(dim,loc,suc);
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