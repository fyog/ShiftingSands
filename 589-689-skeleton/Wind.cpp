#include "Wind.h"
#include "Avalanche.h"


extern std::vector<glm::vec3> wind_field;

float slab_size = .5f;

float threshold_height = 1.5f;

float beta = .05f; // beta is the coefficient that is used to determine whether a sand slab sticks or bounces off of a given cell

int number_of_iterations_2 = 1;

int random_number = 50; // placeholder - I was to lazy to implement RNG tonight

// TO ADD:
//
// - reptation
// - movement of variable slab sizes, the size of which should be some function of wind strength and height

//float getHeight(std::vector<float> heights, int width, int length, int x, int y) {
//	return heights[width * (y - 1) + x];
//}
//
//void setHeight(std::vector<float> heights, int width, int length, int x, int y, float height) {
//	heights[width * (y - 1) + x] = height;
//}

std::vector<glm::vec3> generate_wind_field(std::string type_of_field, int width, int length) {
	std::vector<glm::vec3> tmp;
	if (type_of_field == "linear" || type_of_field == "Linear" || type_of_field == "LINEAR") {
		for (int i = 0; i < width * length; i++) {
			tmp.push_back(glm::vec3{ 5.0f, 0.f, 0.f });
		}
	}
	else {
		// add other types of wind fields later (eg. circular field, sine wave field, field converging from exact opposite directions)
	}
	return tmp;
}

// lifts a small amount of sand from the given cell and returns the x,y coords it should travel to based on the given height and wind strength/direction at that cell, eventually when we want to
// transport a variable amount of sand we can return a vec3 instead where the first entry is the slab size
 glm::vec2 lift(CPU_Geometry * surface, std::vector<float> heights, std::vector<glm::vec3> wind_field, int width, int length, int x, int y) {
	auto wind_strength = wind_field[width * (y - 1) + x];
	float height = getHeight(heights, width, length, x, y);
	setHeight(heights, width, length, x, y, height - slab_size);

	// x, y of deposited slab is being calculated in very rudimentary way (discuss with group to find a better method)
	auto mag = wind_strength.length() + height;

	return 	mag * (wind_strength / (float) wind_strength.length()); // normalize wind direction and multiply by calculated magnitude
}

// deposits sand slab at the given x, y coordinates and returns true if the random number check is above beta, otherwise returns false
bool deposit_sand(CPU_Geometry* surface, std::vector<float> heights, int width, int length, int x, int y) {
	if (random_number % 100 > beta * 100) {
		float height = getHeight(heights, width, length, x, y);
		setHeight(heights, width, length, x, y, height + slab_size);
		return true;
	}
	
	return false;
}

// applies wind behavior to the given surface (CPU_Geometry object) based on the given wind_field and current heights of each cell (missing reptation)
void apply_wind(CPU_Geometry* surface, std::vector<float> heights, std::vector<glm::vec3> wind_field, int width, int length, float number_of_iterations_2) {
	for (int i = 0; i < number_of_iterations_2; i++) {
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < length; y++) {
				if (getHeight(heights, width, length, x, y) > threshold_height) {
					glm::vec2 slab_deposit_distance = lift(surface, heights, wind_field, width, length, x, y);
					if (!deposit_sand(surface, heights, width, length, slab_deposit_distance.x, slab_deposit_distance.y)) {

						// implement reptation behavior here
					}
				}
			}
		}
	}
}
