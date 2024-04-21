#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float time;
uniform vec2 resolution;
uniform vec3 cameraFront; // Camera's front direction
uniform bool boolean;
uniform bool inside;

void main() {
    vec2 q = gl_FragCoord.xy / resolution.xy;
    vec2 p = -1.0 + 2.0 * q;
    p.x *= resolution.x / resolution.y;

    // Generate a noise value based on the fragment position and time
    float noise = fract(sin(dot(q, vec2(0.9898, 7800.233))) * 43758.5453 + time);

    // Adjust the rain direction based on the camera front
    float rainAngle = atan(cameraFront.z, cameraFront.x);

    // Incorporate randomness and camera influence into the st coordinates
    vec2 randomOffset = vec2(noise * 0.002 - 0.01, noise * 0.005) * abs(cos(time));
    vec2 st = p * vec2(1.5, 0.25) + vec2((time * 0.1 - q.y * 0.2) + rainAngle, time * 0.3) + randomOffset;

    float f = texture(screenTexture, st).y * texture(screenTexture, st).x * 1.55;
    f = clamp(pow(abs(f), 7.5) * 1.0, 0.0, 1.0) * 10;

    vec3 col = vec3(f);

    // Base color of the scene
    vec3 baseColor = vec3(0.1, 0.1, 0.1);

    vec3 lightningColor= vec3(0.0);
    if (boolean) {
        lightningColor = vec3(1.0, 1.0, 0.5);
    }


    float rgb = 0.1;

    if (inside) {
        rgb = 0.0;
    }

    vec3 finalColor = mix(baseColor, col, 0.7) + lightningColor;

    FragColor = vec4(finalColor, rgb);
}
