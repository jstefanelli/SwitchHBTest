#version 330

in vec2 vUv;

uniform vec4 uColor;
uniform int uTextureEnabled;
uniform sampler2D uTexture;

out vec4 color;

void main() {
    color = uColor;
    if (uTextureEnabled != 0) {
        color *= texture(uTexture, vUv);
    }
}