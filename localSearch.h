#ifndef LOCALSEARCH_H
#define LOCALSEARCH_H

int funCosto(int dim,vector<int> sol,vector<int> dist, vector<int> flujo);
pair <int,vector<int>> localSearch(int dim, vector<int> dist, vector<int> flujo, int tipo, int porcen = 100);

#endif