#include "SandCell.h"
#include "Texture.h"

SandCell::SandCell(int height, float adhesion) {
	SandCell::height = height;
	SandCell::adhesion = adhesion;
	//SandCell::texture = texture;
}

// getters and setters
float SandCell::getHeight() {
	return SandCell::height;
}

float SandCell::getAdhesion() {
	return SandCell::adhesion;
}

void SandCell::setHeight(float height) {
	SandCell::height = height;
}

void SandCell::setAdhesion(float adhesion) {
	SandCell::adhesion = adhesion;
}



