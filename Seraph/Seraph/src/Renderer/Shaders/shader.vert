#version 450


layout(binding = 0) uniform UniformBufferObject {
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 fragColor;

void main() {
    gl_Position = vec4(inPosition, 1.0f) * ubo.proj;

    if (gl_Position[3] != 0) {
        gl_Position[0] = gl_Position[0] / gl_Position[3];
        gl_Position[1] = gl_Position[1] / gl_Position[3];
        gl_Position[2] = gl_Position[2] / gl_Position[3];
        gl_Position[3] = gl_Position[3] / gl_Position[3];
    }

    float temp = (gl_Position[0]*0 + gl_Position[1]*0 + gl_Position[2]*1) / (gl_Position[0]*gl_Position[0] + gl_Position[1]*gl_Position[1] + gl_Position[2]*gl_Position[2]);

    fragColor = inColor;
}