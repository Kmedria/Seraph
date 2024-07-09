#shader vertex
#version 330 core

layout (location = 0) in
vec3 input;

uniform mat4 project_trans_matrix;

void main() {
    float x = 0, y = 0, z = 0, w = 0;
    x = input.x * project_trans_matrix[0][0] + input.y * project_trans_matrix[1][0] + input.z * project_trans_matrix[2][0] + project_trans_matrix[3][0];
    y = input.x * project_trans_matrix[0][1] + input.y * project_trans_matrix[1][1] + input.z * project_trans_matrix[2][1] + project_trans_matrix[3][1];
    z = input.x * project_trans_matrix[0][2] + input.y * project_trans_matrix[1][2] + input.z * project_trans_matrix[2][2] + project_trans_matrix[3][2];
    w = input.x * project_trans_matrix[0][3] + input.y * project_trans_matrix[1][3] + input.z * project_trans_matrix[2][3] + project_trans_matrix[3][3];
    if (w != 0)
    {
        x /= w;
        y /= w;
        z /= w;
    }
    gl_Position = vec4(x, y, z, 1.0);
}

#shader fragment
#version 330 core

out vec4 FragColor;

uniform vec4 u_colour;

void main() { 
    FragColor = u_colour;
};