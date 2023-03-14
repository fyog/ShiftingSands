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

#include "SandCell.h"
#include "Surface.h"
#include "SurfaceRender.h"

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
	{
		updateUniformLocations();
	}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			shader.recompile();
			updateUniformLocations();
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
		glm::mat4 V = camera.getView();
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

void setflagstrue(bool* flags)
{
	//we assume there will be three flags in the array, and we want to set them all true when we see some change.
	for (int i = 0; i < 3; i++)
	{
		flags[i] = true;
	}
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


	// A "dictionary" that maps models' ImGui display names to their ModelInfo.
	// Because ModelInfo has no default constructor
	// (and there's no good one for it in its current form)
	// We have to use .at() and .emplace() instead of "[]" notation.
	// See: https://stackoverflow.com/questions/29826155/why-a-default-constructor-is-needed-using-unordered-map-and-tuple
	std::unordered_map<std::string, ModelInfo> models;
	models.emplace("Cow", ModelInfo("./models/spot/spot_triangulated.obj"));
	models.emplace("Fish", ModelInfo("./models/blub/blub_triangulated.obj"));
	models.emplace("Torus", ModelInfo("./models/torus.obj"));

	// Select first model by default.
	std::string selectedModelName = models.begin()->first;
	models.at(selectedModelName).bind(); // Bind it.

	// A "dictionary" that maps textures' ImGui display names to their Texture.
	// Because Texture has no default constructor
	// (and there's no good one for it in its current form)
	// We have to use .at() and .emplace() instead of "[]" notation.
	// See: https://stackoverflow.com/questions/29826155/why-a-default-constructor-is-needed-using-unordered-map-and-tuple
	std::unordered_map<std::string, Texture> textures;
	textures.emplace("Cow", Texture("./textures/spot/spot_texture.png", GL_LINEAR));
	textures.emplace("Fish", Texture("./textures/blub/blub_texture.png", GL_LINEAR));
	const std::string noTexName = "None";

	// Select first texture by default.
	std::string selectedTexName = textures.begin()->first;
	textures.at(selectedTexName).bind(); // Bind it.

	// Say we're using textures (if the model supports them).
	bool texExistence = models.at(selectedModelName).hasUVs();

	// Some variables for shading that ImGui may alter.
	glm::vec3 lightPos(0.f, 35.f, -35.f);
	glm::vec3 lightCol(1.f);
	glm::vec3 diffuseCol(1.f, 0.f, 0.f);
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

	Surface terrain = Surface();
	
	terrain.createCells(cells_cpu);
	terrain.prepareCellsForRender(cells_cpu, &lerpline);
	//terrain.cubesRender(cells_cpu, &cubeobj);

	CPU_Geometry splinesurf;
	CPU_Geometry zigcpu;
	bool debug = false;
	if (debug) std::cout << "about to call placesurfacevecs() in main()\n";
	
	placesurfacevecs(cells_cpu, &splinesurf, terrain.getWidth(), terrain.getLength());
	if (debug) std::cout << "placesurfacevecs() successful. now doing zigzagdraw()\n";
	zigzagdraw(splinesurf, &zigcpu, 101, 101);
	if (debug) std::cout << "zigzagdraw() successful\n";
	//int knownwid = 4;
	//int knownlen = 4;
	bool changecheck[3];
	for (int i = 0; i < 3; i++)
	{
		changecheck[i] = false;
	}

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		// Three functions that must be called each new frame.
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Sample window.");

		bool change = false; // Whether any ImGui variable's changed.

		// We'll only render with a texture if the model has UVs and a texture was chosen.
		texExistence = (models.at(selectedModelName).hasUVs() && selectedTexName != noTexName);

		// If a texture is not in use, the user can pick the diffuse colour.
		if (!texExistence) change |= ImGui::ColorEdit3("Diffuse colour", glm::value_ptr(diffuseCol));

		// Framerate display, in case you need to debug performance.
		ImGui::Text("Average %.1f ms/frame (%.1f fps)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();

		// ImGui to control sand cell data structure
		terrain.surfaceImGui(cells_cpu);

		ImGui::Render();

		gpu_obj.bind();

		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_FRAMEBUFFER_SRGB);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, (simpleWireframe ? GL_LINE : GL_FILL));

		shader.use();

		// Boilerplate change check -- may need to change name
		if (change)
		{
			// If any of our shading values was updated, we need to update the
			// respective GLSL uniforms.
			cb->updateShadingUniforms(lightPos, lightCol, diffuseCol, ambientStrength, texExistence);
		}
		cb->viewPipeline();
		//glDrawArrays(GL_LINE_STRIP, 0, GLsizei(splineob.verts.size())); 


		//glDrawArrays(GL_TRIANGLES, 0, GLsizei(models.at(selectedModelName).numVerts())); //Commented this out to test b-spline -Reid
		
		if (terrain.change())
		{
			setflagstrue(changecheck); //All flags will be set to true when the control points are changed
			//we use an array of flags to check if we should redraw the objects instead of a single flag since the b-spline surface wasn't redrawing correctly when we only checked getCellChange()
		}


		// Toggle Render
		if (terrain.show()) {

			// LERP Render mode
			if (terrain.getMode() == 0) {
				//renderCells(cells_cpu);
				if (changecheck[0])
				{
					terrain.prepareCellsForRender(cells_cpu, &lerpline);
					changecheck[0] = false;
				}
				rendertest(lerpline, &gpu_obj); 
			}
			// Cubes Render
			else if (terrain.getMode() == 1) {
				//cubesRender(cells_cpu);
				if (changecheck[1])
				{
					//terrain.cubesRender(cells_cpu, &cubeobj);
					changecheck[1] = false;
				}
				rendertest(cubeobj, &gpu_obj);
			}
			// Smooth Render
			else if (terrain.getMode() == 2) {
				if (changecheck[2])
				{
					placesurfacevecs(cells_cpu, &splinesurf, terrain.getWidth(), terrain.getLength());
					zigzagdraw(splinesurf, &zigcpu, 101, 101);
					changecheck[2] = false;
				}
				rendertest(zigcpu, &gpu_obj);
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
/* BOILERPLATE TO BE FACTORED OUT
	// A drop-down box for choosing the 3D model to render.
	if (ImGui::BeginCombo("Model", selectedModelName.c_str()))
	{
		// Iterate over our dictionary's key-val pairs.
		for (auto& keyVal : models) {
			// Check if this key (a model display name) was last selected.
			const bool isSelected = (selectedModelName == keyVal.first);

			// Now check if the user is currently selecting that model.
			// The use of "isSelected" just changes the colour of the box.
			if (ImGui::Selectable(keyVal.first.c_str(), isSelected))
			{
				selectedModelName = keyVal.first;
				keyVal.second.bind(); // Bind the selected model.
			}
			// Sets the initial focus when the combo is opened
			if (isSelected) ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
		change = true;
	}

	// Only display the texture dropdown if applicable.
	if (models.at(selectedModelName).hasUVs())
	{
		// A drop-down box for choosing the texture to use.
		if (ImGui::BeginCombo("Texture", selectedTexName.c_str()))
		{
			// First, display an option to select NO texture!
			const bool noneSelected = selectedTexName == noTexName;
			if (ImGui::Selectable(noTexName.c_str(), noneSelected))
			{
				selectedTexName = noTexName;
			}
			if (noneSelected) ImGui::SetItemDefaultFocus();

			// Then, present our dictionary's contents as other texture options.
			for (auto& keyVal : textures) {
				// Check if this key (a model display name) was last selected.
				const bool isSelected = (selectedTexName == keyVal.first);
				// Now check if the user is currently selecting that texture.
				// The use of "isSelected" just changes the colour of the box.
				if (ImGui::Selectable(keyVal.first.c_str(), isSelected))
				{
					selectedTexName = keyVal.first;
					keyVal.second.bind(); // Bind the selected texture.
				}
				// Sets the initial focus when the combo is opened
				if (isSelected) ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
			change = true;
		}
	}
	*/
	/* BOILERPLATE TO BE FACTORED OUT
			// The rest of our ImGui widgets.
			change |= ImGui::DragFloat3("Light's position", glm::value_ptr(lightPos));
			change |= ImGui::ColorEdit3("Light's colour", glm::value_ptr(lightCol));
			change |= ImGui::SliderFloat("Ambient strength", &ambientStrength, 0.0f, 1.f);
			change |= ImGui::Checkbox("Simple wireframe", &simpleWireframe);
			*/

			/*
					//Test of b-spline curve below
					CPU_Geometry control;
					control.verts.push_back(glm::vec3((-0.75),(-0.5),0.f));
					control.verts.push_back(glm::vec3((-0.5), (-0.5), 0.f));
					control.verts.push_back(glm::vec3((-0.25), (-0.25), 0.f));
					control.verts.push_back(glm::vec3(0.f, 0.5f, 0.f));
					control.verts.push_back(glm::vec3((0.25), (-0.5), 0.f));
					control.verts.push_back(glm::vec3(0.5, (-0.25), 0.f));
					control.verts.push_back(glm::vec3((0.75), 0.f, 0.f)); //in the end, 7 control points are placed for this example curve

					CPU_Geometry splineob;
					GPU_Geometry goodGPU;

					bspline(4, &control, 0.01, &splineob); //See BSpline.cpp for definition of bspline. this will make a curve of order 4, with the control vertices in control (dereferenced here to be passed into a pointer),
					//and it will have 0.01 set as the u-step value, meaning u will step forward a 100 times then find a point on the curve
					goodGPU.setVerts(splineob.verts); // it would probably be better to set the vertices of the GPU object in the rendering loop, this is just here for testing.
				*/

				//void printCPUVerts(CPU_Geometry cpu) {
				//	for (int i = 0; i < cpu.verts.size(); i++) {
				//		std::cout << cpu.verts.at(i).x << ", " << cpu.verts.at(i).y << ", " << cpu.verts.at(i).z << ", " << std::endl;
				//	}
				//}
