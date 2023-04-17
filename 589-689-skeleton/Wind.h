#include "Geometry.h"
#include <vector>
#include <string>



extern std::vector<glm::vec3> wind_field;

extern float slab_size;

extern float wind_mag;

extern float wind_threshold_height;

extern float beta;

extern int number_of_iterations_2;

extern std::vector<glm::vec3> wind_field;

std::vector<glm::vec3>& getWindField();

glm::vec3 getWind(int x, int y);

void setWind(int x, int y, glm::vec3 wind_vector);

glm::vec3 lift(CPU_Geometry& surface, std::vector<glm::vec3> wind_field, int x, int y);

bool deposit_sand(CPU_Geometry& surface, int x, int y);

void apply_wind(CPU_Geometry& surface, std::vector<glm::vec3> wind_field,  float number_of_iterations);

void generate_wind_field(CPU_Geometry& surface, int field_type, float wind_mag);

