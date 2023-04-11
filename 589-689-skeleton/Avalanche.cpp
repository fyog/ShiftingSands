#include "Avalanche.h"
#include "SandCell.h"

float repose = 1.f;

int number_of_iterations = 1;

float avalanche_amount = .5f;

// applies avalanching behavior to the given surface (CPU_Geometry object)
// currently tries to avalanche to the right, then up, then left, and then finally down - not sure this is the optimal way to do things
void apply_avalanching(CPU_Geometry& surface, std::vector<float> &heights, float repose, float number_of_iterations) {
	std::cout << "heights length: " << heights.size() << "\n";
	for (int i = 0; i < number_of_iterations; i++) {
		for (int x = 0; x < _length; x++) {
			for (int y = 0; y < _width; y++) {

				// avalanche right if repose is above threshold
				if (check_repose_right(surface, heights, x, y)) {
					//avalanche_right(surface, heights, x, y);
				}

				// avalanche up if repose is above threshold
				else if (check_repose_up(surface, heights,  x, y)) {
					//avalanche_up(surface, heights, x, y);
				}

				// avalanche left if repose is above threshold
				else if (check_repose_left(surface, heights, x, y)) {
					//avalanche_left(surface, heights,  x, y);
				}

				// avalanche down if repose is above threshold
				else if (check_repose_down(surface, heights, x, y)) {
					//avalanche_down(surface, heights, x, y);
				}
			}
		}
	}
}

// checks repose of the cell below the given cell
bool check_repose_down(CPU_Geometry& surface, std::vector<float> &heights, int x, int y) {
	if (getHeight(heights, x, y) - getHeight(heights, x, y - 1) > repose) {
		return true;
	}
	return false;
}

// checks repose of the cell left of the given cell
bool check_repose_left(CPU_Geometry& surface, std::vector<float> &heights, int x, int y) {
	if (getHeight(heights, x, y) - getHeight(heights, x - 1, y) > repose) {
		return true;
	}
	return false;
}

// checks repose of the cell right of the given cell
bool check_repose_right(CPU_Geometry& surface, std::vector<float> &heights, int x, int y) {
	if (getHeight(heights, x, y) - getHeight(heights, x + 1, y) > repose) {
		return true;
	}
	return false;
}

// checks repose of the cell above the given cell
bool check_repose_up(CPU_Geometry& surface, std::vector<float> &heights, int x, int y) {
	if (getHeight(heights, x, y) - getHeight(heights, x, y + 1) > repose) {
		return true;
	}
	return false;
}

// avalanches above the given cell
void avalanche_up(CPU_Geometry& surface, std::vector<float> &heights, int x, int y) {
	float currentHeight = getHeight(heights, x, y);
	float currentHeightUp = getHeight(heights, x, y + 1);

	// checks for bedrock
	float max_avalanche_amount = avalanche_amount;
	//if (currentHeight - avalanche_amount < 0) max_avalanche_amount = currentHeight;

	setHeight(heights, x, y, currentHeight - max_avalanche_amount);
	setHeight(heights, x, y + 1, currentHeightUp + max_avalanche_amount);
}

// avalanches to the left of the given cell
void avalanche_left(CPU_Geometry& surface, std::vector<float> &heights, int x, int y) {
	float currentHeight = getHeight(heights, x, y);
	float currentHeightLeft = getHeight(heights, x - 1, y);

	// checks for bedrock
	float max_avalanche_amount = avalanche_amount;
	//if (currentHeight - avalanche_amount < 0) max_avalanche_amount = currentHeight;

	setHeight(heights, x, y, currentHeight - max_avalanche_amount);
	setHeight(heights, x - 1, y, currentHeightLeft + max_avalanche_amount);
}

// avalanches below the given cell
void avalanche_down(CPU_Geometry& surface, std::vector<float> &heights, int x, int y) {
	float currentHeight = getHeight(heights, x, y);
	float currentHeightDown = getHeight(heights, x, y - 1);

	// checks for bedrock
	float max_avalanche_amount = avalanche_amount;
	//if (currentHeight - avalanche_amount < 0) max_avalanche_amount = currentHeight;

	setHeight(heights, x, y, currentHeight - max_avalanche_amount);
	setHeight(heights, x, y - 1, currentHeightDown + max_avalanche_amount);
}

// avalanches to the right of the given cell
void avalanche_right(CPU_Geometry& surface, std::vector<float> &heights, int x, int y) {
	float currentHeight = getHeight(heights, x, y);
	float currentHeightRight = getHeight(heights, x, y);

	// checks for bedrock
	float max_avalanche_amount = avalanche_amount;
	//if (currentHeight - avalanche_amount < 0) max_avalanche_amount = currentHeight;

	setHeight(heights, x, y, currentHeight - max_avalanche_amount);
	setHeight(heights, x + 1, y, currentHeightRight + max_avalanche_amount);
}

// diagonals?
//bool check_repose_diagonal_1(Surface surface, int x, int y) {
//	if (surface.getCell(x, y).getHeight() - surface.getCell(x + 1, y +1).getHeight() > repose) {
//		return true;
//	}
//	return false;
//}
//bool check_repose_diagonal_2(Surface surface, int x, int y) {
//	if (surface.getCell(x, y).getHeight() - surface.getCell(x + 1, y -1).getHeight() > repose) {
//		return true;
//	}
//	return false;
//}
//bool check_repose_diagonal_3(Surface surface, int x, int y) {
//	if (surface.getCell(x, y).getHeight() - surface.getCell(x - 1, y + 1).getHeight() > repose) {
//		return true;
//	}
//	return false;
//}
//bool check_repose_diagonal_4(Surface surface, int x, int y) {
//	if (surface.getCell(x, y).getHeight() - surface.getCell(x - 1, y - 1).getHeight() > repose) {
//		return true;
//	}
//	return false;
//}








