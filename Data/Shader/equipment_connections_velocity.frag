uniform sampler2D u_lastTexture;
uniform sampler2D u_baseTexture;
uniform vec2 u_resolution;
uniform float u_time;
uniform int u_frame;

uniform vec2 u_emitter[2];
uniform vec2 u_emitterDirection[2];

#define dt 0.15  //delta time (how fast moves the fluid)
//#define USE_VORTICITY_CONFINEMENT

//Recommended values between 0.03 and 0.2
//higher values simulate lower viscosity fluids (think billowing smoke)
#define VORTICITY_AMOUNT 0.11

float mag2(vec2 p){return dot(p,p);}


vec4 solveFluid(sampler2D smp, vec2 uv, vec2 w, float time)
{
	const float K = 0.2;
	const float v = 0.55;  //viscosity
    
    vec4 data = texture2D(smp, uv);
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
    
    //emitters
    vec2 newForce = vec2(0);
    newForce.xy += 0.75*u_emitterDirection[0]/(mag2(uv-u_emitter[0])+0.0001);
    newForce.xy += 0.75*u_emitterDirection[1]/(mag2(uv-u_emitter[1])+0.0001);

    
    data.xy += dt*(viscForce.xy - K/dt*densDif + newForce); //update velocity
    data.xy = max(vec2(0), abs(data.xy)-1e-4)*sign(data.xy); //linear velocity decay
    
   #ifdef USE_VORTICITY_CONFINEMENT
   	data.w = (tr.y - tl.y - tu.x + td.x);
    vec2 vort = vec2(abs(tu.w) - abs(td.w), abs(tl.w) - abs(tr.w));
    vort *= VORTICITY_AMOUNT/length(vort + 1e-9)*data.w;
    data.xy += vort;
    #endif
    
    data.y *= smoothstep(.5,.48,abs(uv.y-0.5)); //Boundaries
   // data.x *= smoothstep(.5,.48,abs(uv.x-0.5)); //Boundaries
    
    data = clamp(data, vec4(vec2(-10), 0.5 , -10.), vec4(vec2(10), 3.0 , 10.));
    
    return data;
}



void main()
{
    vec2 uv = gl_FragCoord.xy/u_resolution.xy;
    vec2 w = 1.0/u_resolution.xy;
    
    vec4 data = solveFluid(u_lastTexture, uv, w, u_time);
    
    if (u_frame < 20)
    {
        data = vec4(0.5,0,0,1.0);
    }

    gl_FragColor = data;

    vec4 base = texture2D(u_baseTexture, uv);
    
    if(base.w < 1.0)
        gl_FragColor = vec4(0.0,0.0,0.0,0.0);

}