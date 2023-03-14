#include "Avalance.h"

Surface apply_avalanching(Surface surface, float repose, float spread, float number_of_iterations) {
	auto tmp = surface;
	for (int x = 0; x < surface.getLength(); x++) {
		for (int y = 0; y < surface.getWidth(); y++) {

		}
	}
	return tmp;
}

float getRepose() {
	return repose;
}

bool check_repose_down(Surface surface, int x, int y) {
	if (surface.getCell(x,y).getHeight() - surface.getCell(x, y - 1).getHeight() > repose) {
		return true;
	}
	return false;
}
bool check_repose_left(Surface surface, int x, int y) {
	if (surface.getCell(x,y).getHeight() - surface.getCell(x - 1, y).getHeight() > repose) {
		return true;
	}
	return false;
}
bool check_repose_right(Surface surface, int x, int y) {
	if (surface.getCell(x, y).getHeight() - surface.getCell(x + 1, y).getHeight() > repose) {
		return true;
	}
	return false;
}
bool check_repose_up(Surface surface, int x, int y) {
	if (surface.getCell(x, y).getHeight() - surface.getCell(x, y + 1).getHeight() > repose) {
		return true;
	}
	return false;
}

Surface avalanche_up(Surface, int x, int y) {
	auto tmp = Surface();
	return tmp;

}

Surface avalanche_left(Surface, int x, int y) {
	auto tmp = Surface();
	return tmp;
}
Surface avalanche_down(Surface, int x, int y) {
	auto tmp = Surface();
	return tmp;
}
Surface avalanche_right(Surface, int x, int y) {
	auto tmp = Surface();
	return tmp;
}


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
