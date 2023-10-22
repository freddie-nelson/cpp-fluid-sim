#version 300 es

in vec2 a_Vertex;

uniform mediump float u_Radius;

out mediump float sqrRadius;

void main() {
    sqrRadius = u_Radius * u_Radius;

    float x = float(((uint(gl_VertexID) + 2u) / 3u) % 2u); 
    float y = float(((uint(gl_VertexID) + 1u) / 3u) % 2u); 

    gl_Position = vec4(-1.0f + x * 2.0f, -1.0f + y * 2.0f, 0.0f, 1.0f);
}