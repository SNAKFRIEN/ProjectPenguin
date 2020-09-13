#version 330 core

const int MAX_JOINTS = 200;
const int MAX_WEIGHTS = 4;

layout (location = 0) in vec3 in_position;
//REMOVE in_normal and in_texcoord if possible, they're not used in this shader
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texcoord;
layout (location = 3) in ivec4 in_jointIndices;
layout (location = 4) in vec4 in_weights;

uniform mat4 modelTransform;
uniform mat4 jointTransforms[MAX_JOINTS];

void main()
{
	//Animation is applied by taking weighted average of joints that affect this vertex

	//Local position after animation has been applied
	vec4 totalLocalPos = vec4(0.0);
	
	//Loop through weights to apply animation
	for(int i = 0; i < MAX_WEIGHTS; i++)
	{
		vec4 localPosition = jointTransforms[in_jointIndices[i]] * vec4(in_position, 1.0);
		totalLocalPos += localPosition * in_weights[i];
	}
	gl_Position = modelTransform * totalLocalPos;
}