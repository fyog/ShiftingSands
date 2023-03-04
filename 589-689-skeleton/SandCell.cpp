#include "SandCell.h"


void test() {
	// Test value for adhesion
	_adhesion = 10.f;
	_height = 10.f;

	// The idea is that each data type we want to track for each cell is pushed to a
	// vector array. These vector arrays are index aligned, so the data should be
	// tied to a particular point (might be better as a struct)
	for (int j = 0; j < _area_length_z; j++) {
		for (int i = 0; i < _area_length_x; i++) {
			heights.push_back(_height);
			adhesions.push_back(_adhesion);
			// TODO: push back vertices, and change the y value based on the height value passed
		}
	}

}

