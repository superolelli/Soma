#version 130

uniform sampler2D u_texture;
uniform vec2 u_textureSize;
uniform vec4 u_outlineColor;
uniform vec4 u_textureRect;

void main()
{
    vec2 coordinate = gl_TexCoord[0].xy;
    vec2 halfPixel = 0.5 / u_textureSize;
    vec2 minimum = u_textureRect.xy + halfPixel;
    vec2 maximum = u_textureRect.xy + u_textureRect.zw - halfPixel;
    if (coordinate.x < minimum.x || coordinate.y < minimum.y || coordinate.x > maximum.x || coordinate.y > maximum.y)
    {
        gl_FragColor = vec4(0.0);
        return;
    }
    float coverage = smoothstep(0.02, 0.65, texture2D(u_texture, coordinate).a);
    gl_FragColor = vec4(u_outlineColor.rgb, u_outlineColor.a * coverage);
}
