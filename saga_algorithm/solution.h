/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.h to edit this template
 */

/* 
 * File:   solution.h
 * Author: renzo
 *
 * Created on 8 de mayo de 2025, 11:40
 */

#ifndef SOLUTION_H
#define SOLUTION_H

#include <vector>
#include "dispatchPlan.h"

class Solution {
private:
    vector<DispatchPlan*> finalDispatches;
    double objectiveScore;
public:
    Solution();
};

#endif /* SOLUTION_H */

