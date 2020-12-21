#version 430 core
out vec4 FragColor;
uniform vec3 fillcolor;

void main()
{
    FragColor = vec4(fillcolor, 1.0f);
}