#version 330

in vec2 vUv;

uniform vec4 uColor;

out vec4 color;

void main() {
    color = uColor;
}