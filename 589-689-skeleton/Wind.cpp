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

void setWind(int x, int y, glm::vec3 &wind_vector) {
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
		for (int i = 0; i < _length; i++) {
			for (int j = 0; j < _width; j++) {
				
				if (i < _length) {
					tmp.push_back(glm::normalize(glm::vec3(i, 0.f, j)));
				}
				else {
					tmp.push_back(glm::normalize(glm::vec3(-i, 0.f, j)));
				}
			}
		}
	}

	wind_field = tmp;
}

// lifts a small amount of sand from the given cell and returns the x,y coords it should travel to based on the given height and wind strength/direction at that cell, eventually when we want to
// transport a variable amount of sand we can return a vec3 instead where the first entry is the slab size
glm::vec3 lift(CPU_Geometry& surface, std::vector<glm::vec3> &wind_field, int x, int y) {
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
		onecellavalanche(&surface, x, y);
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

glm::vec3 toricindex(CPU_Geometry* surface, int x, int y)
{
	int xp, yp;
	glm::vec3 temp;
	float xc, yc;
	xc = 0.f;
	yc = 0.f;
	xp = x;
	yp = y;
	while (xp < 0)
	{
		xp += getWidth();
		xc -= 1.f;
	}
	while (yp < 0)
	{
		yp += getLength();
		yc -= 1.f;
	}
	while (xp >= getWidth())
	{
		xp -= getWidth();
		xc += 1.f;
	}
	while (yp >= getLength())
	{
		yp -= getLength();
		yc += 1.f;
	}
	temp = surface->verts[(getWidth() * yp) + xp];
	temp.x = temp.x + (xc * float(getWidth()));
	temp.z = temp.z + (yc * float(getLength()));
	return temp;
}

bool checkwindpath(glm::vec3 windvec, glm::vec3 distvec)
{
	//this one's a bit strange, so i should probably explain it. we use the dot product to check the angle between
	//the wind direction and some distance vector we get from subtracting a point from another. if distvec is in some
	//approximate range from the wind direction (defined here about give or take 15 degrees off from it) we return true, otherwise false
	glm::vec3 zerovec = glm::vec3(0.f, 0.f, 0.f);
	if (distvec == zerovec || windvec == zerovec ) return false;
	glm::vec3 normwind, normdist;
	normwind = glm::normalize(windvec);
	normdist = glm::normalize(distvec);
	float dotprod = glm::dot(normwind, normdist);
	if (dotprod > 0.95) return true; //the dot product gives cos(theta) between the two vectors, cos(0) = 1, and cos(15) = cos (-15) is approximately 0.966. setting the check value at 0.95 is close enough for our purposes.
	return false;
}

bool checkoneshadow(glm::vec3 obvert, glm::vec3 subvert)
{
	if (obvert.y >= subvert.y) return true;
	glm::vec3 flatob, flatsub, normdist, normflat;
	flatob = glm::vec3(obvert.x, 0.f, obvert.z);
	flatsub = glm::vec3(subvert.x, 0.f, subvert.z);
	return checkwindpath((flatob - flatsub), (obvert - subvert));
}

bool subcheckshadow(CPU_Geometry* surface, int x, int y, int xstep, int ystep, glm::vec3 oppwind)
{

	float windrad = glm::length(oppwind);
	if (windrad > float(getWidth()) || windrad > float(getLength())) return false;
	glm::vec3 mainvert, tempvert, tempdist;
	int xp, yp;
	xp = x;
	yp = y;
	float distance;
	mainvert = toricindex(surface, x, y);
	tempvert = mainvert;
	tempdist = mainvert - tempvert;
	distance = 0.f;
	bool returner = true;
	while (distance < windrad && returner == true)
	{
		while (distance < windrad && returner == true)
		{
			if (checkwindpath(oppwind, tempdist)) returner = checkoneshadow(mainvert, tempvert); //if mainvert is blocked by tempvert, then returner will be set to false and the loops will break
			xp += xstep;
			tempvert = toricindex(surface, xp, yp);
			tempdist = mainvert - tempvert;
			distance = glm::length(tempdist);
		}
		xp = x; //reset xp for inner loop
		yp += ystep;
		tempvert = toricindex(surface, xp, yp);
		tempdist = mainvert - tempvert;
		distance = glm::length(tempdist);
	}
	return returner;
}

bool checkshadow(CPU_Geometry* surface, int x, int y, std::vector<glm::vec3>* wind_field)
{
	//If a given control point isn't in a wind-shadow, this will return true, otherwise, false.
	glm::vec3 oppwind = (-1.f) * (wind_field->at((getWidth() * y) + x)); //the opposite direction of the wind at the cell
	//float mag = glm::length(oppwind);
	bool returner = true;
	if (oppwind.x >= 0.f && oppwind.z >= 0.f) returner = subcheckshadow(surface, x, y, 1, 1, oppwind);
	else if (oppwind.x >= 0.f && oppwind.z < 0.f) returner = subcheckshadow(surface, x, y, 1, (-1), oppwind);
	else if (oppwind.x < 0.f && oppwind.z >= 0.f) returner = subcheckshadow(surface, x, y, (-1), 1, oppwind);
	else if (oppwind.x < 0.f && oppwind.z < 0.f) returner = subcheckshadow(surface, x, y, (-1), (-1), oppwind);
	return true;
}

// applies wind behavior to the given surface (CPU_Geometry object) based on the given wind_field
void apply_wind(CPU_Geometry& surface, std::vector<glm::vec3> &wind_field, float number_of_iterations_2) {
	for (int i = 0; i < number_of_iterations_2; i++) {
		for (int x = 0; x < getWidth(); x++) {
			for (int y = 0; y < getLength(); y++) {


				// Checks the height of the cell, compares it to the current threshold
				// This will generate a sort of % of how likely it is to get picked up
				// if the value is above 1 it will always get picked up
				float adhesion_percent = (getHeight(surface, x, y)) / wind_threshold_height;

				// if the value is between 1 and 0 it has that % chance of getting picked up
				// Effectively this makes it so that sand above a certain height always gets picked up, while
				// sand below a certain height sometimes gets picked up
				if (adhesion_percent > (randNumber(0, 1)) && checkshadow(&surface, x, y, &wind_field)) {
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
