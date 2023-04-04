
#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <limits>
#include <functional>
#include <unordered_map>

// Window.h `#include`s ImGui, GLFW, and glad in correct order.
#include "Window.h"

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "BSpline.h"

#include "GeomLoaderForOBJ.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Avalanche.h"
#include "SandCell.h"
#include "SurfaceRender.h"
#include "Wind.h"

glm::vec3 lookAtPoint = glm::vec3(0,0,0);
float scrollSpeed = 0.01f;

// EXAMPLE CALLBACKS
class Callbacks3D : public CallbackInterface {

public:
	// Constructor. We use values of -1 for attributes that, at the start of
	// the program, have no meaningful/"true" value.
	Callbacks3D(ShaderProgram& shader, int screenWidth, int screenHeight)
		: shader(shader)
		, camera(glm::radians(45.f), glm::radians(45.f), 3.0)
		, aspect(1.0f)
		, rightMouseDown(false)
		, mouseOldX(-1.0)
		, mouseOldY(-1.0)
		, screenWidth(screenWidth)
		, screenHeight(screenHeight)
		, upPressed(false)
		, downPressed(false)
		, rightPressed(false)
		, leftPressed(false)
	{
		updateUniformLocations();
	}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			shader.recompile();
			updateUniformLocations();
		}

		if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
			upPressed = true;
		}
		else if (key == GLFW_KEY_UP && action == GLFW_RELEASE) {
			upPressed = false;
		}

		if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
			downPressed = true;
		}
		else if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE) {
			downPressed = false;
		}

		if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
			rightPressed = true;
		}
		else if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE) {
			rightPressed = false;
		}

		if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
			leftPressed = true;
		}
		else if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE) {
			leftPressed = false;
		}
	}

	virtual void mouseButtonCallback(int button, int action, int mods) {
		// If we click the mouse on the ImGui window, we don't want to log that
		// here. But if we RELEASE the mouse over the window, we do want to
		// know that!
		auto& io = ImGui::GetIO();
		if (io.WantCaptureMouse && action == GLFW_PRESS) return;

		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (action == GLFW_PRESS)			rightMouseDown = true;
			else if (action == GLFW_RELEASE)	rightMouseDown = false;
		}
	}

	// Updates the screen width and height, in screen coordinates
	// (not necessarily the same as pixels)
	virtual void windowSizeCallback(int width, int height) {
		screenWidth = width;
		screenHeight = height;
		aspect = float(width) / float(height);
	}

	virtual void cursorPosCallback(double xpos, double ypos) {
		if (rightMouseDown) {
			camera.incrementTheta(ypos - mouseOldY);
			camera.incrementPhi(xpos - mouseOldX);
		}
		mouseOldX = xpos;
		mouseOldY = ypos;
	}
	virtual void scrollCallback(double xoffset, double yoffset) {
		camera.incrementR(yoffset);
	}

	void viewPipeline() {
		glm::mat4 M = glm::mat4(1.0);
		glm::mat4 V = camera.getView(lookAtPoint);
		glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.01f, 1000.f);
		glUniformMatrix4fv(mLoc, 1, GL_FALSE, glm::value_ptr(M));
		glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(V));
		glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(P));
	}

	void updateShadingUniforms(
		const glm::vec3& lightPos, const glm::vec3& lightCol,
		const glm::vec3& diffuseCol, float ambientStrength, bool texExistence
	)
	{
		// Like viewPipeline(), this function assumes shader.use() was called before.
		glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(lightColLoc, lightCol.r, lightCol.g, lightCol.b);
		glUniform3f(diffuseColLoc, diffuseCol.r, diffuseCol.g, diffuseCol.b);
		glUniform1f(ambientStrengthLoc, ambientStrength);
		glUniform1i(texExistenceLoc, (int)texExistence);
	}

	// Converts the cursor position from screen coordinates to GL coordinates
	// and returns the result.
	glm::vec2 getCursorPosGL() {
		glm::vec2 screenPos(mouseOldX, mouseOldY);
		// Interpret click as at centre of pixel.
		glm::vec2 centredPos = screenPos + glm::vec2(0.5f, 0.5f);
		// Scale cursor position to [0, 1] range.
		glm::vec2 scaledToZeroOne = centredPos / glm::vec2(screenWidth, screenHeight);

		glm::vec2 flippedY = glm::vec2(scaledToZeroOne.x, 1.0f - scaledToZeroOne.y);

		// Go from [0, 1] range to [-1, 1] range.
		return 2.f * flippedY - glm::vec2(1.f, 1.f);
	}

	Camera camera;

	bool getUpPressed() {
		return upPressed;
	}
	bool getDownPressed() {
		return downPressed;
	}
	bool getLeftPressed() {
		return leftPressed;
	}
	bool getRightPressed() {
		return rightPressed;
	}

private:
	// Uniform locations do not, ordinarily, change between frames.
	// However, we may need to update them if the shader is changed and recompiled.
	void updateUniformLocations() {
		mLoc = glGetUniformLocation(shader, "M");
		vLoc = glGetUniformLocation(shader, "V");
		pLoc = glGetUniformLocation(shader, "P");;
		lightPosLoc = glGetUniformLocation(shader, "lightPos");;
		lightColLoc = glGetUniformLocation(shader, "lightCol");;
		diffuseColLoc = glGetUniformLocation(shader, "diffuseCol");;
		ambientStrengthLoc = glGetUniformLocation(shader, "ambientStrength");;
		texExistenceLoc = glGetUniformLocation(shader, "texExistence");;
	}

	int screenWidth;
	int screenHeight;

	bool rightMouseDown;
	float aspect;
	double mouseOldX;
	double mouseOldY;

	bool upPressed;
	bool downPressed;
	bool rightPressed;
	bool leftPressed;

	// Uniform locations
	GLint mLoc;
	GLint vLoc;
	GLint pLoc;
	GLint lightPosLoc;
	GLint lightColLoc;
	GLint diffuseColLoc;
	GLint ambientStrengthLoc;
	GLint texExistenceLoc;

	ShaderProgram& shader;
};

// You may want to make your own class to replace this one.
class ModelInfo {
public:
	ModelInfo(std::string fileName)
		: fileName(fileName)
	{
		// Uses our .obj loader (relying on the tinyobjloader library).
		cpuGeom = GeomLoaderForOBJ::loadIntoCPUGeometry(fileName);
		gpuGeom.bind();
		gpuGeom.setVerts(cpuGeom.verts);
		gpuGeom.setNormals(cpuGeom.normals);
		gpuGeom.setUVs(cpuGeom.uvs);
	}

	void bind() { gpuGeom.bind(); }

	size_t numVerts() { return cpuGeom.verts.size(); }

	bool hasUVs() { return (cpuGeom.uvs.size() > 0); }

private:
	std::string fileName;
	CPU_Geometry cpuGeom;
	GPU_Geometry gpuGeom;
};

void printCPUVerts(CPU_Geometry cpu) {
	for (int i = 0; i < cpu.verts.size(); i++) {
		std::cout << cpu.verts.at(i).x << ", " << cpu.verts.at(i).y << ", " << cpu.verts.at(i).z << ", " << std::endl;
	}
}

void setflagstrue(bool* flags)
{
	//we assume there will be three flags in the array, and we want to set them all true when we see some change.
	for (int i = 0; i < 3; i++)
	{
		flags[i] = true;
	}
}


// ImGui variables
bool randomHeights = false;
bool random_submenu = false;
float random_height = 2.f;
bool avalanche = false;
bool avalanche_submenu = false;
float pillarHeight = 0.f;
bool wind = false;
bool wind_submenu = false;
bool cellChange = false;
bool showCells = true;
int renderMode = 0;
int field_type = 0;
int _order_k = 4;

// Names of wind field types to be displayed in slider
const char* wind_field_type[] = { "Linear", "Radial", "Converging" };

// ImGui panel to control the surface ---------------------------------------------------------------------------------
void sandCellImGui(CPU_Geometry& cpuGeom) {
	ImGui::Begin("Surface Tuning");

	cellChange = false;

	// Names of render modes to be displayed in slider
	const char* renderModeNames[] = { "LERP", "Cubes", "Smooth" };

	cellChange |= ImGui::InputInt("Length (X): ", &_length);
	cellChange |= ImGui::InputInt("Width  (Z): ", &_width);
	cellChange |= ImGui::InputInt("Order k of B-Spline Surface: ", &_order_k);
	cellChange |= ImGui::Checkbox("Random Heights", &random_submenu);

	// randomize cell heights
	if (random_submenu) {
		cellChange |= ImGui::InputFloat("Randomized height threshold: ", &random_height);
		cellChange |= ImGui::Checkbox("Randomize", &randomHeights);
	}

	// avalanche behavior
	ImGui::Checkbox("Avalanching behavior", &avalanche_submenu);
	if (avalanche_submenu) {
		cellChange |= ImGui::InputFloat("Avalanching amount: ", &avalanche_amount);
		cellChange |= ImGui::InputFloat("Iterations: ", &repose);
		cellChange |= ImGui::Checkbox("Avalanche", &avalanche);
	}

	// wind behavior
	ImGui::Checkbox("Wind Behavior", &wind_submenu);
	if (wind_submenu) {
		ImGui::Text("Type of wind field:");
		ImGui::SliderInt(wind_field_type[field_type], &field_type, 0, 2);
		cellChange |= ImGui::InputFloat("Beta", &beta);
		cellChange |= ImGui::InputFloat("Wind threshold height: ", &wind_threshold_height);
		cellChange |= ImGui::InputFloat("Slab size: ", &slab_size);
		cellChange |= ImGui::InputInt("Number of iterations: ", &number_of_iterations_2);
		cellChange |= ImGui::Checkbox("Wind", &wind);
	}

	// otherwise, ask for pillar height
	else {
		cellChange |= ImGui::InputFloat("Height of Pillar", &pillarHeight, 0.1f, 1.f);
	}

	// any time there is a change to the surface parameters, recreate the surface
	if (cellChange) {
		createCells(cpuGeom);
	}

	ImGui::Checkbox("Render Cells", &showCells);
	if (showCells) {
		ImGui::Text("Number of Draw Calls:");
		ImGui::SliderInt(renderModeNames[renderMode], &renderMode, 0, 2);
	}

	ImGui::End();
}

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 589/689"); // could set callbacks at construction if desired

	GLDebug::enable();

	// SHADERS
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	auto cb = std::make_shared<Callbacks3D>(shader, window.getWidth(), window.getHeight());
	// CALLBACKS
	window.setCallbacks(cb);

	window.setupImGui(); // Make sure this call comes AFTER GLFW callbacks set.

	//Texture cowtex = Texture("./textures/spot/spot_texture.png", GL_LINEAR);
	Texture sandtex = Texture("./textures/spot/spot_texture.png", GL_LINEAR); //had to put it at the same path as the old cow texture because for some reason it wasn't working otherwise.
	sandtex.bind();
	//std::cout << "Texture is " << cowtex.getDimensions().x << " by " << cowtex.getDimensions().y << "\n";

	// Say we're using textures (if the model supports them).
	bool texExistence = true;

	// Some variables for shading that ImGui may alter.
	glm::vec3 lightPos(0.f, 35.f, -35.f);
	glm::vec3 lightCol(1.f);
	glm::vec3 diffuseCol(0.f, 0.f, 0.f);
	float ambientStrength = 0.035f;
	bool simpleWireframe = false;

	// Set the initial, default values of the shading uniforms.
	shader.use();
	cb->updateShadingUniforms(lightPos, lightCol, diffuseCol, ambientStrength, texExistence);

	CPU_Geometry cells_cpu;
	//CPU_Geometry cells_patch_cpu;
	CPU_Geometry lerpline;
	CPU_Geometry cubeobj;

	GPU_Geometry gpu_obj;

	createCells(cells_cpu);
	//preparecellsforrender(cells_cpu, &lerpline);
	renderCells(cells_cpu);

	cubesRender(cells_cpu, &cubeobj);

	CPU_Geometry splinesurf;
	CPU_Geometry zigcpu;
	bool debug = true;
	if (debug) std::cout << "about to call placesurfacevecs() in main()\n";
	
	placesurfacevecs(cells_cpu, &splinesurf, getWidth(), getLength(), _order_k);
	//if (debug) std::cout << "placesurfacevecs() successful. now doing zigzagdraw()\n";
	//zigzagdraw(splinesurf, &zigcpu, getWidth(), getLength());
	splineframe(splinesurf, &zigcpu, getWidth(), getLength());
	//if (debug) std::cout << "zigzagdraw() successful\n";
	//int knownwid = 4;
	//int knownlen = 4;
	bool changecheck[3];
	//for (int i = 0; i < 3; i++)
	//{
	//	changecheck[i] = false;
	//}
	setflagstrue(changecheck);
	std::cout << "about to enter render loop\n";

	// RENDER LOOP ----------------------------------------------------------------------------------------------------
	while (!window.shouldClose()) {
		glfwPollEvents();

		// Three functions that must be called each new frame.
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Render Info.");

		bool change = false; // Whether any ImGui variable's changed.

		// Framerate display, in case you need to debug performance.
		ImGui::Text("Average %.1f ms/frame (%.1f fps)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		ImGui::Separator();
		ImGui::Text("Camera Look At Point");
		ImGui::InputFloat("x", &lookAtPoint.x);
		ImGui::InputFloat("y", &lookAtPoint.y);
		ImGui::InputFloat("z", &lookAtPoint.z);
		ImGui::InputFloat("Scroll speed: ", &scrollSpeed);
		ImGui::End();

		// ImGui to control sand cell data structure
		sandCellImGui(cells_cpu);

		if (cb->getUpPressed()) {
			lookAtPoint.x += scrollSpeed;
		}
		else if (cb->getDownPressed()) {
			lookAtPoint.x -= scrollSpeed;
		}
		if (cb->getRightPressed()) {
			lookAtPoint.z += scrollSpeed;
		}
		else if (cb->getLeftPressed()) {
			lookAtPoint.z -= scrollSpeed;
		}

		ImGui::Render();

		gpu_obj.bind();

		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_FRAMEBUFFER_SRGB);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, (simpleWireframe ? GL_LINE : GL_FILL));

		shader.use();

		// recreate with random heights, making sure no height is above the random_height variable
		if (randomHeights) {
			pillarSetup(cells_cpu, random_height);
			randomHeights = false;
		}

		// otherwise, recreate the surface using pillarHeight
		else {
			pillarSetup(cells_cpu, pillarHeight);
		}

		if (avalanche) {
			apply_avalanching(&cells_cpu, heights, getWidth(), getLength(), repose, number_of_iterations);
			avalanche = false;
		}

		if (wind) {
			auto wind_field_gen = generate_wind_field(wind_field_type[field_type], getWidth(), getLength());
			apply_wind(&cells_cpu, heights, wind_field_gen, getWidth(), getLength(), number_of_iterations_2);
			wind = false;
		}

		// Boilerplate change check -- may need to change name
		if (change)
		{
			// If any of our shading values was updated, we need to update the
			// respective GLSL uniforms.
			//cb->updateShadingUniforms(lightPos, lightCol, diffuseCol, ambientStrength, texExistence);
		}
		cb->viewPipeline();

		//glDrawArrays(GL_LINE_STRIP, 0, GLsizei(splineob.verts.size())); 
		//glDrawArrays(GL_TRIANGLES, 0, GLsizei(models.at(selectedModelName).numVerts())); //Commented this out to test b-spline -Reid
		
		if (getCellChange())
		{
			setflagstrue(changecheck); //All flags will be set to true when the control points are changed
			//we use an array of flags to check if we should redraw the objects instead of a single flag since the b-spline surface wasn't redrawing correctly when we only checked getCellChange()
		}

		// Toggle Render
		if (getShowCells()) {

			// LERP Render mode
			if (getRenderMode() == 0) {
				//renderCells(cells_cpu);
				if (changecheck[0])
				{
					//createCells(cells_cpu);
					preparecellsforrender(cells_cpu, &lerpline);
					
					changecheck[0] = false;
				}

				cb->updateShadingUniforms(lightPos, lightCol, diffuseCol, ambientStrength, false);
				rendertest(lerpline, &gpu_obj); 
			}

			// Cubes Render
			else if (getRenderMode() == 1) {
				//cubesRender(cells_cpu);
				if (changecheck[1])
				{
					cubesRender(cells_cpu, &cubeobj);
					changecheck[1] = false;
				}
				cb->updateShadingUniforms(lightPos, lightCol, diffuseCol, ambientStrength, false);
				rendertest(cubeobj, &gpu_obj);
			}

			// Smooth Render
			else if (getRenderMode() == 2) {
				if (changecheck[2])
				{
					//if (debug) printCPUVerts(cells_cpu);
					placesurfacevecs(cells_cpu, &splinesurf, getWidth(), getLength(), _order_k);
					//zigzagdraw(splinesurf, &zigcpu, getWidth(), getLength());
					//splineframe(splinesurf, &zigcpu, getWidth(), getLength());
					drawtexturedsurface(&splinesurf, &zigcpu, getWidth(), getLength());
					changecheck[2] = false;
				}
				//textures.at(selectedTexName).bind();
				cb->updateShadingUniforms(lightPos, lightCol, diffuseCol, ambientStrength, true);
				renderpoly(zigcpu, &gpu_obj, &sandtex);
			}
		}


		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		window.swapBuffers();
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}
