#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferObject {
	mat4 mvpMat;
    mat4 mMat;
    mat4 nMat;
} ubo;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 fragUV;


void main() {
	gl_Position = ubo.mvpMat * vec4(inPosition, 1.0);
	fragUV = inUV;
}
