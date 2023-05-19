#version 120

// OF default
varying vec4 globalColor;

void main()
{
	gl_Position = ftransform();

	globalColor = gl_Color;
}