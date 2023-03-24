#include "Surface.h"
#include "SandCell.h"
#include "SurfaceRender.h"

// Toggle if you want to generate random heights
bool randomHeights = false;
float pillarHeight = 0.f;

// constructor

Surface::Surface(int width, int length, CPU_Geometry surface) {
	Surface::width = width;
	Surface::length = length;
}

// getters and setters
int Surface::getWidth() {
	return Surface::width;
}

int Surface::getLength() {
	return Surface::length;
}

void Surface::setWidth(int width) {
	Surface::width= width;
}

void Surface::setLength(int length) {
	Surface::length = length;
}
std::vector<float> Surface::getAdhesionVector() {
	return Surface::adhesions;
}

std::vector<float> Surface::getHeightsVector() {
	return Surface::heights;
}

bool Surface::show() {
	return Surface::showCells;
}

int Surface::getMode() {
	return Surface::renderMode;
}

void Surface::setMode(int renderMode) {
	Surface::renderMode = renderMode;
}

bool Surface::getRandomizedHeights() {
	return Surface::randomHeights;
}

float Surface::getPillarHeight() {
	return Surface::pillarHeight;
}
// LERP render of cell structure, uses built in values
// Helpful for visualizing data structure
// 
// Currently doing one draw call per each X and Z value
// This causes performance issues if values are above 100
// TODO:: optimize into less draw calls

void Surface::renderCells(CPU_Geometry& input_cpu) {
	CPU_Geometry output_cpu;
	GPU_Geometry output_gpu;

	int index = 0;

	// Draws all the rows first
	for (int j = 0; j < length; j++) {
		for (int i = 0; i < width; i++) {
			output_cpu.verts.push_back(input_cpu.verts.at(index));
			index++;
		}
		output_gpu.setVerts(output_cpu.verts);
		output_gpu.bind();
		glDrawArrays(GL_LINE_STRIP, 0, GLsizei(output_cpu.verts.size()));
		output_cpu.verts.clear();
	}

	// Then draws all the columns
	for (int i = 0; i < width; i++) {
		index = i;
		for (int j = 0; j < length; j++) {
			output_cpu.verts.push_back(input_cpu.verts.at(index));
			index += width;
		}
		output_gpu.setVerts(output_cpu.verts);
		output_gpu.bind();
		glDrawArrays(GL_LINE_STRIP, 0, GLsizei(output_cpu.verts.size()));
		output_cpu.verts.clear();
	}

	output_cpu.verts.clear();

}

// Function to create cells that uses build in values
void Surface::createCells(CPU_Geometry& cpuGeom) {
	cpuGeom.verts.clear();
	// Test value for adhesion, will need to be removed
	int _adhesion = 10.f;

	// The idea is that each data type we want to track for each cell is pushed to a
	// vector array. These vector arrays are index aligned, so the data should be
	// tied to a particular point (might be better as a struct)

	for (int j = 0; j < length; j++) {
		for (int i = 0; i < width; i++) {

			if (randomHeights) {
				// Random heights test
				heights.push_back(randNumber(0.f, 2.5f));
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
void Surface::createCells(int width, int length, CPU_Geometry& cpuGeom) {
	Surface::width = width;
	Surface::length = length;

	createCells(cpuGeom);
}

// Returns if the control points
// of the cell were changed
bool Surface::change() {
	return Surface::cellChange;
}

// Function to render cells, can be passed X & Z values instead of using ImGui
void Surface::renderCells(CPU_Geometry& input_cpu, int _x, int _z) {
	width = _x;
	length = _z;
	renderCells(input_cpu);
}

// Cell render with only two draw calls (2 zig zag patterns overlayed)
void Surface::prepareCellsForRender(CPU_Geometry input_cpu, CPU_Geometry * output_cpu) {
	output_cpu->verts.clear();
	zigzagdraw(input_cpu, output_cpu, Surface::getWidth(), Surface::getLength());
	zagzigdraw(input_cpu, output_cpu, Surface::getWidth(), Surface::getLength());
}

// Cell render with only two draw calls
// I thought this would increase frame rate - it does not
void Surface::renderCells2Calls(CPU_Geometry& input_cpu) {
	CPU_Geometry output_cpu;
	GPU_Geometry output_gpu;

	int index = 0;

	for (int j = 0; j < Surface::getLength(); j++) {

		if (j % 2 == 0) {
			for (int i = 0; i < Surface::getWidth(); i++) {
				output_cpu.verts.push_back(input_cpu.verts.at(index));
				index++;
			}
			index--;
		}

		else {
			index = index + Surface::getWidth();
			for (int i = 0; i < Surface::getWidth(); i++) {
				output_cpu.verts.push_back(input_cpu.verts.at(index));
				index--;
			}
			index++;
			index = index + Surface::getWidth();
		}
	}
	output_gpu.setVerts(output_cpu.verts);
	output_gpu.bind();
	glDrawArrays(GL_LINE_STRIP, 0, GLsizei(output_cpu.verts.size()));
	output_cpu.verts.clear();

	index = 0;

	for (int i = 0; i < Surface::getWidth(); i++) {

		if (i % 2 == 0) {
			for (int j = 0; j < Surface::getLength(); j++) {
				output_cpu.verts.push_back(input_cpu.verts.at(index));
				index += Surface::getWidth();
			}
			index -= Surface::getWidth();
			index++;
		}
		else {
			for (int j = 0; j < Surface::getLength(); j++) {
				output_cpu.verts.push_back(input_cpu.verts.at(index));
				index -= Surface::getWidth();
			}
			index += Surface::getWidth();
			index++;
		}

	}
	output_gpu.setVerts(output_cpu.verts);
	output_gpu.bind();
	glDrawArrays(GL_LINE_STRIP, 0, GLsizei(output_cpu.verts.size()));
	output_cpu.verts.clear();

}

void Surface::cubesRender(CPU_Geometry& inputCPU, CPU_Geometry* outputCPU) {
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

void Surface::pillarSetup(CPU_Geometry& inputCPU, float _height) {
	//int halfX = Surface::getWidth() / 2;
	//int halfZ = Surface::getLength() / 2;

	// This is a quick way to find the halfway point of the vertices
	// Does not work for all sizes as it will often end up on the edge of the patch
	int halfway = inputCPU.verts.size() / 2;

	// Changes the middle point's height value to create a pillar
	inputCPU.verts.at(halfway) = (glm::vec3(inputCPU.verts.at(halfway).x,
		_height,
		inputCPU.verts.at(halfway).z));
}

// ImGui panel to control the sand cells
void Surface::sandCellImGui(CPU_Geometry & cpuGeom) {
	ImGui::Begin("Sand Cell Tuning");
	bool cellChange = Surface::change();
	//cellChange = false;
	bool show = Surface::show();

	int l = Surface::getLength();
	int w = Surface::getWidth();
	bool r_heights = Surface::getRandomizedHeights();
	float p_height = Surface::getPillarHeight();
	int mode = Surface::getMode();

	
	// Names of render modes to be displayed in slider
	const char* renderModeNames[] = { "LERP", "Cubes", "Smooth" };

	cellChange |= ImGui::InputInt("Length (X): ", &l);
	cellChange |= ImGui::InputInt("Width  (Z): ", &w);

	cellChange |= ImGui::Checkbox("Random Heights", &r_heights);
	if (!r_heights) {
		cellChange |= ImGui::InputFloat("Height of Pillar", &p_height, 0.1f, 1.f);
	}

	if (cellChange) {
		// Recreate cells
		createCells(cpuGeom);
		if (!r_heights) {
			pillarSetup(cpuGeom, p_height);
		}
	}

	ImGui::Checkbox("Render Cells", &showCells);
	if (show) {
		ImGui::Text("Number of Draw Calls:");
		ImGui::SliderInt(renderModeNames[mode], &mode, 0, 2);
	}

	ImGui::End();
}

// Random number generator
float Surface::randNumber(float min, float max) {

	// Set up the random number generator
	std::random_device rd;
	std::mt19937 gen(rd());

	// Define the range for the random number
	std::uniform_int_distribution<> dis(min, max);

	// Generate a random number and print it
	float random_number = dis(gen);

	return random_number;
}
