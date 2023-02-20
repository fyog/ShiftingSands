#version 330 core

in vec3 fragPos;
in vec3 fragColor;
in vec3 n;
in vec2 tc;
flat in int object_ID_frag;

uniform vec3 ePos;
uniform vec3 light_diff;
uniform vec3 light_spec;
uniform sampler2D sampler;

out vec4 color;

vec3 lightPos = vec3(0,0,0);


void main() {

	// Sun
	if (object_ID_frag == 3) {

		vec3 tex = texture(sampler, tc).xyz;
		vec3 lightDir = normalize(fragPos - light_diff);
		vec3 normal = normalize(n);
		float diff = max(dot(lightDir, normal), 0.0);
		color = vec4(diff * tex, 1.0);

	// Moon
	} else if (object_ID_frag == 2) {

		// Texture
		vec3 tex = texture(sampler, tc).xyz;

		// Ambient light
		vec3 ambient = 0.01f * tex;
		color = vec4(ambient, 1.f);

	// Night sky
	} else if (object_ID_frag == 1) {

		// Texture
		vec3 tex = texture(sampler, tc).xyz;

		// Ambient light
		vec3 ambient = 0.05f * tex;
		color = vec4(ambient, 1.f);

	// Earth
	} else {

		vec3 tex = texture(sampler, tc).xyz;

		// Ambient light
		vec3 ambient = 0.01f * tex;
		
		/*
		// Specular light
		
		vec3 viewDir = normalize(viewPos - fragPos);

		float spec = pow(max(dot(normal, ePos), 0.0), 32.0f);
	    vec3 specular = vec3(0.3f) * spec * tex;
		
		vec4 color_temp = vec4(pow(ambient, vec3(1.0f/2.2f)), 1.0f);
		*/
		color = vec4(ambient, 1.f);
		

	}
	
}
