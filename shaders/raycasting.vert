#version 400

layout (location = 0) in vec3 vertex_pos;
layout (location = 1) in vec3 vertex_color;

out vec3 EntryPoint;
out vec4 ExitPointCoord;

uniform mat4 MVP;

void main()
{	
	EntryPoint = vertex_pos; // 진입 지점은 vertex_pos와 같다.
	gl_Position = MVP * vec4(vertex_pos,1.0);
	ExitPointCoord = gl_Position;
}

