#version 400

out vec3 Color;
layout(location = 0) in vec3 Vertex_Pos;
layout(location = 1) in vec3 Vertex_Color;
uniform mat4 MVP;

void main()
{
    Color = Vertex_Pos;
    gl_Position = MVP * vec4(Vertex_Pos, 1.0);
}