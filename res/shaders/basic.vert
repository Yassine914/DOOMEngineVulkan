#version 450

// clang-format off

vec2 pos[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

vec3 cols[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);


// layout(push_constant) uniform constants{
//     mat4 model;
// } ObjectData;

layout(location = 0) out vec3 fragCol;

void main()
{
    gl_Position = /* ObjectData.model * */ vec4(pos[gl_VertexIndex], 0.0, 1.0);
    fragCol = cols[gl_VertexIndex];
}
// clang-format on