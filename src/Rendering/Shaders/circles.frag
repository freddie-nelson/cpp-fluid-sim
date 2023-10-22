#version 300 es

precision mediump float;

uniform vec2 u_Circles[500];
uniform vec4 u_Colors[500];
uniform int u_NumCircles;

uniform vec2 u_Resolution;
uniform float u_Radius;

in float sqrRadius;

out vec4 outColor;

void main() {
	for(int i = 0; i < u_NumCircles; i++) {
		vec2 pos = u_Circles[i];

        float xDiff = gl_FragCoord.x - pos.x;
        float yDiff = u_Resolution.y - gl_FragCoord.y - pos.y;

        // calculate sqr distance to circle
        float sqrDist = xDiff * xDiff + yDiff * yDiff;
        if (sqrDist <= sqrRadius) {
            outColor = u_Colors[i];
            break;
        }
	} 

}