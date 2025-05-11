/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.h to edit this template
 */

/* 
 * File:   gene.h
 * Author: renzo
 *
 * Created on 8 de mayo de 2025, 11:28
 */

#ifndef GENE_H
#define GENE_H

#include "block.h"
#include "dispatchPlan.h"

class Gene {
private:
    Block* block;
    DispatchPlan* dispatch;
public:
    Gene();
};

#endif /* GENE_H */

