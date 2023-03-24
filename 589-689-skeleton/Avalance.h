#include "Surface.h"
#include "SandCell.h"
#include <vector>
using namespace std;
float repose;
float spread;
float number_of_iterations;

Surface apply_avalanching(Surface surface, float repose, float spread, float number_of_iterations) {
	auto tmp = surface;
	for (int x = 0; x < surface.getLength(); x++) {
		for (int y = 0; y < surface.getWidth(); y++) {

		}
	}
	return tmp;
}
