#version 330

uniform sampler2D u_texture;

uniform float u_kernelSize;

uniform float u_horizontalPass;

uniform vec2 u_ImageSize;

uniform vec4 u_blurringRect;

#define PI 3.1415926535897932384626433832795

void main(void)
{
    float offset = 2.0;
    vec4 blurringRect = vec4(u_blurringRect.xy / u_ImageSize, u_blurringRect.zw / u_ImageSize);
    if(gl_TexCoord[0].x > blurringRect.x && gl_TexCoord[0].x < blurringRect.x + blurringRect.z && gl_TexCoord[0].y > blurringRect.y && gl_TexCoord[0].y < blurringRect.y + blurringRect.w)
    {
        vec2 direction = vec2(offset * u_horizontalPass, offset * (1.0 - u_horizontalPass));
        float sigma = ceil((u_kernelSize + 2.0) / 3.0);
        vec3 incrementalGaussian;
        incrementalGaussian.x = 1.0 / (sqrt(2.0 * PI) * sigma);
        incrementalGaussian.y = exp(-0.5 / (sigma * sigma));
        incrementalGaussian.z = incrementalGaussian.y * incrementalGaussian.y;
        float numPixelsPerSide = float(u_kernelSize + 2.0);
        vec4 avgValue = texture(u_texture, gl_TexCoord[0].xy) * incrementalGaussian.x;
        float coefficientSum = incrementalGaussian.x;
        incrementalGaussian.xy *= incrementalGaussian.yz;
        for(float i = 1.0; i <= numPixelsPerSide; i++) {
            avgValue += texture(u_texture, gl_TexCoord[0].xy - (direction * i) / u_ImageSize) * incrementalGaussian.x;
            avgValue += texture(u_texture, gl_TexCoord[0].xy + (direction * i) / u_ImageSize) * incrementalGaussian.x;
            coefficientSum += 2.0 * incrementalGaussian.x;
            incrementalGaussian.xy *= incrementalGaussian.yz;
        }
        gl_FragColor = avgValue / coefficientSum;
    }
    else
        gl_FragColor = texture(u_texture, gl_TexCoord[0].xy);
}