#include "Geometry.h"
#include <vector>
#include <string>



extern std::vector<glm::vec3> wind_field;

extern float slab_size;

extern float wind_threshold_height;

extern float beta;

extern int number_of_iterations_2;

glm::vec2 lift(CPU_Geometry& surface, std::vector<glm::vec3> wind_field, int x, int y);

bool deposit_sand(CPU_Geometry& surface, int x, int y);

void apply_wind(CPU_Geometry& surface, std::vector<glm::vec3> wind_field,  float number_of_iterations);


std::vector<glm::vec3> generate_wind_field(CPU_Geometry& surface, int field_type);

