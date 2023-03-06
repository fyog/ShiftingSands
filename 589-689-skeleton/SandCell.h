#pragma once
#ifndef SANDCELL_H
#define SANDCELL_H

#include <vector>
#include "Geometry.h"


// Size of the simulation, it will be a square of
// sim_size x sim_size
extern float _height;
extern float _adhesion;

extern std::vector<float> heights;
extern std::vector<float> adhesions;
extern std::vector<CPU_Geometry> locations;

void createCells(int _length, int _width, CPU_Geometry cpuGeom);

#endif
