local: localSearch.cpp
	g++ -std=c++11 localSearch.cpp -o local
simulated: simAnnealing.cpp
	g++ -std=c++11 simAnnealing.cpp -o sim
tabu: tabuSearch.cpp
	g++ -std=c++11 tabuSearch.cpp -o tabu
ils: ils.cpp
	g++ -std=c++11 ils.cpp -o ils
genetic: geneticAlgorithm.cpp
	g++ -std=c++11 geneticAlgorithm.cpp -o gen