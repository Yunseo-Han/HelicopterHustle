#version 120

varying vec4 globalColor;

// attribute float life;

void main ()
{
    // vec4 alphaMask = vec4(0, 0, 0, life);
    // vec4 color = globalColor - alphaMask;
    gl_FragColor = globalColor;
}
