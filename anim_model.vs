#version 330 core
layout(location=0) in vec3  aPos;
layout(location=1) in vec3  aNormal;
layout(location=2) in vec2  aTex;
layout(location=3) in vec3  aTan;
layout(location=4) in vec3  aBitan;
layout(location=5) in ivec4 aBoneIds;
layout(location=6) in vec4  aWeights;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

const int MAX_BONES = 200;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

out vec2 vTex;

void main(){
    vec4 skinned = vec4(0.0);
    for(int i=0;i<MAX_BONE_INFLUENCE;i++){
        int id = aBoneIds[i];
        if (id < 0) continue;
        if (id >= MAX_BONES){ skinned = vec4(aPos,1.0); break; }
        skinned += (finalBonesMatrices[id] * vec4(aPos,1.0)) * aWeights[i];
    }
    gl_Position = projection * view * model * skinned;
    vTex = aTex;
}
