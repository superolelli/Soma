#define dt 0.15


uniform vec2 u_resolution;
uniform sampler2D u_textureVelo;
uniform sampler2D u_textureColor;
uniform sampler2D u_baseTexture;
uniform float u_time;
uniform int u_frame;

uniform vec4 u_emitterColor;
uniform vec2 u_emitter[2];



mat2 mm2(in float a){float c = cos(a), s = sin(a);return mat2(c,s,-s,c);}


vec4 calculateColor()
{
    vec2 uv = gl_FragCoord.xy / u_resolution.xy;
    vec2 w = 1.0/u_resolution.xy;
    
    vec2 velo = texture2D(u_textureVelo, uv).xy; //get velocity
    vec4 col = texture2D(u_textureColor, uv - dt*velo*w*3.); //advection

    if (gl_FragCoord.y < 1. && gl_FragCoord.x < 1.)
        col = vec4(0);
    
    
    col += .0025/(0.0005+pow(length(uv - u_emitter[0]),1.75))*dt*0.12*u_emitterColor;
    col += .0025/(0.0005+pow(length(uv - u_emitter[1]),1.75))*dt*0.12*u_emitterColor;
    
    
    if (u_frame < 20)
    {
        col = vec4(0.);
    }
    
    col = clamp(col, 0.,5.);
    col = max(col - (0.0001 + col*0.004)*.5, 0.); //decay

    return col;
}


void main( )
{
    vec2 uv = gl_FragCoord.xy / u_resolution.xy;

    vec4 col = calculateColor();

  //  if (gl_FragCoord.y < 1. || gl_FragCoord.y >= (u_resolution.y-1.))
  //      col = vec4(0);

  //  if (gl_FragCoord.x < 1. || gl_FragCoord.x >= (u_resolution.x-1.))
   //     col = vec4(0);    

    vec4 base = texture2D(u_baseTexture, uv);
    gl_FragColor = vec4(col.xyz, base.w);
}