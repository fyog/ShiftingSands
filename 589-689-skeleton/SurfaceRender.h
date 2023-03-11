#pragma once
#include "Geometry.h"

void zigzagdraw(CPU_Geometry obj, CPU_Geometry* newobj, int width, int length);
void zagzigdraw(CPU_Geometry obj, CPU_Geometry* newobj, int width, int length);
glm::vec3 point2dindex(CPU_Geometry obj, int rowdex, int coldex, int width);
void rendertest(CPU_Geometry lineobj, GPU_Geometry* output_gpu);
void placesurfacevecs(CPU_Geometry control, CPU_Geometry* surface, int width, int length, int k);
