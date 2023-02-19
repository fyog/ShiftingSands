#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <limits>
#include <functional>
#include <math.h>
#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"
#include "Camera.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

struct State {
	bool pause = false;
	bool restart = false;
	glm::vec3 viewPos = glm::vec3(0, 0, 2);

	bool pauseStatus() {
		return pause;
	}
	void resetRestart() {
		restart = false;
	}
	bool getRestart() {
		return restart;
	}
	glm::vec3 getViewPos() {
		return viewPos;
	}
};

struct Sphere {
	int object_ID;
	glm::vec3 position;

	//Setter for object ID
	void setObjectId(int ID) {
		object_ID = ID;
	}


	// Getter for object ID
	int objectID() {
		return object_ID;
	}

	//Setter for position
	void setPosition(glm::vec3 pos) {
		position = pos;
	}

	// Getter for position
	glm::vec3 getPosition() {
		return position;
	}

	
	// Generates a sphere, along with normals and texture coordinates. This method takes as input the radius of the
	// desired sphere and CPU_Geometry object associated with it, as well as the desired number of stacks (lines of
	// longitude) and sectors (lines of latitude) that the sphere should have.
	void generateSphere(double r, int horizontal_count, int vertical_count, CPU_Geometry& sphere, bool normals_inverted
		, int object_ID) {

		// Set fields
		this -> object_ID = object_ID;
	
		// Preliminary calculations
		double step_vertical = acos(-1) / (double)vertical_count;
		double step_horizontal = 2 * acos(-1) / (double)horizontal_count;

		// Method variables
		double x, y, xy, z;
		double nx, ny, nz;
		double angle_horizontal, angle_vertical;
		double s, t;

		// Generate sphere ============================================================================================

		

		// For each vertical stack...
		for (int i = 0; i <= vertical_count; i++) {
			angle_vertical = acos(-1) / 2 - i * step_vertical;
			xy = r * cosf(angle_vertical);
			z = r * sinf(angle_vertical);
			double difference = z - r;
			setPosition(glm::vec3(0.f, 0.f , difference));

			// For each horizontal sector...
			for (int j = 0; j <= horizontal_count; ++j) {
				angle_horizontal = j * step_horizontal;

				// Sphere vertices
				x = xy * cosf(angle_horizontal);
				y = xy * sinf(angle_horizontal);
				sphere.verts.push_back(glm::vec3(x, y, z));

				// Vertex normals
				if (!normals_inverted) {
					nx = x / r;
					ny = y / r;
					nz = z / r;
					sphere.normals.push_back(glm::vec3(nx, ny, nz));
				}
				else {
					nx = x / r;
					ny = y / r;
					nz = z / r;
					sphere.normals.push_back(glm::vec3(-nx, -ny, -nz));
				}

				// Texture coordinates
				s = (double)j / horizontal_count;
				t = (double)i / vertical_count;
				sphere.texCoords.push_back(glm::vec2(s, t));
			}
		}

		// Copy sphere vertices to temp vector
		std::vector<glm::vec3> temp, temp2;
		std::vector<glm::vec2> temp3;
		for (int i = 0; i < sphere.verts.size(); ++i) {
			temp.push_back(sphere.verts[i]);
			temp2.push_back(sphere.normals[i]);
			temp3.push_back(sphere.texCoords[i]);
		}

		// Vector for holding triangle indices
		std::vector<int> indices;

		// Find triangle indices, this next section of the method was wrote using
		// http://www.songho.ca/opengl/gl_sphere.html as a reference. I made sure I understood everything before I
		// implemented anything from that website.

		int k1, k2;

		// For each vertical stack...
		for (int i = 0; i < vertical_count; ++i)
		{
			k1 = i * (horizontal_count + 1);
			k2 = k1 + horizontal_count + 1;

			// For each horizontal sector...
			for (int j = 0; j <= horizontal_count; ++j, ++k1, ++k2)
			{
				// Triangle: k1 => k2 => k1+1
				if (i != 0)
				{
					indices.push_back(k1);
					indices.push_back(k2);
					indices.push_back(k1 + 1);
				}

				// Triangle: k1+1 => k2 => k2+1
				if (i != (vertical_count - 1))
				{
					indices.push_back(k1 + 1);
					indices.push_back(k2);
					indices.push_back(k2 + 1);
				}
			}
		}

		// Code that written with reference to http://www.songho.ca/opengl/gl_sphere.html ends here.

		// Clear all vertices
		sphere.verts.clear();
		sphere.normals.clear();
		sphere.texCoords.clear();

		// Copy the vertices, normals, and texture coordinates of the triangles into sphere.verts, sphere.normals, and
		// sphere.texCoords
		for (int i = 0; i < indices.size(); ++i) {
			sphere.verts.push_back(temp[indices[i]]);
			sphere.normals.push_back(temp2[indices[i]]);
			sphere.texCoords.push_back(temp3[indices[i]]);
		}
	}
};

// This method updates the GPU_Geometry object that is passed to it using the CPU_Geometry that is passed as the
// second parameter.
void updateGPUGeometry(GPU_Geometry &gpuGeom, CPU_Geometry const &cpuGeom) {
	gpuGeom.bind();
	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setNormals(cpuGeom.normals);
	gpuGeom.setTexCoords(cpuGeom.texCoords);
}

// Callbacks class, which implements CallbackInterface ================================================================
class Assignment4 : public CallbackInterface {

public:

	Camera camera;

	// Default constructor
	Assignment4() : camera(0.0, 0.0, 2.0), aspect(1.0f) {}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_P && action == GLFW_PRESS) {
			if (!state.pause) {
				state.pause = true;
			}
			else if (state.pause) {
				state.pause = false;
			}
		}
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
				state.restart = true;
		}
	}

	virtual void mouseButtonCallback(int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (action == GLFW_PRESS) {
				rightMouseDown = true;
			} else if (action == GLFW_RELEASE) {
				rightMouseDown = false;
			}
		}
	}

	virtual void cursorPosCallback(double xpos, double ypos) {
		if (rightMouseDown) {
			double dx = xpos - mouseOldX;
			double dy = ypos - mouseOldY;
			camera.incrementTheta(dy);
			camera.incrementPhi(dx);
			this ->getState().viewPos = glm::vec3(dx, dy, 0.f);
		}
		mouseOldX = xpos;
		mouseOldY = ypos;
		
	}

	virtual void scrollCallback(double xoffset, double yoffset) {
		camera.incrementR(yoffset);
	}

	virtual void windowSizeCallback(int width, int height) {

		// The CallbackInterface::windowSizeCallback will call glViewport for us
		CallbackInterface::windowSizeCallback(width,  height);
		aspect = float(width)/float(height);

	}
	State getState() {
		return state;
	}

	// Sets up the viewing pipeline -----------------------------------------------------------------------------------
	void viewPipeline(ShaderProgram &sp) {
		glm::mat4 M = glm::mat4(1.0);
		glm::mat4 V = camera.getView();
		glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.01f, 1000.f);

		// Model matrix
		GLint uniMat = glGetUniformLocation(sp, "M");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(M));

		// Viewing matrix
		uniMat = glGetUniformLocation(sp, "V");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(V));

		// Perspective matrix
		uniMat = glGetUniformLocation(sp, "P");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(P));
	}

private:

	struct State state;
	bool rightMouseDown = false;
	float aspect;
	double mouseOldX;
	double mouseOldY;
};
// Main method ====================================================================================================
int main() {
	Log::debug("Starting main");

	// Window
	glfwInit();
	Window window(800, 800, "CPSC 453"); // can set callbacks at construction if desired
	GLDebug::enable();

	// Callbacks
	auto a4 = std::make_shared<Assignment4>();
	window.setCallbacks(a4);

	// Shaders
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	// Generate earth sphere --------------------------------------------------------------------------------------
	// Draw the scene
	struct Sphere earth;
	CPU_Geometry earth_cpu;
	earth.generateSphere(0.6f, 50, 50, earth_cpu, false, 0);
	GPU_Geometry earth_gpu;
	Texture texture("1.jpg", GL_NEAREST);
	
	// Generate night sky sphere --------------------------------------------------------------------------------------
	// Draw the scene
	struct Sphere night_sky;
	CPU_Geometry night_sky_cpu;
	night_sky.generateSphere(100, 500, 500, night_sky_cpu, true, 1);
	GPU_Geometry ns_gpu;
	Texture texture2("2.jpg", GL_NEAREST);
	
	// Generate moon sphere ---------------------------------------------------------------------------------------
	// Draw the scene
	struct Sphere moon;
	CPU_Geometry moon_cpu;
	moon.generateSphere(0.2f, 50, 50, moon_cpu, false, 2);
	GPU_Geometry moon_gpu;
	Texture texture3("3.jpg", GL_NEAREST);
	
	// Generate sun sphere ---------------------------------------------------------------------------------------
	// Draw the scene
	struct Sphere sun;
	CPU_Geometry sun_cpu;
	sun.generateSphere(.9f, 50, 50, sun_cpu, false, 3);
	GPU_Geometry sun_gpu;
	Texture texture4("4.jpg", GL_NEAREST);

	// Render loop variables
	double theta = 0.0;
	double theta2 = 0.0;
	double theta3 = 0.0;

	// Render Loop ------------------------------------------------------------------------------------------------
	while (!window.shouldClose()) {

		// Check for events/changes
		glfwPollEvents();
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_LIGHTING);
		glEnable(GL_FRAMEBUFFER_SRGB);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		shader.use();

		a4->viewPipeline(shader);
		if (!a4->getState().pauseStatus()) {

			// Draw the earth -------------------------------------------------------------------------------------
			// Set uniform locations for object ID and transformation matrix
			GLuint location1 = glGetUniformLocation(shader.getID(), "object_ID_vert");
			GLuint location1_2 = glGetUniformLocation(shader.getID(), "Te");
			GLuint location1_3 = glGetUniformLocation(shader.getID(), "light_diff");
			GLuint location1_4 = glGetUniformLocation(shader.getID(), "ePos");

			// Light vector
			glm::vec3 light_diff = glm::vec3(0.3, 0.3, 0.3);

			// Rotation of the earth
			glm::mat4 rotationMatrix = glm::mat4(
				cos(theta), -sin(theta), 0.f, 0.f,
				sin(theta), cos(theta), 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				0.f, 0.f, 0.f, 1.f
			);

			// Translation of the earth
			glm::mat4 translationMatrix = glm::mat4(
				1.f, 0.f, 0.f, 0.f,
				0.f, 1.f, 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				2.f, 2.f, 0.f, 1.f
			);

			// Multiply translation matrix and rotation matrix to get transformation matrix
			glm::mat4 transformationMatrix = rotationMatrix * translationMatrix;

			// Fill uniform locations with appropriate data
			glUniform1i(location1, earth.objectID());
			glUniformMatrix4fv(location1_2, 1, GL_FALSE, (const GLfloat*)&transformationMatrix);
			glUniform3fv(location1_3, (GLsizei)3, (const GLfloat*)&light_diff);
			glm::vec3 ePos = earth.getPosition();
			glUniform3fv(location1_4, (GLsizei) ePos.length(), (const GLfloat*) &ePos);

			// Draw
			updateGPUGeometry(earth_gpu, earth_cpu);
			texture.bind();
			glDrawArrays(GL_TRIANGLES, 0, (GLsizei)earth_cpu.verts.size());
			texture.unbind();

			// Draw the moon --------------------------------------------------------------------------------------
			// Set uniform location for object ID and transformation matrix
			GLuint location2 = glGetUniformLocation(shader.getID(), "object_ID_vert");
			GLuint location2_2 = glGetUniformLocation(shader.getID(), "Tm");
			GLuint location2_3 = glGetUniformLocation(shader.getID(), "light_diff");

			// Light vector
			light_diff = glm::vec3(0.3, 0.3, 0.3);

			// Rotation of the moon at same speed of earth's rotation
			rotationMatrix = glm::mat4(
				cos(theta), -sin(theta), 0.f, 0.f,
				sin(theta), cos(theta), 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				0.f, 0.f, 0.f, 1.f
			);

			// Translation of the moon to centre of earth
			translationMatrix = glm::mat4(
				1.f, 0.f, 0.f, 0.f,
				0.f, 1.f, 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				2.f, 2.f, 0., 1.f
			);

			// Rotation of the moon itself
			glm::mat4 rotationMatrix2 = glm::mat4(
				cos(theta3), -sin(theta3), 0.f, 0.f,
				sin(theta3), cos(theta3), 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				0.f, 0.f, 0.f, 1.f
			);

			// Translation of the moon from the earth
			glm::mat4 translationMatrix2 = glm::mat4(
				1.f, 0.f, 0.f, 0.f,
				0.f, 1.f, 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				.8f, .8f, .0f, 1.f
			);

			// Multiply translation matrix and rotation matrix to get transformation matrix
			transformationMatrix = rotationMatrix * translationMatrix * rotationMatrix2 *
				translationMatrix2;

			// Fill uniform locations with appropriate data
			glUniform1i(location2, moon.objectID());
			glUniformMatrix4fv(location2_2, 1, GL_FALSE, (const GLfloat*)&transformationMatrix);
			glUniform3fv(location2_3, (GLsizei)3, (const GLfloat*)&light_diff);


			// Draw
			updateGPUGeometry(moon_gpu, moon_cpu);
			texture3.bind();
			glDrawArrays(GL_TRIANGLES, 0, (GLsizei)moon_cpu.verts.size());
			texture3.unbind();

			// Draw the sun ---------------------------------------------------------------------------------------
			// Set uniform location for object ID and transformation matrix
			GLuint location4 = glGetUniformLocation(shader.getID(), "object_ID_vert");
			GLuint location4_2 = glGetUniformLocation(shader.getID(), "Ts");
			GLuint location4_3 = glGetUniformLocation(shader.getID(), "light_diff");

			// Light vector
			light_diff = glm::vec3(0.3, 0.3, 0.3);
		
			// Translation of the sun
			translationMatrix = glm::mat4(
				1.f, 0.f, 0.f, 0.f,
				0.f, 1.f, 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				0.f, 0.f, 0.f, 1.f
			);

			// Rotation of the sun
			rotationMatrix = glm::mat4(
				cos(theta2), -sin(theta2), 0.f, 0.f,
				sin(theta2), cos(theta2), 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				0.f, 0.f, 0.f, 1.f
			);

			// Multiply translation matrix and rotation matrix to get transformation matrix
			transformationMatrix = rotationMatrix * translationMatrix;

			// Fill uniform locations with appropriate data
			glUniform1i(location4, sun.objectID());
			glUniformMatrix4fv(location4_2, 1, GL_FALSE, (const GLfloat*)&transformationMatrix);
			glUniform3fv(location4_3, (GLsizei)3, (const GLfloat*)&light_diff);

			updateGPUGeometry(sun_gpu, sun_cpu);
			texture4.bind();
			glDrawArrays(GL_TRIANGLES, 0, (GLsizei)sun_cpu.verts.size());
			texture4.unbind();

			// Fill uniform locations with appropriate data
			GLuint location3 = glGetUniformLocation(shader.getID(), "object_ID_vert");
			glUniform1i(location3, night_sky.objectID());

			// Draw
			updateGPUGeometry(ns_gpu, night_sky_cpu);
			texture2.bind();
			glDrawArrays(GL_TRIANGLES, 0, (GLsizei)night_sky_cpu.verts.size());
			texture2.bind();

			// Clean-up
			glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui
			window.swapBuffers();

			// Increment angles
			if (!a4->getState().pauseStatus()) {
				theta += 0.002f;
				theta2 += 0.005f;
				theta3 += 0.01f;
			}
		}

		// Restart command
		if (a4->getState().getRestart()) {

		}
	}
	glfwTerminate();
	return 0;
}

