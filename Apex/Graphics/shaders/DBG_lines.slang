// debug_lines.slang

struct Camera {
    float4x4 view;
    float4x4 projection;
    float4x4 inverseView;
};



[shader("vertex")]
float4 vsMain(uint vertexId: SV_VertexID) : SV_Position {
    return float4(0, 0, 0, 0);
} 
