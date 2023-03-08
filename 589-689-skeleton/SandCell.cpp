#include "SandCell.h"

// X and Z range of the cells
int _width = 4;
int _length = 4;

// Toggle if you want to generate random heights
bool randomHeights = false;
float pillarHeight = 0.f;
// Toggle to render the cells
bool showCells = true;
int renderMode = 0;

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

bool getShowCells() {
	return showCells;
}

int getRenderMode() {
	return renderMode;
}

// ImGui panel to control the sand cells
void sandCellImGui(CPU_Geometry& cpuGeom) {
	ImGui::Begin("Sand Cell Tuning");


	// Names of render modes to be displayed in slider
	const char* renderModeNames[] = { "Multi-Call", "Two-Call", "Cubes"};

	bool change = false;

	change |= ImGui::InputInt("Length (X): ", &_width);
	change |= ImGui::InputInt("Width  (Z): ", &_length);
	change |= ImGui::Checkbox("Random Heights", &randomHeights);
	if (!randomHeights) {
		change |= ImGui::InputFloat("Height of Pillar", &pillarHeight, 0.1f, 1.f);
		pillarSetup(cpuGeom, pillarHeight);
	}

	if (change) {
		createCells(cpuGeom);
	}

	ImGui::Checkbox("Render Cells", &showCells);
	if (showCells) {
		ImGui::Text("Number of Draw Calls:");
		ImGui::SliderInt(renderModeNames[renderMode], &renderMode, 0, 2);
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
				heights.push_back(randNumber(0.f, 2.5f));
			}
			else {
				heights.push_back(0.f);
			}
			
			adhesions.push_back(_adhesion);
			cpuGeom.verts.push_back(glm::vec3(i, heights.back(), j));
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
// Helpful for visualizing data structure
// 
// Currently doing one draw call per each X and Z value
// This causes performance issues if values are above 100
// TODO:: optimize into less draw calls
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

// Cell render with only two draw calls
// I thought this would increase frame rate - it does not
void renderCells2Calls(CPU_Geometry& input_cpu, CPU_Geometry& output_cpu, GPU_Geometry& output_gpu) {
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

// Function to render cells, can be passed X & Z values instead of using ImGui
void renderCells(CPU_Geometry &input_cpu, CPU_Geometry &output_cpu, GPU_Geometry &output_gpu, int _x, int _z) {
	_width = _x;
	_length = _z;

	renderCells(input_cpu, output_cpu, output_gpu);

}

void cubesRender(CPU_Geometry &inputCPU) {
	CPU_Geometry tempCPU;
	CPU_Geometry outputCPU;
	GPU_Geometry outputGPU;

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
			outputCPU.verts.push_back(tempCPU.verts.at(traversalOrder[i]));
		}

		outputGPU.setVerts(outputCPU.verts);
		outputGPU.bind();
		glDrawArrays(GL_LINE_STRIP, 0, GLsizei(outputCPU.verts.size()));
		outputCPU.verts.clear();
		tempCPU.verts.clear();

	}
}

void pillarSetup(CPU_Geometry& inputCPU, float _height) {
	//int halfX = _width / 2;
	//int halfZ = _length / 2;

	int halfway = inputCPU.verts.size() / 2;	

	inputCPU.verts.at(halfway) = (glm::vec3(inputCPU.verts.at(halfway).x,
											_height,
											inputCPU.verts.at(halfway).z));
}
