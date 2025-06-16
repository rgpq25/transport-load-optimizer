#include "alphaReactive.h"
#include <cstdlib>
#include <numeric>

AlphaReactive::AlphaReactive(const std::vector<double>& alphaSet)
    : alphas(alphaSet), scores(alphaSet.size(), 1.0), lastChosenIndex(0) {}

double AlphaReactive::pick() {
    // Simple roulette-wheel selection based on scores
    double total = accumulate(scores.begin(), scores.end(), 0.0);
    double r = (rand() / (double)RAND_MAX) * total;
    for (size_t i = 0; i < scores.size(); ++i) {
        if (r <= scores[i]) {
            lastChosenIndex = i;
            return alphas[i];
        }
        r -= scores[i];
    }
    lastChosenIndex = scores.size() - 1;
    return alphas.back();
}

void AlphaReactive::update(int idx, double reward) {
    if (idx >= 0 && idx < (int)scores.size()) {
        scores[idx] += reward;
    }
}
