#ifndef ALPHAREACTIVE_H
#define ALPHAREACTIVE_H

#include <vector>

using namespace std;

class AlphaReactive {
public:
    AlphaReactive(const vector<double>& alphaSet);
    
    double pick();
    void update(int idx, double reward);
private:
    vector<double> alphas;          // Conjunto A de coeficientes
    vector<double> scores;          // Métrica acumulada para cada alpha
    int lastChosenIndex;            // Índice del alpha elegido en la iteración anterior
};

#endif /* ALPHAREACTIVE_H */

