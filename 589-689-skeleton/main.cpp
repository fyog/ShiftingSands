#include <iostream>

// Window.h `#include`s ImGui, GLFW, and glad in correct order.
#include "Window.h"

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"

// CALLBACKS
class MyCallbacks : public CallbackInterface {

public:

	// Constructor - we use values of -1 for attributes that, at the start of
	// the program, have no meaningful/"true" value
	MyCallbacks(ShaderProgram& shader, int screenWidth, int screenHeight)
		: shader(shader)
		, currentFrame(0)
		, leftMouseActiveVal(false)
		, lastLeftPressedFrame(-1)
		, lastRightPressedFrame(-1)
		, screenMouseX(-1.0)
		, screenMouseY(-1.0)
		, screenWidth(screenWidth)
		, screenHeight(screenHeight)
	{}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			shader.recompile();
		}
	}

	virtual void mouseButtonCallback(int button, int action, int mods) {

		// If we click the mouse on the ImGui window, we don't want to log that
		// here. But if we RELEASE the mouse over the window, we do want to
		// know that!
		auto& io = ImGui::GetIO();
		if (io.WantCaptureMouse && action == GLFW_PRESS) return;
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			leftMouseActiveVal = true;
			lastLeftPressedFrame = currentFrame;
		}
		if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
			lastRightPressedFrame = currentFrame;
		}
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
			leftMouseActiveVal = false;
		}
	}

	// Updates the screen width and height, in screen coordinates
	// (not necessarily the same as pixels)
	virtual void windowSizeCallback(int width, int height) {
		screenWidth = width;
		screenHeight = height;
	}

	// Sets the new cursor position, in screen coordinates
	virtual void cursorPosCallback(double xpos, double ypos) {
		screenMouseX = xpos;
		screenMouseY = ypos;
	}

	// Whether the left mouse was pressed down this frame.
	bool leftMouseJustPressed() {
		return lastLeftPressedFrame == currentFrame;
	}

	// Whether the left mouse button is being pressed down at all
	bool leftMouseActive() {
		return leftMouseActiveVal;
	}

	// Whether the right mouse button was pressed down this frame
	bool rightMouseJustPressed() {
		return lastRightPressedFrame == currentFrame;
	}

	// Tell the callbacks object a new frame has begun
	void incrementFrameCount() {
		currentFrame++;
	}

	glm::vec2 getCursorPosGL() {
		glm::vec2 screenPos(screenMouseX, screenMouseY);
		glm::vec2 centredPos = screenPos + glm::vec2(0.5f, 0.5f);
		glm::vec2 scaledToZeroOne = centredPos / glm::vec2(screenWidth, screenHeight);
		glm::vec2 flippedY = glm::vec2(scaledToZeroOne.x, 1.0f - scaledToZeroOne.y);
		return 2.f * flippedY - glm::vec2(1.f, 1.f);
	}

	int indexOfPointAtCursorPos(const std::vector<glm::vec3>& glCoordsOfPointsToSearch, float screenCoordThreshold) {
		std::vector<glm::vec3> screenCoordVerts;
		for (const auto& v : glCoordsOfPointsToSearch) {
			screenCoordVerts.push_back(glm::vec3(glPosToScreenCoords(v), 0.f));
		}
		glm::vec3 cursorPosScreen(screenMouseX + 0.5f, screenMouseY + 0.5f, 0.f);
		for (size_t i = 0; i < screenCoordVerts.size(); i++) {
			glm::vec3 diff = screenCoordVerts[i] - cursorPosScreen;
			if (glm::length(diff) < screenCoordThreshold) {
				return i;
			}
		}
		return -1; // No point within threshold found.
	}

private:
	int screenWidth;
	int screenHeight;
	double screenMouseX;
	double screenMouseY;
	int currentFrame;
	bool leftMouseActiveVal;
	int lastLeftPressedFrame;
	int lastRightPressedFrame;
	ShaderProgram& shader;

	// Converts GL coordinates to screen coordinates.
	glm::vec2 glPosToScreenCoords(glm::vec2 glPos) {

		// Convert the [-1, 1] range to [0, 1]
		glm::vec2 scaledZeroOne = 0.5f * (glPos + glm::vec2(1.f, 1.f));
		glm::vec2 flippedY = glm::vec2(scaledZeroOne.x, 1.0f - scaledZeroOne.y);
		glm::vec2 screenPos = flippedY * glm::vec2(screenWidth, screenHeight);
		return screenPos;
	}
};

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 589/689"); // could set callbacks at construction if desired
	GLDebug::enable();

	// SHADERS
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");
	auto cb = std::make_shared<MyCallbacks>(shader, window.getWidth(), window.getHeight());

	// CALLBACKS
	window.setCallbacks(cb);
	window.setupImGui(); // Make sure this call comes AFTER GLFW callbacks set.

	// GEOMETRY
	CPU_Geometry cpuGeom, cpuGeom2;
	GPU_Geometry gpuGeom;

	// Variables that ImGui will alter.
	float pointSize = 5.f; // Diameter of drawn points
	float color[3] = { 1.f, 0.f, 0.f }; // Color of new points
	bool drawControl = true; // Whether to draw connecting lines
	bool drawPoly = false;
	int selectedPointIndex = -1; // Used for point dragging & deletion
	float u = 0.04f;
	glm::mat3 rotation_matrix = glm::mat3(cos(3.1415f / 2.f), -sin(3.1415f / 2.f), 0.f,
		sin(3.1415f / 2.f), cos(3.1415f / 2.f), 0.f,
		0.f, 0.f, 1.f);
	bool change = false; // Whether any ImGui variable's changed

	// RENDER LOOP
	while (!window.shouldClose()) {

		cpuGeom2.verts.clear();
		cpuGeom2.cols.clear();

		// Tell callbacks object a new frame's begun BEFORE polling events!
		cb->incrementFrameCount();
		glfwPollEvents();

		// If mouse just went down, see if it was on a point
		if (cb->leftMouseJustPressed() || cb->rightMouseJustPressed() && cpuGeom.verts.size() < 3) {
			float threshold = pointSize;
			selectedPointIndex = cb->indexOfPointAtCursorPos(cpuGeom.verts, threshold);
		}

		// If left mouse is clicked
		if (cb->leftMouseJustPressed() && cpuGeom.verts.size() < 3) {
			if (selectedPointIndex < 0) {

				// If we just clicked empty space, add new point
				cpuGeom.verts.push_back(glm::vec3(cb->getCursorPosGL(), 0.f));
				cpuGeom.cols.push_back(glm::vec3(color[0], color[1], color[2]));
				gpuGeom.setVerts(cpuGeom.verts);
				gpuGeom.setCols(cpuGeom.cols);
			}
		}

		// If right mouse click is pressed
		else if (cb->rightMouseJustPressed() && cpuGeom.verts.size() < 3) {
			if (selectedPointIndex >= 0) {

				// If we right-clicked on a vertex, erase it
				cpuGeom.verts.erase(cpuGeom.verts.begin() + selectedPointIndex);
				cpuGeom.cols.erase(cpuGeom.cols.begin() + selectedPointIndex);
				selectedPointIndex = -1; // So that we don't drag in next frame.
				gpuGeom.setVerts(cpuGeom.verts);
				gpuGeom.setCols(cpuGeom.cols);
			}
		}

		// If there are three control points, plot the polynomial
		if (cpuGeom.verts.size() >= 3) {
			for (float u = 0.f; u <= 1.0f; u += 0.01f) {
				cpuGeom2.verts.push_back(cpuGeom.verts[0] + cpuGeom.verts[1] * u + cpuGeom.verts[2] * u * u);
				cpuGeom2.cols.push_back(glm::vec3(0.f, 0.f, 1.f));
				drawPoly = true;
			}
		}

		// Three functions that must be called each new frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Controls");

		// Clear the scene
		if (ImGui::Button("Clear")) {
			change = true;
			cpuGeom.verts.clear();
			cpuGeom.cols.clear();
			cpuGeom2.verts.clear();
			cpuGeom2.cols.clear();
			gpuGeom.setVerts(cpuGeom.verts);
			gpuGeom.setCols(cpuGeom.cols);
		}

		// Affine transformation (rotation by 90 degrees)
		if (ImGui::Button("Rotate 90")) {
			change = true;
			auto tmp = cpuGeom.verts;
			auto tmp2 = cpuGeom2.verts;
			cpuGeom.verts.clear();
			cpuGeom.cols.clear();
			cpuGeom2.verts.clear();
			cpuGeom2.cols.clear();

			// Plot rotated control points
			for (int i = 0; i < 3; i++) {
				cpuGeom.verts.push_back(rotation_matrix * tmp[i]);
				cpuGeom.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
			}

			// Plot polynomial using rotated control points
			for (float u = 0.f; u <= 1.0f; u += 0.01f) {
				cpuGeom2.verts.push_back(cpuGeom.verts[0] + cpuGeom.verts[1] * u + cpuGeom.verts[2] * u * u);
				cpuGeom2.cols.push_back(glm::vec3(0.f, 0.f, 1.f));
				drawPoly = true;
			}

			gpuGeom.setVerts(cpuGeom.verts);
			gpuGeom.setCols(cpuGeom.cols);
		}

		ImGui::Text("Average %.1f ms/frame (%.1f fps)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
		ImGui::Render();

		shader.use();
		gpuGeom.bind();

		glPointSize(pointSize);

		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (drawControl) {
			gpuGeom.setVerts(cpuGeom.verts);
			gpuGeom.setCols(cpuGeom.cols);
			glDrawArrays(GL_POINTS, 0, GLsizei(cpuGeom.verts.size()));
		}

		if (drawPoly) {
			gpuGeom.setVerts(cpuGeom2.verts);
			gpuGeom.setCols(cpuGeom2.cols);
			glDrawArrays(GL_LINE_STRIP, 0, GLsizei(cpuGeom2.verts.size()));
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
