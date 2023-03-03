#include <vector>
#include "Geometry.h"


// Size of the simulation, it will be a square of
// sim_size x sim_size
int _area_length_x;
int _area_length_z;

float _height;
float _adhesion;

std::vector<float> heights;
std::vector<float> adhesions;
std::vector<CPU_Geometry> locations;

void test();
