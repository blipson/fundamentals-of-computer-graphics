// vertex shader template

#version 150
in vec4 vertex_position;
in vec4 vertex_color;
out vec4 vcolor;
uniform mat4 transformationMatrix;

void main()  {
	gl_Position = transformationMatrix*vertex_position; // update vertex position using transformationMatrix
	vcolor = vertex_color;  // pass vertex color to fragment shader
}