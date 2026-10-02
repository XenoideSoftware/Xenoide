#version 330 core

uniform mat4 modelViewProj;
uniform mat4 model;

uniform float seconds;

in vec3 vertCoord;
in vec3 vertNormal;
in vec2 vertTexCoord;

out vec4 fragColor;
out vec2 fragTexCoord;

float fmod(float x, float y) {
    return x - y * floor(x / y);
}

float wave(float seconds) {
    return (cos(seconds) + 1.0) / 2.0 * 0.5 + 0.5;
}

vec3 computeLightingColour() {
    return vec3(wave(seconds), wave(seconds + 0.5), wave(seconds - 0.5));
}

void main() {
    vec3 lightDirection[4];
    lightDirection[0] = normalize(vec3(0.5, 0.3, 0.4));
    lightDirection[1] = normalize(vec3(-0.5, 0.3, -0.4));
    lightDirection[2] = normalize(vec3(0.5, -0.3, 0.4));
    lightDirection[3] = normalize(vec3(-0.5, -0.3, -0.4));

    gl_Position = vec4(vertCoord, 1.0) * modelViewProj;

    vec4 color = vec4(0.0);
    for (int i = 0; i < 4; i++) {
        color += vec4(computeLightingColour() * max(dot(lightDirection[i], vertNormal), 0.0), 1.0);
    }
    fragColor = color;

    fragTexCoord = vertTexCoord;
}