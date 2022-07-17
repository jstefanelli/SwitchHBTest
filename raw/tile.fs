#version 330

in vec2 vUv;

uniform vec4 uColor;
uniform int uTextureEnabled;
uniform sampler2D uTexture;

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

out vec4 color;

void main() {
    vec3 color_hsv = rgb2hsv(uColor.xyz);

    color = uColor;
    if (uTextureEnabled != 0) {
        vec4 tex_color = texture(uTexture, vUv);
        vec3 tex_hsv = rgb2hsv(tex_color.xyz);
        color = vec4(hsv2rgb(vec3(color_hsv.r, tex_hsv.g * color_hsv.g, tex_hsv.b * color_hsv.b)), uColor.a * tex_color.a);
    }
}