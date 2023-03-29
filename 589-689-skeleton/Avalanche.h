#include "Geometry.h"

extern float repose;

extern float avalanche_amount;

extern int number_of_iterations;

float getHeight(std::vector<float> heights, int width, int length, int x, int y);

void setHeight(std::vector<float> heights, int width, int length, int x, int y, float height);

void apply_avalanching(CPU_Geometry* surface, std::vector<float> heights, int width, int length, float repose, float number_of_iterations);

bool check_repose_down(CPU_Geometry* surface, std::vector<float> heights, int width, int length, int x, int y);

bool check_repose_left(CPU_Geometry* surface, std::vector<float> heights, int width, int length, int x, int y);

bool check_repose_right(CPU_Geometry* surface, std::vector<float> heights, int width, int length, int x, int y);

bool check_repose_up(CPU_Geometry* surface, std::vector<float> heights, int width, int length, int x, int y);

void avalanche_up(CPU_Geometry* surface, std::vector<float> heights, int width, int length, int x, int y);

void avalanche_left(CPU_Geometry* surface, std::vector<float> heights, int width, int length, int x, int y);

void avalanche_down(CPU_Geometry* surface, std::vector<float> heights, int width, int length, int x, int y);

void avalanche_right(CPU_Geometry* surface, std::vector<float> heights, int width, int length, int x, int y);
