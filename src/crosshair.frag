#version 330 core
uniform sampler2D background;
uniform vec2 screenSize;

void main()
{
    vec2 uv = gl_FragCoord.xy / screenSize;
    vec4 bgColor = texture(background, uv);
    gl_FragColor = vec4(vec3(1.0) - bgColor.rgb, 0.75);
}
