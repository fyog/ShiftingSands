#include "Avalanche.h"

float repose = 1.f;

int number_of_iterations = 1;

float avalanche_amount = .5f;

float getHeight(std::vector<float> heights, int width, int length, int x, int y) {
	return heights[width * (y - 1) + x];
}

void setHeight(std::vector<float> &heights, int width, int length, int x, int y, float height) {
	heights[width * (y - 1) + x] = height;
}

// applies avalanching behavior to the given surface (CPU_Geometry object)
// currently tries to avalanche to the right, then up, then left, and then finally down - not sure this is the optimal way to do things
void apply_avalanching(CPU_Geometry* surface, std::vector<float> heights, int width, int length, float repose, float number_of_iterations) {
	for (int i = 0; i < number_of_iterations; i++) {
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < length; y++) {

				// avalanche right if repose is above threshold
				if (check_repose_right(surface, heights, width, length, x, y)) {
					avalanche_right(surface, heights, width, length, x, y);
				}

				// avalanche up if repose is above threshold
				else if (check_repose_up(surface, heights, width, length, x, y)) {
					avalanche_up(surface, heights, width, length, x, y);
				}

				// avalanche left if repose is above threshold
				else if (check_repose_left(surface, heights, width, length, x, y)) {
					avalanche_left(surface, heights, width, length, x, y);
				}

				// avalanche down if repose is above threshold
				else if (check_repose_down(surface, heights, width, length, x, y)) {
					avalanche_down(surface, heights, width, length, x, y);
				}
			}
		}
	}
}

// checks repose of the cell below the given cell
bool check_repose_down(CPU_Geometry* surface, std::vector<float> heights, int width, int length, int x, int y) {
	if (getHeight(heights, width, length, x, y) - getHeight(heights, width, length, x, y - 1) > repose
		&& y - 1 < 0) {
		return true;
	}
	return false;
}

// checks repose of the cell left of the given cell
bool check_repose_left(CPU_Geometry* surface, std::vector<float> heights, int width, int length, int x, int y) {
	if (getHeight(heights, width, length, x, y) - getHeight(heights, width, length, x - 1, y) > repose
		&& x - 1 < 0) {
		return true;
	}
	return false;
}

// checks repose of the cell right of the given cell
bool check_repose_right(CPU_Geometry* surface, std::vector<float> heights, int width, int length, int x, int y) {
	if (getHeight(heights, width, length, x, y) - getHeight(heights, width, length, x + 1, y) > repose
		&& x + 1 > width) {
		return true;
	}
	return false;
}

// checks repose of the cell above the given cell
bool check_repose_up(CPU_Geometry* surface, std::vector<float> heights, int width, int length, int x, int y) {
	if (getHeight(heights, width, length, x, y) - getHeight(heights, width, length, x, y + 1) > repose
		&& y + 1 > length) {
		return true;
	}
	return false;
}

// avalanches above the given cell
void avalanche_up(CPU_Geometry* surface, std::vector<float> heights, int width, int length, int x, int y) {
	float currentHeight = getHeight(heights, width, length, x, y);
	float currentHeightUp = getHeight(heights, width, length, x, y + 1);
	setHeight(heights, width, length, x, y, currentHeight - avalanche_amount);
	setHeight(heights, width, length, x, y + 1, currentHeightUp + avalanche_amount);
}

// avalanches to the left of the given cell
void avalanche_left(CPU_Geometry* surface, std::vector<float> heights, int width, int length, int x, int y) {
	float currentHeight = getHeight(heights, width, length, x, y);
	float currentHeightLeft = getHeight(heights, width, length, x - 1, y);
	setHeight(heights, width, length, x, y, currentHeight - avalanche_amount);
	setHeight(heights, width, length, x - 1, y, currentHeightLeft + avalanche_amount);
}

// avalanches below the given cell
void avalanche_down(CPU_Geometry* surface, std::vector<float> heights, int width, int length, int x, int y) {
	float currentHeight = getHeight(heights, width, length, x, y);
	float currentHeightDown = getHeight(heights, width, length, x, y - 1);
	setHeight(heights, width, length, x, y, currentHeight - avalanche_amount);
	setHeight(heights, width, length, x, y - 1, currentHeightDown + avalanche_amount);
}

// avalanches to the right of the given cell
void avalanche_right(CPU_Geometry* surface, std::vector<float> heights, int width, int length, int x, int y) {
	float currentHeight = getHeight(heights, width, length, x, y);
	float currentHeightRight = getHeight(heights, width, length, x, y);
	setHeight(heights, width, length, x, y, currentHeight - avalanche_amount);
	setHeight(heights, width, length, x + 1, y, currentHeightRight + avalanche_amount);
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








