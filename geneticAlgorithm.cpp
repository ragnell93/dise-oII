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

int buscarLista(vector<int> lista, int elemento, int dim){
    int aux = 0;
    for (int i = 0; i < dim; i++){
        if (lista[i] == elemento){aux = 1;}
    }
    return aux;
}

pair<int,vector<int>>geneticAlgorithm(int tamPob,int dim, vector<int> dist, vector<int> flujo){

    done = 0;
    std::signal(SIGALRM, game_over);
    alarm(200);

    vector<vector<int>> poblacion(tamPob, vector<int>(dim));
    vector<vector<int>> hijos(2, vector<int>(dim));
    vector<vector<int>> auxPob(tamPob, vector<int>(dim)); //Auxiliar para el reordenamiento de elementos
    vector<pair<int,int>> costos(tamPob);
    vector<int> mejorSol(dim);
    vector<int> vectorAux(dim); //para cuando se hagan los cambios de posiciones al insertar un hijo
    random_device rd; // obtener un número aleatorio de hardware
    mt19937 eng(rd()); // semilla para el generador
    uniform_int_distribution<> disInt1(0,dim-1);
    uniform_int_distribution<> disInt2(0,tamPob-1); 
    uniform_real_distribution<> disReal(0,1);
    int sinMejoria = 0;
    int padre1, padre2 = 10; //indices de los respectivos cromosomas
    int aux1,aux2,aux3; //auxiliares para guardar los índices generados aleatoriamente
    int mejorCosto;
    int minHijo, maxHijo, maxPadre, minPadre, costoMaxPadre, costoMinPadre, costoMaxHijo, costoMinHijo;
    int primero, segundo; //indices para la mutacion

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

    for (int l = 0; l < tamPob; l++){
        costos[l] = make_pair(l,funCosto(dim,poblacion[l],dist,flujo));
    }

    quicksort(&costos,0,tamPob-1);
    auxPob = poblacion;
    for (int i = 0; i<tamPob ; i++){
        poblacion[i] = auxPob[costos[i].first]; //Ordenados los elementos de menor a mayor costo
    }

    mejorSol = poblacion[0];
    mejorCosto = funCosto(dim,mejorSol,dist,flujo);

    while ((sinMejoria < 30000) && !done){

        for (int i= 0; i < 3; i++){
            /*torneo de 3 elementos,como estan ordenados 
            con solo elegir el de menor indice se tiene el mejor*/
            aux1 = disInt2(eng);
            aux2 = disInt2(eng);
            if (aux1 < padre1){padre1 = aux1;}
            if (aux2 < padre2){padre2 = aux2;} 
            if (padre1 == padre2){
                do{ //para evitar que sean los mismos padres
                    padre2 = disInt2(eng);
                }while(padre1 == padre2);
            } 
        }

        /*cruce OX, elige aleatoriamente los elementos que se heredan del padre 1 y el resto 
        lo completa con los del padre 2*/
        for (int i = 0; i < dim; i++){
            //limpia los hijos antes de usarlos
            hijos[0][i] = 0;
            hijos[1][i] = 0;
        }

        aux1 = disInt1(eng);
        for (int i = 0; i < aux1; i++){
            aux2 = disInt1(eng);
            hijos[0][aux2] = poblacion[padre1][aux2];  //fijando un elemento aleatorio del primer padre
            hijos[1][aux2] = poblacion[padre2][aux2]; //fijando los mismos elementos con el otro padre en el segundo hijo
        }

        for (int i = 0; i < dim; i++){
            if (buscarLista(hijos[0],poblacion[padre2][i],dim) == 0){
                for (int j = 0; j < dim; j++){
                    //empieza a llenar los espacios faltantes con los elementos del padre 2
                    if (hijos[0][j] == 0){
                        hijos[0][j] = poblacion[padre2][i];
                        break;
                    }
                }
            }
            if (buscarLista(hijos[1],poblacion[padre1][i],dim) == 0){
                for (int j = 0; j < dim; j++){
                    //empieza a llenar los espacios faltantes con los elementos del padre 1
                    if (hijos[1][j] == 0){
                        hijos[1][j] = poblacion[padre1][i];
                        break;
                    }
                }
            }
        }

        //los hijos reemplazan a los padres si son mejores
        if (funCosto(dim,poblacion[padre1],dist,flujo) < funCosto(dim,poblacion[padre2],dist,flujo)){
            maxPadre = padre2; minPadre = padre1;
        }
        else {maxPadre = padre1; minPadre = padre2;}

        if (funCosto(dim,hijos[0],dist,flujo) < funCosto(dim,hijos[1],dist,flujo)){
            maxHijo = 1; minHijo = 0;
        }
        else {maxHijo = 0; minHijo = 1;}

        costoMaxPadre = funCosto(dim,poblacion[maxPadre],dist,flujo);
        costoMinPadre = funCosto(dim,poblacion[minPadre],dist,flujo);
        costoMaxHijo = funCosto(dim,hijos[maxHijo],dist,flujo);
        costoMinHijo = funCosto(dim,hijos[minHijo],dist,flujo);

        if (costoMinHijo < costoMaxPadre){
            poblacion[maxPadre] = hijos[minHijo];
            if (costoMaxHijo < costoMinPadre){poblacion[minPadre] = hijos[maxHijo];}
        }

        //mutaciones
        if (disReal(eng) < 0.10){
            aux1 = disInt2(eng);
            for (int i = 0; i < aux1; i++){
                aux2 = disInt2(eng);
                primero = disInt1(eng);
                do{segundo = disInt1(eng);}while(primero == segundo);
                aux3 = poblacion[aux2][primero];
                poblacion[aux2][primero] = poblacion[aux2][segundo];
                poblacion[aux2][segundo] = aux3;
            }
        }

        //reordenamos la población
        for (int l = 0; l < tamPob; l++){
            costos[l] = make_pair(l,funCosto(dim,poblacion[l],dist,flujo));
        }

        quicksort(&costos,0,tamPob-1);
        auxPob = poblacion;
        for (int i = 0; i<tamPob ; i++){
            poblacion[i] = auxPob[costos[i].first]; //Ordenados los elementos de menor a mayor costo
        }

        if (funCosto(dim,poblacion[0],dist,flujo) < mejorCosto){
            //se consiguió una mejor solución 
            mejorSol = poblacion[0];
            mejorCosto = (funCosto(dim,poblacion[0],dist,flujo));
            sinMejoria = 0;
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

        pairSol = geneticAlgorithm(50,dim,loc,suc);
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