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
extern float _adhesion;

extern bool randomHeights;
extern float pillarHeight;

extern bool cellChange;
extern bool showCells;
extern int renderMode;

extern std::vector<float> heights;
extern std::vector<float> adhesions;

std::vector<float> getAdhesionVector();
std::vector<float> getHeightsVector();
int getWidth();
int getLength();
bool getCellChange();
bool getShowCells();
int getRenderMode();
int getOrderK();

void sandCellImGui(CPU_Geometry& cpuGeom);

float randNumber(float _min, float _max);
void createCells(CPU_Geometry& cpuGeom);
void createCells(int _width, int _height, CPU_Geometry &cpuGeom);
void renderCells(CPU_Geometry &input_cpu);
void renderCells(CPU_Geometry &input_cpu, int _width, int _height);
void renderCells2Calls(CPU_Geometry& input_cpu);
void cubesRender(CPU_Geometry& inputCPU, CPU_Geometry* outputCPU);
void pillarSetup(CPU_Geometry& inputCPU, float _height);
void preparecellsforrender(CPU_Geometry input_cpu, CPU_Geometry* output_cpu);

#endif
