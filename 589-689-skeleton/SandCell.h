#pragma once
#ifndef SANDCELL_H
#define SANDCELL_H

#include <vector>
#include <random>
#include "Geometry.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"


// Size of the simulation, it will be a square of
// sim_size x sim_size
extern int _width;
extern int _length;
extern float _height;
extern float _adhesion;

extern bool randomHeights;
extern bool showCells;

extern std::vector<float> heights;
extern std::vector<float> adhesions;

std::vector<float> getAdhesionVector();
std::vector<float> getHeightsVector();
bool getShowCells();

void sandCellImGui(CPU_Geometry& cpuGeom);

float randNumber(float _min, float _max);
void createCells(CPU_Geometry& cpuGeom);
void createCells(int _width, int _height, CPU_Geometry &cpuGeom);
void renderCells(CPU_Geometry& input_cpu, CPU_Geometry& output_cpu, GPU_Geometry& output_gpu);
void renderCells(CPU_Geometry &input_cpu, CPU_Geometry &output_cpu, GPU_Geometry &output_gpu, int _width, int _height);

#endif
