uniform vec2 u_resolution;
uniform sampler2D u_baseTexture;
uniform float u_progress;

uniform vec4 u_emitterColor;
uniform vec2 u_emitter[2];

float quarticOut(float t) {
  return pow(t - 1.0, 3.0) * (1.0 - t) + 1.0;
}

float manhattanDistance(vec2 p1, vec2 p2) 
{
	float d1 = abs(p1.x - p2.x);
	float d2 = abs(p1.y - p2.y);
	return d1 + d2;
}

const float MAX_DISTANCE = 1.5; // manhatten distance of one emitter and the end point

void main( )
{
    vec2 uv = gl_FragCoord.xy / u_resolution.xy;

    float relativeDistance = min(manhattanDistance(uv, u_emitter[0]), manhattanDistance(uv, u_emitter[1])) / MAX_DISTANCE;
    float progress = quarticOut(u_progress);
    vec3 color = u_emitterColor.rgb * step(relativeDistance, progress);

    vec4 textureColor = texture2D(u_baseTexture, uv);
    gl_FragColor = vec4(color, textureColor.w);
}