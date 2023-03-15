#include "Geometry.h"
#include "SandCell.h"
#include <vector>


using namespace std;

class Surface {
private:

	std::vector <std::vector<SandCell>> discrete_surface;
	int length, width;
	bool cellChange = false;
	bool showCells = true;
	int renderMode = 0;
	bool randomHeights = false;
	float pillarHeight = 1.0f;
	//CPU_Geometry cpuGeom;

public:

	// constructors
	Surface();
	Surface(int width, int height);

	// getters and setters
	int getWidth();
	int getLength();
	void setWidth(int width);
	void setLength(int length);
	SandCell getCell(int x, int y);
	void setCell(SandCell cell, int x, int y);

	int getMode();
	void setMode(int renderMode);
	float getPillarHeight();
	bool getRandomizedHeights();
	bool change();
	bool show();
	
	// other misc. methods
	float randNumber(float min, float max);
	void createCells(CPU_Geometry& cpuGeom);
	void createCells(int _width, int _height, CPU_Geometry& cpuGeom);
	void renderCells(CPU_Geometry& input_cpu);
	void renderCells(CPU_Geometry& input_cpu, int _width, int _height);
	//void renderCells2Calls(CPU_Geometry& input_cpu);
	void cubesRender(CPU_Geometry& inputCPU, CPU_Geometry* outputCPU);
	void pillarSetup(CPU_Geometry& inputCPU, float _height);
	void prepareCellsForRender(CPU_Geometry input_cpu, CPU_Geometry* output_cpu);
	void surfaceImGui(CPU_Geometry& cpuGeom);
};
