#include "Wind.h"
#include "Avalanche.h"
#include "SandCell.h"
#include <random>



float slab_size = .5f;

float wind_threshold_height = .5f;

float wind_mag = 2.f;

float beta = .15f; // beta is the coefficient that is used to determine whether a sand slab sticks or bounces when deposited

int number_of_iterations_2 = 1;

int random_number = 50;

std::vector<glm::vec3> wind_field;

std::vector<glm::vec3>& getWindField() {
	return wind_field;
}

glm::vec3 getWind(int x, int y) {
	return wind_field[getWidth() * y + x];
}

glm::vec3 getDirection(int x, int y) {
	return glm::normalize(glm::vec3(x, 0.f, y));
}

void setWind(int x, int y, glm::vec3 wind_vector) {
	wind_field[getWidth() * y + x] = wind_vector;
}

// generate wind fields - three different types: linear, radial, converging
void generate_wind_field(CPU_Geometry& surface, int field_type, float wind_mag) {
	std::vector<glm::vec3> tmp;

	// parse the desired type of wind field and then generate it
	// Linear
	if (field_type == 0) {
		for (int i = 0; i < surface.verts.size(); i++) {
			tmp.push_back(glm::vec3{ wind_mag, 0.f, 0.f });
		}
	}
	// Radial
	else if (field_type == 1) {
		for (int i = 0; i < _length; i++) {
			for (int j = 0; j < _width; j++) {
				tmp.push_back(getDirection(i, j));
			}
		}
	}
	// Convergent
	else if (field_type == 2) {
		for (int i = 0; i < surface.verts.size(); i++) {
			if (surface.verts[i].y < _length / 2) {
				tmp.push_back(glm::vec3(wind_mag, 0.f, 0.f));
			}
			else {
				tmp.push_back(glm::vec3(-wind_mag, 0.f, 0.f));
			}
		}
	}

	wind_field = tmp;
}

// lifts a small amount of sand from the given cell and returns the x,y coords it should travel to based on the given height and wind strength/direction at that cell, eventually when we want to
// transport a variable amount of sand we can return a vec3 instead where the first entry is the slab size
glm::vec3 lift(CPU_Geometry& surface, std::vector<glm::vec3> wind_field, int x, int y) {
	auto wind_strength = wind_field[getWidth() * (y)+x];
	float height = getHeight(surface, x, y);

	setHeight(surface, x, y, height - slab_size);

	// location of deposited slab is being calculated in very rudimentary/stupid way (discuss with group to find a better method)
	auto mag = glm::length(wind_strength) + height;

	// normalize wind direction and multiply by calculated magnitude
	return 	mag * (wind_strength / glm::length(wind_strength));
}

// attempts to deposit a sand slab at the given x, y coordinates. returns true if the depositing was successful, otherwise returns false
bool deposit_sand(CPU_Geometry& surface, int x, int y) {

	// check if a random number between 0 - 1 is greater than beta
	if ((randNumber(0, 100) / 100.f) > beta) {

		// deposit sand at the given x, y coordinates
		float height = getHeight(surface, x, y);
		setHeight(surface, x, y, height + slab_size);
		return true;
	}

	return false;
}

// collect the given cell's immediate neighbour's x, y, z coordinates 
// returns a vec3 where first entry x, second entry is y/height, third entry is z
std::vector<glm::vec3> get_neighbours_heights(CPU_Geometry surface, int x, int y) {
	std::vector<glm::vec3> tmp;
	tmp.push_back(glm::vec3(x + 1, getHeight(surface, x + 1, y), y));
	tmp.push_back(glm::vec3(x - 1, getHeight(surface, x - 1, y), y));
	tmp.push_back(glm::vec3(x, getHeight(surface, x, y + 1), y + 1));
	tmp.push_back(glm::vec3(x, getHeight(surface, x, y - 1), y - 1));
	return tmp;
}


// applies reptation behavior to the given surface
void reptation(CPU_Geometry& surface, int x, int y) {

	int n = 2; // this value was suggested by the paper

	// find neighbour's heights and store them in a vector (along with the cell's x, y coordinates)
	auto neighbours = get_neighbours_heights(surface, x, y);

	// sort neighbour's heights in descending order
	//std::sort(neighbours.begin() -> x, neighbours.end() -> x, std::greater<float>());

	// pop off n entries from the back of the vector (n = 2)
	auto entry_1 = neighbours.back();
	neighbours.pop_back();
	auto entry_2 = neighbours.back();

	// adjust the heights of the vectors experiencing reptation 
	setHeight(surface, entry_1.x, entry_1.z, entry_1.y + slab_size / n);
	setHeight(surface, entry_2.x, entry_2.z, entry_2.y + slab_size / n);
}

// applies wind behavior to the given surface (CPU_Geometry object) based on the given wind_field
void apply_wind(CPU_Geometry& surface, std::vector<glm::vec3> wind_field, float number_of_iterations_2) {
	for (int i = 0; i < number_of_iterations_2; i++) {
		for (int x = 0; x < getWidth(); x++) {
			for (int y = 0; y < getLength(); y++) {

				// if the current cell's height is above the wind threshold height
				if (getHeight(surface, x, y) > wind_threshold_height) {

					// lift a slab of sand and return the x, y coordinates of where it should be deposited
					glm::vec3 slab_deposit_distance = lift(surface, wind_field, x, y);

					// attempt to deposit sand at the given x, y coordinates
					if (!deposit_sand(surface, slab_deposit_distance.x + x, slab_deposit_distance.z + y)) {

						// disperse sand to n nearest neighbours if the sand was not able to be deposited
						reptation(surface, x, y);
					}
				}
			}
		}
	}
}
