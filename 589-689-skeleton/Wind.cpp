#include "Wind.h"
#include "Avalanche.h"
#include "SandCell.h"
#include <random>



float slab_size = .5f;

float wind_threshold_height = 1.5f;

float beta = .15f; // beta is the coefficient that is used to determine whether a sand slab sticks or bounces when deposited

int number_of_iterations_2 = 1;

int random_number = 50;

// TO ADD:
//
// - movement of variable slab sizes, the size of which should be some function of wind strength and height
//

// Random number generator to test the structure 
//float randNumber(float _min, float _max) {
//	// Set up the random number generator
//	std::random_device rd;
//	std::mt19937 gen(rd());
//
//	// Define the range for the random number
//	std::uniform_int_distribution<> dis(_min, _max);
//
//	// Generate a random number and print it
//	float random_number = dis(gen);
//
//	return random_number;
//}

// generate wind fields - three different types: linear, radial, converging
std::vector<glm::vec3> generate_wind_field(std::string type_of_field) {
	std::vector<glm::vec3> tmp;

	// parse the desired type of wind field and then generate it
	if (type_of_field == "linear" || type_of_field == "Linear" || type_of_field == "LINEAR") {
		for (int i = 0; i < getWidth() * getLength(); i++) {
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
 glm::vec2 lift(CPU_Geometry surface, std::vector<float> heights, std::vector<glm::vec3> wind_field, int x, int y) {
	auto wind_strength = wind_field[getWidth() * (y - 1) + x];
	float height = getHeight(heights, x, y);
	setHeight(heights, x, y, height - slab_size);

	// location of deposited slab is being calculated in very rudimentary/stupid way (discuss with group to find a better method)
	auto mag = wind_strength.length() + height;

	// normalize wind direction and multiply by calculated magnitude
	return 	mag * (wind_strength / (float) wind_strength.length()); 
}

// attempts to deposit a sand slab at the given x, y coordinates. returns true if the depositing was successful, otherwise returns false
bool deposit_sand(CPU_Geometry surface, std::vector<float> heights, int x, int y) {

	// check if a random number between 0 - 1 is greater than beta
	if ((random_number / 100) > beta) {

		// deposit sand at the given x, y coordinates
		float height = getHeight(heights, x, y);
		setHeight(heights, x, y, height + slab_size);
		return true;
	}
	
	return false;
}

// collect the given cell's immediate neighbour's x, y, z coordinates 
// returns a vec3 where first entry x, second entry is y/height, third entry is z
std::vector<glm::vec3> get_neighbours_heights(CPU_Geometry surface, std::vector<float> heights, int x, int y) {
	std::vector<glm::vec3> tmp;
	tmp.push_back(glm::vec3(x + 1, getHeight(heights,  x + 1, y) , y));
	tmp.push_back(glm::vec3(x - 1, getHeight(heights,x - 1, y), y));
	tmp.push_back(glm::vec3(x, getHeight(heights,  x, y + 1), y + 1));
	tmp.push_back(glm::vec3(x, getHeight(heights,  x, y - 1), y - 1));
	return tmp;
}


// applies reptation behavior to the given surface
void reptation(CPU_Geometry surface, std::vector<float> heights, int x, int y) {

	int n = 2; // this value was suggested by the paper

	// find neighbour's heights and store them in a vector (along with the cell's x, y coordinates)
	auto neighbours = get_neighbours_heights(surface, heights, x, y);

	// sort neighbour's heights in descending order
	//std::sort(neighbours.begin() -> x, neighbours.end() -> x, std::greater<float>());

	// pop off n entries from the back of the vector (n = 2)
	auto entry_1 = neighbours.back();
	neighbours.pop_back();
	auto entry_2 = neighbours.back();

	// adjust the heights of the vectors experiencing reptation 
	setHeight(heights, entry_1.x, entry_1.z, entry_1.y + slab_size / n);
	setHeight(heights, entry_2.x, entry_2.z, entry_2.y + slab_size / n);
}

// applies wind behavior to the given surface (CPU_Geometry object) based on the given wind_field
void apply_wind(CPU_Geometry &surface, std::vector<float> heights, std::vector<glm::vec3> wind_field, float number_of_iterations_2) {
	for (int i = 0; i < number_of_iterations_2; i++) {
		for (int x = 0; x < getWidth(); x++) {
			for (int y = 0; y < getLength(); y++) {

				// if the current cell's height is above the wind threshold height
				if (getHeight(heights, x, y) > wind_threshold_height) {

					// lift a slab of sand and return the x, y coordinates of where it should be deposited
					glm::vec2 slab_deposit_distance = lift(surface, heights, wind_field, x, y);

					// attempt to deposit sand at the given x, y coordinates
					if (!deposit_sand(surface, heights, slab_deposit_distance.x, slab_deposit_distance.y)) {

						// disperse sand to n nearest neighbours if the sand was not able to be deposited
						reptation(surface, heights, x, y);
					}
				}
			}
		}
	}
}
