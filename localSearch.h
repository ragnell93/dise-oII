#ifndef LOCALSEARCH_H
#define LOCALSEARCH_H

#include <vector>
#include <utility>

int funCosto(int dim,std::vector<int> sol,std::vector<int> dist, std::vector<int> flujo);
std::pair <int,std::vector<int>> localSearch(int dim, std::vector<int> dist, std::vector<int> flujo, int tipo, int porcen = 100);

#endif