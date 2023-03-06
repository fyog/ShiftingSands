#pragma once
#ifndef SANDCELL_H
#define SANDCELL_H

#include <vector>
#include <random>
#include "Geometry.h"


// Size of the simulation, it will be a square of
// sim_size x sim_size
extern float _height;
extern float _adhesion;

extern std::vector<float> heights;
extern std::vector<float> adhesions;
extern std::vector<CPU_Geometry> locations;

float randNumber(float _min, float _max);
void createCells(int _length, int _height, CPU_Geometry &cpuGeom);
void renderCells(CPU_Geometry &input_cpu, CPU_Geometry &output_cpu, GPU_Geometry &output_gpu, int _width, int _height);

#endif
