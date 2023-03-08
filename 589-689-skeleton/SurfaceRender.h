#pragma once
#include "Geometry.h"

void zigzagdraw(CPU_Geometry obj, CPU_Geometry* newobj, int width, int length);
glm::vec3 point2dindex(CPU_Geometry obj, int rowdex, int coldex, int width);
void rendertest(CPU_Geometry lineobj);
void placesurfacevecs(CPU_Geometry control, CPU_Geometry* surface, int width, int length);
