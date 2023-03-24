#include "Geometry.h"
#include <vector>


using namespace std;

class Surface {
private:

	std::vector<float> heights;
    std::vector<float> adhesions; 
	int length, width;
	bool cellChange;
	bool showCells;
	int renderMode;
	bool randomHeights;
	float pillarHeight;

public:

	// constructor
	Surface(int width, int height, CPU_Geometry surface);

	// getters and setters
	int getWidth();
	int getLength();
	void setWidth(int width);
	void setLength(int length);
	std::vector<float> getAdhesionVector();
	std::vector<float> getHeightsVector();
	int getMode();
	void setMode(int renderMode);
	float getPillarHeight();
	bool getRandomizedHeights();

	bool change();
	bool show();
	float randNumber(float min, float max);

	void createCells(CPU_Geometry& cpuGeom);
	void createCells(int _width, int _height, CPU_Geometry& cpuGeom);
	void renderCells(CPU_Geometry& input_cpu);
	void renderCells(CPU_Geometry& input_cpu, int _width, int _height);
	void renderCells2Calls(CPU_Geometry& input_cpu);
	void cubesRender(CPU_Geometry& inputCPU, CPU_Geometry* outputCPU);
	void pillarSetup(CPU_Geometry& inputCPU, float _height);
	void prepareCellsForRender(CPU_Geometry input_cpu, CPU_Geometry* output_cpu);
	void sandCellImGui(CPU_Geometry& cpuGeom);
};
