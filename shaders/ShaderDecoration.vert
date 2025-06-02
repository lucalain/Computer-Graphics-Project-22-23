#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferObject {
	mat4 mvpMat[5];
    mat4 mMat;
    mat4 nMat;
} ubo;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0)out vec3 fragPos;
layout(location = 1) out vec3 fragNorm;
layout(location = 2) out vec2 fragUV;


void main() {
	gl_Position = ubo.mvpMat[gl_InstanceIndex] * vec4(inPosition, 1.0);
	fragUV = inUV;
    fragNorm = (ubo.nMat*vec4(inNormal,0.0)).xyz;;
    fragPos = (ubo.mMat*vec4(inPosition,1.0)).xyz;
}
