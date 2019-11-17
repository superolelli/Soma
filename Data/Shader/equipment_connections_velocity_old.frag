uniform sampler2D u_lastTexture;
uniform sampler2D u_baseTexture;
uniform vec2 u_resolution;
uniform float u_time;
uniform int u_frame;
uniform vec4 u_mouse;

uniform vec2 u_emitter[8];

#define dt 0.1


float mag2(vec2 p){return dot(p,p);}
float length2(vec2 p){return dot(p,p);}
mat2 mm2(in float a){float c = cos(a), s = sin(a);return mat2(c,s,-s,c);}


vec4 solveFluid(sampler2D smp, vec2 uv, vec2 w, float time, vec3 mouse, vec3 lastMouse)
{
	const float K = 0.2;
	const float v = 0.55;
    
    vec4 data = texture2D(smp, uv);

    vec4 base = texture2D(u_baseTexture, uv);
    if(base.w != 1.0)
    {
        return data;
    }

    vec4 tr = texture2D(smp, uv + vec2(w.x , 0));
    vec4 tl = texture2D(smp, uv - vec2(w.x , 0));
    vec4 tu = texture2D(smp, uv + vec2(0 , w.y));
    vec4 td = texture2D(smp, uv - vec2(0 , w.y));
    
    vec3 dx = (tr.xyz - tl.xyz)*0.5;
    vec3 dy = (tu.xyz - td.xyz)*0.5;
    vec2 densDif = vec2(dx.z ,dy.z);
    
    data.z -= dt*dot(vec3(densDif, dx.x + dy.y) ,data.xyz); //density
    vec2 laplacian = tu.xy + td.xy + tr.xy + tl.xy - 4.0*data.xy;
    vec2 viscForce = vec2(v)*laplacian;
    data.xyw = texture2D(smp, uv - dt*data.xy*w).xyw; //advection
    
    vec2 newForce = vec2(0);

    newForce.xy += 3.9*vec2(.0003, 0.005)/(mag2(uv-u_emitter[5])+0.001);
    newForce.xy += 3.9*vec2(-.0003, 0.005)/(mag2(uv-u_emitter[4])+0.001);
    newForce.xy += 3.9*vec2(.0003, -0.005)/(mag2(uv-u_emitter[0])+0.001);
    newForce.xy += 3.9*vec2(-.0003, -0.005)/(mag2(uv-u_emitter[1])+0.001);

    newForce.xy += 2.9*vec2(0.005, -.0003)/(mag2(uv-u_emitter[7])+0.151);
    newForce.xy += 2.9*vec2(0.005, .0003)/(mag2(uv-u_emitter[6])+0.151);
    newForce.xy += 2.9*vec2(-0.005, -.0003)/(mag2(uv-u_emitter[2])+0.0801);
    newForce.xy += 2.9*vec2(-0.005, .0003)/(mag2(uv-u_emitter[3])+0.0801);

    
    data.xy += dt*(viscForce.xy - K/dt*densDif + newForce); //update velocity
    data.xy = max(vec2(0), abs(data.xy)-1e-4)*sign(data.xy); //linear velocity decay
    
    data.y *= smoothstep(.5,.48,abs(uv.y-0.5)); //Boundaries
    
    data = clamp(data, vec4(vec2(-10), 0.5 , -10.), vec4(vec2(10), 3.0 , 10.));
    
    return data;
}



void main()
{
    vec2 uv = gl_FragCoord.xy/u_resolution.xy;
    vec2 w = 1.0/u_resolution.xy;
    
    vec4 lastMouse = texelFetch(u_lastTexture, ivec2(0,0), 0);
    vec4 data = solveFluid(u_lastTexture, uv, w, u_time, u_mouse.xyz, lastMouse.xyz);
    
    if (u_frame < 20)
    {
        data = vec4(0.5, 0.0, 0.0, 1.0);
    }
    
    gl_FragColor = data;
}