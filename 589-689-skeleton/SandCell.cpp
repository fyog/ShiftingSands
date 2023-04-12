#include "SandCell.h"

// X and Z range of the cells
int _width = 4;
int _length = 4;
float random_height = 2.5f;
float rand_max = 2.5f;

// Collections of height and adhesion values
std::vector<float> heights;
std::vector<float> adhesions;

// Return / get functions
std::vector<float> getAdhesionVector() {
	return adhesions;
}

std::vector<float> getHeightsVector() {
	return heights;
}

int& getWidth() {
	return _width;
}

int& getLength() {
	return _length;
}

std::vector<float>& getHeights() {
	return heights;
}

float getHeight(std::vector<float> &heights, int x, int y) {

	// check the values of x and y to prevent out of bounds error (toric domain)
	if (x < 0) {
		x %= (_length-1);
	}
	if (y < 0) {
		y %= (_width - 1);
	}
	if (x > _length-1) {
		x %= (_length - 1);
	}
	if (y > _width-1) {
		y %= (_width - 1);
	}

	//return 10.f;
	return heights.at((_length-1) * y + x);
}

void setHeight(std::vector<float>& heights, int x, int y, float height) {

	// check the values of x and y to prevent out of bounds error (toric domain)
	if (x < 0) {
		x %= (_length - 1);
	}
	if (y < 0) {
		y %= (_width - 1);
	}
	if (x > _length-1) {
		x %= (_length - 1);
	}
	if (y > _width-1) {
		y %= (_width - 1);
	}

	heights.at((_length-1) * y + x) = height;
}

// Returns if the control points
// of the cell were changed
bool getCellChange() {
	return surfaceChange;
}

bool getShowCells() {
	return showCells;
}

int getRenderMode() {
	return renderMode;
}

float& getRandomHeight() {
	return random_height;
}

void updateCell(CPU_Geometry& cpu_geom, float height, int x, int y) {
	heights.at((_width * y) + x)= height;
}

float& get_rand_max() {
	return rand_max;
}

// Random number generator to test the structure 
float randNumber(float _min, float _max) {

	// Set up the random number generator
	std::random_device rd;
	std::mt19937 gen(rd());

	// Define the range for the random number
	std::uniform_int_distribution<> dis(_min, _max);

	// Generate a random number and print it
	float random_number = dis(gen);

	return random_number;
}

// zeroes all the heights
void clearCells(CPU_Geometry& cpu_geom) {
	heights.clear();
	for (int i = 0; i < _length; i++) {
		for (int j = 0; j < _width; j++) {

			// update heights list
			heights.push_back(0.f);
		}
	}
}

// zero the board heights and update the surface
void initializeSurface(CPU_Geometry& cpu_geom, std::vector<float> &heights) {

	// clear the list of vertices
	cpu_geom.verts.clear();
	heights.clear();


	// repopulate the list of vertices setting all heights to zero
	for (int i = 0; i < _length; i++) {
		for (int j = 0; j < _width; j++) {



			// update heights list
			heights.push_back(0.f);

			// update surface
			cpu_geom.verts.push_back(glm::vec3(i, 0.f, j));


		}
	}
}

// randomize surface heights
void randomizeHeights(CPU_Geometry& cpuGeom, std::vector<float> &heights, float max_random_height) {

	cpuGeom.verts.clear();
	heights.clear();
	float rand_height;
	for (int i = 0; i < _length; i++) {
		for (int j = 0; j < _width; j++) {
			rand_height = randNumber(0, max_random_height);
			cpuGeom.verts.push_back(glm::vec3(i, rand_height, j));
			heights.push_back(rand_height);
		}
	}
}

void redrawSurface(CPU_Geometry& cpu_geom) {

	cpu_geom.verts.clear();

	// Test value for adhesion, will need to be removed
	int _adhesion = 10.f;

	// The idea is that each data type we want to track for each cell is pushed to a
	// vector array. These vector arrays are index aligned, so the data should be
	// tied to a particular point (might be better as a struct)

	for (int i = 0; i < _length; i++) {
		for (int j = 0; j < _width; j++) {
			float cell_height;
			if (_width * j + i < heights.size()) {
				cell_height = heights.at(_width * j + i);
			}
			else {
				cell_height = 0.f;
			}

			adhesions.push_back(_adhesion);
			cpu_geom.verts.push_back(glm::vec3(i, cell_height, j));
		}
	}
}

// Function to recreate cells that uses heights vector values
void createCells(CPU_Geometry& cpuGeom) {

	cpuGeom.verts.clear();

	// Test value for adhesion, will need to be removed
	int _adhesion = 10.f;

	// The idea is that each data type we want to track for each cell is pushed to a
	// vector array. These vector arrays are index aligned, so the data should be
	// tied to a particular point (might be better as a struct)

	for (int i = 0; i < _length; i++) {
		for (int j = 0; j < _width; j++) {

			auto cell_height = heights[_width * j + i];
			//adhesions.push_back(_adhesion);
			cpuGeom.verts.push_back(glm::vec3(i, cell_height, j));
		}
	}
}

// Function that creates cells can be passed values of X & Z
void createCells(int _x, int _z, CPU_Geometry& cpuGeom) {
	_width = _x;
	_length = _z;

	createCells(cpuGeom);
}

// LERP render of cell structure, uses built in values
// Helpful for visualizing data structure
// 
// Currently doing one draw call per each X and Z value
// This causes performance issues if values are above 100
// TODO:: optimize into less draw calls
void renderCells(CPU_Geometry& input_cpu) {
	CPU_Geometry output_cpu;
	GPU_Geometry output_gpu;

	int index = 0;

	// Draws all the rows first
	for (int j = 0; j < _length; j++) {
		for (int i = 0; i < _width; i++) {
			output_cpu.verts.push_back(input_cpu.verts.at(index));
			index++;
		}
		output_gpu.setVerts(output_cpu.verts);
		output_gpu.bind();
		glDrawArrays(GL_LINE_STRIP, 0, GLsizei(output_cpu.verts.size()));
		output_cpu.verts.clear();
	}

	// Then draws all the columns
	for (int i = 0; i < _width; i++) {
		index = i;
		for (int j = 0; j < _length; j++) {
			output_cpu.verts.push_back(input_cpu.verts.at(index));
			index += _width;
		}
		output_gpu.setVerts(output_cpu.verts);
		output_gpu.bind();
		glDrawArrays(GL_LINE_STRIP, 0, GLsizei(output_cpu.verts.size()));
		output_cpu.verts.clear();
	}

	output_cpu.verts.clear();

}


// Function to render cells, can be passed X & Z values instead of using ImGui
void renderCells(CPU_Geometry& input_cpu, int _x, int _z) {
	_width = _x;
	_length = _z;

	renderCells(input_cpu);

}

// Cell render with only two draw calls (2 zig zag patterns overlayed)

void preparecellsforrender(CPU_Geometry* input_cpu, CPU_Geometry* output_cpu)
{
	output_cpu->verts.clear();
	zigzagdraw(input_cpu, output_cpu, _width, _length);
	zagzigdraw(input_cpu, output_cpu, _width, _length);
}

// Cell render with only two draw calls

// I thought this would increase frame rate - it does not
void renderCells2Calls(CPU_Geometry& input_cpu) {
	CPU_Geometry output_cpu;
	GPU_Geometry output_gpu;

	int index = 0;

	for (int j = 0; j < _length; j++) {

		if (j % 2 == 0) {
			for (int i = 0; i < _width; i++) {
				output_cpu.verts.push_back(input_cpu.verts.at(index));
				index++;
			}
			index--;
		}

		else {
			index = index + _width;
			for (int i = 0; i < _width; i++) {
				output_cpu.verts.push_back(input_cpu.verts.at(index));
				index--;
			}
			index++;
			index = index + _width;
		}
	}
	output_gpu.setVerts(output_cpu.verts);
	output_gpu.bind();
	glDrawArrays(GL_LINE_STRIP, 0, GLsizei(output_cpu.verts.size()));
	output_cpu.verts.clear();

	index = 0;

	for (int i = 0; i < _width; i++) {

		if (i % 2 == 0) {
			for (int j = 0; j < _length; j++) {
				output_cpu.verts.push_back(input_cpu.verts.at(index));
				index += _width;
			}
			index -= _width;
			index++;
		}
		else {
			for (int j = 0; j < _length; j++) {
				output_cpu.verts.push_back(input_cpu.verts.at(index));
				index -= _width;
			}
			index += _width;
			index++;
		}

	}

	output_gpu.setVerts(output_cpu.verts);
	output_gpu.bind();
	glDrawArrays(GL_LINE_STRIP, 0, GLsizei(output_cpu.verts.size()));
	output_cpu.verts.clear();

}

void cubesRender(CPU_Geometry& inputCPU, CPU_Geometry* outputCPU) {
	CPU_Geometry tempCPU;
	//CPU_Geometry outputCPU;
	GPU_Geometry outputGPU;
	outputCPU->verts.clear();

	for (int i = 0; i < inputCPU.verts.size(); i++) {
		// Makes the eight vertices of a cube
		// P0
		tempCPU.verts.push_back(glm::vec3(inputCPU.verts.at(i).x - 0.5f,
			0.0f,
			inputCPU.verts.at(i).z - 0.5f));

		// P1
		tempCPU.verts.push_back(glm::vec3(inputCPU.verts.at(i).x + 0.5f,
			0.0f,
			inputCPU.verts.at(i).z - 0.5f));

		// P2
		tempCPU.verts.push_back(glm::vec3(inputCPU.verts.at(i).x - 0.5f,
			0.0f,
			inputCPU.verts.at(i).z + 0.5f));

		// P3
		tempCPU.verts.push_back(glm::vec3(inputCPU.verts.at(i).x + 0.5f,
			0.0f,
			inputCPU.verts.at(i).z + 0.5f));

		// P4
		tempCPU.verts.push_back(glm::vec3(inputCPU.verts.at(i).x - 0.5f,
			inputCPU.verts.at(i).y,
			inputCPU.verts.at(i).z - 0.5f));

		// P5
		tempCPU.verts.push_back(glm::vec3(inputCPU.verts.at(i).x + 0.5f,
			inputCPU.verts.at(i).y,
			inputCPU.verts.at(i).z - 0.5f));

		// P6
		tempCPU.verts.push_back(glm::vec3(inputCPU.verts.at(i).x - 0.5f,
			inputCPU.verts.at(i).y,
			inputCPU.verts.at(i).z + 0.5f));

		// P7
		tempCPU.verts.push_back(glm::vec3(inputCPU.verts.at(i).x + 0.5f,
			inputCPU.verts.at(i).y,
			inputCPU.verts.at(i).z + 0.5f));

		// Traversal order for line strip in indices
		std::vector<int> traversalOrder = { 0,2,6,4,0,1,5,7,6,4,5,7,3,2,0,1,3 };

		/*
			   5 ------ 7
			  /|       / |
			 / |      /  |
			4 ------ 6   |
			|  1 ----|-- 3
			| /      |  /
			|/       | /
			0 ------ 2
		*/

		// Push back the cube verts in a particular order to draw lines on every edge
		for (int i = 0; i < traversalOrder.size(); i++) {
			outputCPU->verts.push_back(tempCPU.verts.at(traversalOrder[i]));
		}

		//outputGPU.setVerts(outputCPU.verts);
		//outputGPU.bind();
		//glDrawArrays(GL_LINE_STRIP, 0, GLsizei(outputCPU->verts.size()));
		//outputCPU->verts.clear();
		tempCPU.verts.clear();

	}
}

void pillarSetup(CPU_Geometry& inputCPU, float _height, int x, int y) {

	//int halfX = _width / 2;
	//int halfZ = _length / 2;
	//int halfway = inputCPU.verts.size() / 2;

	// Changes the middle point's height value to create a pillar
	inputCPU.verts[(_width * y) + x].y = _height;
	//inputCPU.verts[_width * y + x] = (
	//	glm::vec3(inputCPU.verts[_width * y + x].x,
	//		_height,
	//		inputCPU.verts[_width * y + x].z)
	//	);
}
