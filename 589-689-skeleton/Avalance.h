#include "Surface.h"
#include "SandCell.h"
#include <vector>
using namespace std;
float repose;
float spread;
float number_of_iterations;

Surface apply_avalanching(Surface surface, float repose, float spread, float number_of_iterations);

float getRepose();

bool check_repose_up(Surface surface, int x, int y);
bool check_repose_left(Surface surface, int x, int y);
bool check_repose_right(Surface surface, int x, int y);
bool check_repose_down(Surface surface, int x, int y);

Surface avalanche_up(Surface, int x, int y);
Surface avalanche_left(Surface, int x, int y);
Surface avalanche_right(Surface, int x, int y);
Surface avalanche_down(Surface, int x, int y);


// does avalanching occur in the diagonal direction?

//bool check_repose_diagonal_1(Surface surface);
//bool check_repose_diagonal_2(Surface surface);
//bool check_repose_diagonal_3(Surface surface);
//bool check_repose_diagonal_4(Surface surface);
