#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 texCoord;

uniform int object_ID_vert;
uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform mat4 Ts;
uniform mat4 Te;
uniform mat4 Tm;

out vec3 fragPos;
out vec3 fragColor;
out vec3 n;
out vec2 tc;
flat out int object_ID_frag;

vec3 lightPos = vec3(0,0,0);

void main() {

	// Earth ----------------------------------------------------------------------------------------------------------
	if (object_ID_vert == 0) {
		vec4 temp = vec4(Te * vec4(pos, 1.f));
		tc = texCoord;
		fragPos = pos;
		object_ID_frag = object_ID_vert;
		fragColor = color;
		n = normal;
		gl_Position = P * V * M * temp;

	// Night Sky ------------------------------------------------------------------------------------------------------
	} else if (object_ID_vert == 1) {
		tc = texCoord;
		fragPos = pos;
		object_ID_frag = object_ID_vert;
		fragColor = color;
		n = normal;
		gl_Position = P * V * M * vec4(pos, 1.0f);

	// Moon -----------------------------------------------------------------------------------------------------------
	} else if (object_ID_vert == 2) {
		vec4 temp = vec4(Tm * vec4(pos, 1.f));
		tc = texCoord;
		fragPos = pos;
		object_ID_frag = object_ID_vert;
		fragColor = color;
		n = normal;
		gl_Position = P * V * M * temp;
		
	// Sun ------------------------------------------------------------------------------------------------------------
	} else

	if (object_ID_vert == 3) {
		vec4 temp = vec4(Ts * vec4(pos, 1.f));
		tc = texCoord;
		fragPos = pos;
		object_ID_frag = object_ID_vert;
		fragColor = color;
		n = normal;
		gl_Position = P * V * M * temp;
	}
}
