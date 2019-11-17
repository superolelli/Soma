#define dt 0.15


uniform vec2 u_resolution;
uniform sampler2D u_textureVelo;
uniform sampler2D u_textureColor;
uniform sampler2D u_baseTexture;
uniform float u_time;
uniform int u_frame;

uniform vec4 u_emitterColor[8];
uniform vec2 u_emitter[8];


vec4 calculateColor()
{
    vec2 uv = gl_FragCoord.xy / u_resolution.xy;
    vec2 w = 1.0/u_resolution.xy;
    
    vec2 velo = texture2D(u_textureVelo, uv).xy;
    vec4 col = texture2D(u_textureColor, uv - dt*velo*w*3.); //advection

    if (gl_FragCoord.y < 1. && gl_FragCoord.x < 1.)
        col = vec4(0);
    
    col += .0025/(0.0005+pow(length(uv - u_emitter[0]), 1.75))*dt*0.12*u_emitterColor[0];
    col += .0025/(0.0005+pow(length(uv - u_emitter[1]), 1.75))*dt*0.12*u_emitterColor[1];
    col += .0025/(0.0005+pow(length(uv - u_emitter[2]), 1.75))*dt*0.12*u_emitterColor[2];
    col += .0025/(0.0005+pow(length(uv - u_emitter[3]), 1.75))*dt*0.12*u_emitterColor[3];
    col += .0025/(0.0005+pow(length(uv - u_emitter[4]), 1.75))*dt*0.12*u_emitterColor[4];
    col += .0025/(0.0005+pow(length(uv - u_emitter[5]), 1.75))*dt*0.12*u_emitterColor[5];
    col += .0025/(0.0005+pow(length(uv - u_emitter[6]), 1.75))*dt*0.12*u_emitterColor[6];
    col += .0025/(0.0005+pow(length(uv - u_emitter[7]), 1.75))*dt*0.12*u_emitterColor[7];
    
    if (u_frame < 20)
    {
        col = vec4(0.);
    }

    col = clamp(col, 0.,5.);
    col = max(col - (0.0001 + col*0.01)*.5, 0.); //decay

    return col;   
}


void main()
{
    vec4 col = calculateColor();

    if (gl_FragCoord.y < 1. || gl_FragCoord.y >= (u_resolution.y-1.) )
        col = vec4(0);

    vec4 base = texture2D(u_baseTexture, gl_FragCoord/u_resolution.xy);

    gl_FragColor = vec4(vec3(col.xyz), base.w);
}