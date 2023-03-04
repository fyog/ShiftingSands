#include "Geometry.h"
#include <iostream>

float calcomega(float * knots, float u, int i, int r);
int getdelta(float* knots, int n, float u, int delta);
void standardknots(float* knots, int k, int m);
glm::vec3 onesplinecalc(float omega, CPU_Geometry* rangecopy, int poindex);
glm::vec3 quickspline(CPU_Geometry * control, float * knots, float u, int k, int m);
void bspline(int k, CPU_Geometry * control, float ustep, CPU_Geometry * curve);
bool checkpointcount(int k, CPU_Geometry* control);
