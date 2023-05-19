#version 120

// OF default
varying vec4 globalColor;

// per particle, passed in during ofApp::draw() method
attribute float life;

void main()
{
	vec4 position = ftransform();
    
    float displacementY = life * -1;
	
    vec4 modifiedPosition = position;
	modifiedPosition.y += displacementY;
	
	gl_Position = modifiedPosition;
    
    // in OpenGL 2.0 we must get the global color using the gl_Color command,
    // and store it in a globalColor (varying) that is passed to our frag shader.
    // please note that the frag shader also has a globalColor (varying),
    // and this is the standard way of passing data from the vertex shader to the frag shader.
    globalColor = gl_Color;
}