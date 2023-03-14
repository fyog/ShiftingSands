#pragma once
#ifndef SANDCELL_H
#define SANDCELL_H

#include <vector>
#include <random>
#include "Geometry.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"

class SandCell {
	private:

		float height;
		float adhesion;

	public:

		// constructor
		SandCell(int height, float adhesion);

		// getters and setters
		float getHeight();
		void setHeight(float height);
		float getAdhesion();
		void setAdhesion(float adhesion);
};
#endif
