#version 330 core
uniform vec2 resolution;
uniform float zoom;
uniform vec2 pan;

uniform vec2 origin;
uniform vec2 delta;


void main()
{

    vec2 uv = gl_FragCoord.xy / resolution; // Normalized coordinates (0.0 to 1.0)

    /*
    float aspectRatio = resolution.x / resolution.y;
    vec2 scale = vec2(
    (uv.x - 0.5) * 3.5 * zoom * aspectRatio/2.0 + pan.x,
    (uv.y - 0.5) * 2.0 * zoom + pan.y
    );
*/

    vec2 pixel = gl_FragCoord.xy;
    //vec2 scale = (pixel * origin) + delta;

    vec2 scale = pixel * delta + origin;

    vec3 colorA = vec3(0.15,0.15,0.60);
    vec3 colorB = vec3(0.75,0.25,0.25);

    float x = 0.0;
    float y = 0.0;

    float breakValue = 0.0;
    float xtemp = 0.0;
    float iteration = 0.0;
    float MAX_ITERATIONS = 200.0;

    for(iteration = 0.0; iteration<MAX_ITERATIONS; iteration++){

        xtemp = x*x-y*y + scale.x;
        y = 2.0*x*y + scale.y;
        x = xtemp;
        breakValue = (x*x) + (y*y);

        if(breakValue >= 4.0){
            break;
        }
    }

    float normalizedIteration = 4.0*iteration/(MAX_ITERATIONS);

    float smoothed = smoothstep(0.0, 1.0, normalizedIteration);

    vec3 finalColor = mix(colorA, colorB, smoothed);

    if(iteration >=MAX_ITERATIONS){
        finalColor = vec3(0,0,0);
    }

    // Output the final color
    gl_FragColor = vec4(finalColor, 1.0);
}
