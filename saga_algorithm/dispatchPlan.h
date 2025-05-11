/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.h to edit this template
 */

/* 
 * File:   dispatchPlan.h
 * Author: renzo
 *
 * Created on 8 de mayo de 2025, 11:31
 */

#ifndef DISPATCHPLAN_H
#define DISPATCHPLAN_H

#include "route.h"
#include "block.h"
#include "transportUnit.h"
#include <string>
#include <vector>


class DispatchPlan {
private:
    Route* route;
    string dispatchDate;
    string dispatchTime;
    TransportUnit* transportUnit;
    vector<Block*> blocks;
public:
    DispatchPlan();
};

#endif /* DISPATCHPLAN_H */

