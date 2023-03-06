#include "SandCell.h"


void createCells(int _length, int _width, CPU_Geometry cpuGeom) {
	// Test value for adhesion
	int _adhesion = 10.f;
	int _height = 10.f;

	std::vector<float> heights;
	std::vector<float> adhesions;

	// The idea is that each data type we want to track for each cell is pushed to a
	// vector array. These vector arrays are index aligned, so the data should be
	// tied to a particular point (might be better as a struct)
	for (int j = 0; j < _length; j++) {
		for (int i = 0; i < _width; i++) {
			heights.push_back(_height);
			adhesions.push_back(_adhesion);
			cpuGeom.verts.push_back(glm::vec3(j, _height, i));
			// TODO: push back vertices, and change the y value based on the height value passed
		}
	}

}

