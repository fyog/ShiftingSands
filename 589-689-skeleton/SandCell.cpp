#include "SandCell.h"

// X and Z range of the cells
int _width = 4;
int _length = 4;
float random_height = 2.5f;

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

std::vector<float> getHeights() {
	return heights;
}

int getWidth() {
	return _width;
}

int getLength() {
	return _length;
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

float* getRandomHeight() {
	return &random_height;
}

void updateCell(CPU_Geometry& cpu_geom, float height, int width, int length, int x, int y) {
	cpu_geom.verts[width * y + x].y = height;
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

// Function to create cells that uses build in values
void createCells(CPU_Geometry& cpuGeom) {
	cpuGeom.verts.clear();

	// Test value for adhesion, will need to be removed
	int _adhesion = 10.f;

	// The idea is that each data type we want to track for each cell is pushed to a
	// vector array. These vector arrays are index aligned, so the data should be
	// tied to a particular point (might be better as a struct)

	for (int j = 0; j < _length; j++) {
		for (int i = 0; i < _width; i++) {

			if (randomHeights) {

				// Random heights test
				heights.push_back(randNumber(0.f, random_height));
			}
			else {
				heights.push_back(0.f);
			}

			adhesions.push_back(_adhesion);
			// Pushes row by row
			cpuGeom.verts.push_back(glm::vec3(i, heights.back(), j));
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

// randomize surface heights
void randomizeHeights(CPU_Geometry& cpuGeom, std::vector<float> heights, float max_random_height) {

	float rand_height;
	for (int i = 0; i < _length; i++) {
		for (int j = 0; j < _width; j++) {
			rand_height = randNumber(0, max_random_height);

			heights[_width * i + j] = rand_height;
		}
	}
}

void pillarSetup(CPU_Geometry& inputCPU, float _height, int x, int y) {

	//int halfX = _width / 2;
	//int halfZ = _length / 2;
	//int halfway = inputCPU.verts.size() / 2;

	// Changes the middle point's height value to create a pillar
	inputCPU.verts[_width * y + x] = (
		glm::vec3(inputCPU.verts[_width * y + x].x,
			_height,
			inputCPU.verts[_width * y + x].z)
		);
}
