#include "SandCell.h"

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

void createCells(int _width, int _height, CPU_Geometry &cpuGeom) {
	// Test value for adhesion
	int _adhesion = 10.f;

	std::vector<float> heights;
	std::vector<float> adhesions;

	// The idea is that each data type we want to track for each cell is pushed to a
	// vector array. These vector arrays are index aligned, so the data should be
	// tied to a particular point (might be better as a struct)
	for (int j = 0; j < _height; j++) {
		for (int i = 0; i < _width; i++) {
			//heights.push_back(randNumber(-2.5f,2.5f));
			heights.push_back(0.f);
			adhesions.push_back(_adhesion);
			cpuGeom.verts.push_back(glm::vec3(j, heights.back(), i));
			// TODO: push back vertices, and change the y value based on the height value passed
		}
	}
}

void renderCells(CPU_Geometry &input_cpu, CPU_Geometry &output_cpu, GPU_Geometry &output_gpu, int _width, int _height) {
	int index = 0;
	for (int j = 0; j < _height; j++) {
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
		for (int j = 0; j < _height; j++) {
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

