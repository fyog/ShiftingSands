#include "SandCell.h"

int _width = 4;
int _length = 4;

bool randomHeights = false;

std::vector<float> heights;
std::vector<float> adhesions;

std::vector<float> getAdhesionVector() {
	return adhesions;
}

std::vector<float> getHeightsVector() {
	return heights;
}

void sandCellImGui(CPU_Geometry &cpuGeom) {
	ImGui::Begin("Sand Cell Tuning");

	bool change = false;

	change |= ImGui::InputInt("Length (X): ", &_width);
	change |= ImGui::InputInt("Width  (Z): ", &_length);
	change |= ImGui::Checkbox("Random Heights", &randomHeights);


	if (change) {
		createCells(cpuGeom);
	}
	

	ImGui::End();
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
				heights.push_back(randNumber(-2.5f, 2.5f));
			}
			else {
				heights.push_back(0.f);
			}
			
			adhesions.push_back(_adhesion);
			cpuGeom.verts.push_back(glm::vec3(j, heights.back(), i));
			// TODO: push back vertices, and change the y value based on the height value passed
		}
	}
}

// Function that creates cells can be passed values of X & Z
void createCells(int _x, int _z, CPU_Geometry &cpuGeom) {
	_width = _x;
	_length = _z;

	createCells(cpuGeom);
}

// LERP render of cell structure, uses built in values
void renderCells(CPU_Geometry& input_cpu, CPU_Geometry& output_cpu, GPU_Geometry& output_gpu) {
	int index = 0;
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

void renderCells(CPU_Geometry &input_cpu, CPU_Geometry &output_cpu, GPU_Geometry &output_gpu, int _x, int _z) {
	_width = _x;
	_length = _z;

	renderCells(input_cpu, output_cpu, output_gpu);

}

