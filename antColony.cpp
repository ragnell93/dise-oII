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

int buscarLista(vector<int> lista, int elemento, int dim){
    int aux = 0;
    for (int i = 0; i < dim; i++){
        if (lista[i] == elemento){aux = 1;}
    }
    return aux;
}

vector<int> localSearch(int dim,vector<int> sol, vector<int> dist, vector<int> flujo){

    vector<int> solActual(dim), solAux(dim);
    solActual = sol;
    int mejorCosto = funCosto(dim,solActual,dist,flujo);
    int costoActual, mejorCostoAnterior;

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

    return solActual;
}

pair<int,vector<int>> antColony(int numAnts, int dim, vector<int> dist, vector<int> flujo){

    done = 0;
    std::signal(SIGALRM, game_over);
    alarm(300);

    vector<vector<int>> hormigas(numAnts, vector<int>(dim));
    vector<vector<int>> hormigasNuevas(numAnts, vector<int>(dim));
    vector<vector<double>> feromonas(dim, vector<double>(dim));
    vector<int> mejorSol, mejorSolNueva;
    double mejorCosto, mejorCostoNuevo, totalFer, prob, auxProb, probAcum = 0 ;
    int numSwaps = dim/3;  // número de cambios que va a hacer cada hormiga en cada iteración
    random_device rd; // obtener un número aleatorio de hardware
    mt19937 eng(rd()); // semilla para el generador
    uniform_int_distribution<> disInt(0,dim-1);
    uniform_real_distribution<> disReal(0,1);
    int prim, seg, aux, contadorRep, sinMejoria; //indices 
    int inten = 1; //la intensificación está prendida al comienzo
    for (int i = 1; i <= dim; i++){
        hormigas[0][i-1] = i;
    }
    for (int j = 1; j < numAnts; j++){
        hormigas[j] = hormigas[0];
    }
    //obtener una semilla basada en el tiempo
    for (int k = 0; k < numAnts; k++){
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        shuffle(hormigas[k].begin(),hormigas[k].end(), default_random_engine(seed));
    } 

    for (int i = 0; i < numAnts; i++){ //aplica local Search para las soluciones aleatorias
        hormigas[i] = localSearch(dim,hormigas[i],dist,flujo);
    }

    mejorCosto = funCosto(dim,hormigas[0],dist,flujo);
    mejorSol = hormigas[0];
    for (int i = 1; i < numAnts; i++){
        if (funCosto(dim,hormigas[i],dist,flujo) < mejorCosto){
            mejorCosto = funCosto(dim,hormigas[i],dist,flujo);
            mejorSol = hormigas[i];
        }
    }
    for (int i = 0; i<dim; i++){
        for (int j = 0 ; j<dim; j++){
            feromonas[i][j] = 1.0/(100*mejorCosto);
        }
    }

    for (int z = 0; z < 10; z++){ //máximo número de iteraciones

        for (int i = 0; i < numAnts; i++ ){

            hormigasNuevas[i] = hormigas[i];

            for (int j = 0; j < numSwaps; j++){
                prim = disInt(eng);
                auxProb = disReal(eng);
                seg = -1;
                probAcum = 0;
                do{
                    seg++;
                    if (seg == prim){seg++;}
                    totalFer = 0; //limpiamos para esta iteración
                    for (int k = 0; k < dim; k++){
                        if (k != prim){
                            totalFer += feromonas[prim][hormigasNuevas[i][k]-1] + feromonas[k][hormigasNuevas[i][prim]-1];
                        }
                    }    
                    //construimos la probabilidad con la que es aceptado el cambio segun las feromonas
                    prob = (feromonas[prim][hormigasNuevas[i][seg]-1]+feromonas[seg][hormigasNuevas[i][prim]-1]) / totalFer;
                    probAcum += prob;
                }while ((auxProb > probAcum) && (seg < dim));

                if (seg == dim){seg--;}

                aux = hormigasNuevas[i][prim];
                hormigasNuevas[i][prim] = hormigasNuevas[i][seg];
                hormigasNuevas[i][seg] = aux;

            }
            //mejoramos la solución modificada por la hormiga
            hormigasNuevas[i] = localSearch(dim,hormigasNuevas[i],dist,flujo);
        }

        //intensificación
        for (int a = 0; a < numAnts; a++){
            if (inten == 1){
                if (funCosto(dim,hormigasNuevas[a],dist,flujo) < funCosto(dim,hormigas[a],dist,flujo)){
                    hormigas[a] = hormigasNuevas[a];
                }
                else{ contadorRep++;} //para contar cuantas veces se queda con la respuesta anterior
            }
            else{
                hormigas[a] = hormigasNuevas[a];
            }
        }

        if (contadorRep == numAnts){inten = 0;} //apaga la intensificación

        mejorCostoNuevo = funCosto(dim,hormigas[0],dist,flujo);
        for (int b = 1; b < numAnts; b++){
            if (funCosto(dim,hormigas[b],dist,flujo) < mejorCostoNuevo){
                mejorCostoNuevo = funCosto(dim,hormigas[b],dist,flujo);
                mejorSolNueva = hormigas[b];
            }
        }

        if (mejorCostoNuevo < mejorCosto){
            //si se consigue una mejor solucíón
            mejorCosto = mejorCostoNuevo;
            mejorSol = mejorSolNueva;
            inten = 1;
            sinMejoria = 0;
        }

        //actualiación de la matriz de feromonas
        for (int c = 0; c < dim; c++){
            for (int d = 0; d < dim; d++){ //Evaporación
                feromonas[c][d] = 0.9*feromonas[c][d];
            }
        }

        for (int e = 0; e < dim; e++){//reforzamos la mejor solución
            feromonas[e][mejorSol[e]-1] = feromonas[e][mejorSol[e]-1] + 0.1/mejorCosto;
        }

        //diversificación
        if (sinMejoria > dim/2){
            
            hormigas[0] = mejorSol; //se queda la mejor solución hasta el momento
            //se vuelve a inicializar las hormigas
            for (int k = 1; k < numAnts; k++){
                unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
                shuffle(hormigas[k].begin(),hormigas[k].end(), default_random_engine(seed));
            } 

            for (int i = 0; i < numAnts; i++){ //aplica local Search para las soluciones aleatorias
                hormigas[i] = localSearch(dim,hormigas[i],dist,flujo);
            }

            mejorCosto = funCosto(dim,hormigas[0],dist,flujo);
            mejorSol = hormigas[0];
            for (int i = 1; i < numAnts; i++){
                if (funCosto(dim,hormigas[i],dist,flujo) < mejorCosto){
                    mejorCosto = funCosto(dim,hormigas[i],dist,flujo);
                    mejorSol = hormigas[i];
                }
            }

            //renovar la matriz de feromonas
            for (int i = 0; i<dim; i++){
                for (int j = 0 ; j<dim; j++){
                    feromonas[i][j] = 1.0/(100*mejorCosto);
                }
            }
        }

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

        pairSol = antColony(10,dim,loc,suc);
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