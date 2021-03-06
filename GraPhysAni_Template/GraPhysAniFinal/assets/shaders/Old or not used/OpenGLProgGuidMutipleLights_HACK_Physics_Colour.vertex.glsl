// This shader is from the OpenGL Programming Guide, 8th edition, pg 377-378

#version 400

// in and out mean vertex buffer
layout(location=0) in vec4 in_Position;  
layout(location=1) in vec4 in_Color;	
layout(location=2) in vec4 in_Normal;	// We'll only use vec3 of the normal


uniform mat4 WorldMatrix;    		// aka ModelMatrix
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

// From the fragment shader
uniform float Shininess;
uniform float Strength;
uniform vec3 EyeDirection;

//***************************
// HACK for physics example
uniform vec4 ObjectColourHACK;
//***************************
		
out vec4 ex_Position;
out vec4 ex_Color;
out vec3 ex_Normal;			// Note Normal is a vec3

void main(void)
{	
	mat4 MVMatrix = ViewMatrix * WorldMatrix;
	mat4 MVPMatrix = ProjectionMatrix * MVMatrix;

	// gl_Position is a built-in variable
	gl_Position = MVPMatrix * in_Position;

	ex_Position = WorldMatrix * in_Position;	
			
	ex_Normal = vec3(in_Normal);

	//************************
	//*** HACK for physics ***
	//************************
	//ex_Color = in_Color;
	ex_Color = ObjectColourHACK;
}