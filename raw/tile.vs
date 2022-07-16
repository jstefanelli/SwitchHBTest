#version 330

in vec3 aPos;
in vec2 aUv;

uniform mat4 uMvp;

out vec2 vUv;

void main() {
    gl_Position = uMvp * vec4(aPos, 1.0);
    vUv = aUv;
}