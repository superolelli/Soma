uniform sampler2D u_lastTexture;
uniform sampler2D u_baseTexture;
uniform vec2 u_resolution;
uniform float u_time;

uniform vec2 u_emitter;

void main()
{
    vec2 uv = gl_FragCoord.xy/u_resolution.xy;
    vec4 col = texture2D(u_lastTexture, uv);

    col *= 3.0 * (1.0 - length(uv-u_emitter) - u_time / 3.0f);

    col = clamp(col, 0.0, 1.0);

    vec4 base = texture2D(u_baseTexture, uv);
    
    gl_FragColor = vec4(col.xyz, base.w);
}