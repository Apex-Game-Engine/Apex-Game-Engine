#version 460
#include "common_include.glh"

layout (location = 0) out vec2 vUV;

void main()
{
    // Generate vertex positions based on gl_VertexIndex
    // This creates a large triangle covering the entire NDC space
    const vec2 positions[3] = vec2[](
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0)
    );

    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);

    // Generate UV coordinates
    // This maps the screen-space quad to [0,1] texture coordinates
    const vec2 uvs[3] = vec2[](
        vec2(0.0, 0.0),
        vec2(2.0, 0.0),
        vec2(0.0, 2.0)
    );
    
    vUV = uvs[gl_VertexIndex];
}
