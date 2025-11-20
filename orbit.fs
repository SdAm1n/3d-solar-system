#version 330 core
out vec4 FragColor;

uniform vec3 orbitColor;

void main()
{
    // Increased alpha from 0.4 to 0.7 for more visible orbit lines
    FragColor = vec4(orbitColor, 0.7);
}
