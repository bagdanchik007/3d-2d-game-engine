#version 450 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;
uniform vec3 u_NormalMatrixCol0;
uniform vec3 u_NormalMatrixCol1;
uniform vec3 u_NormalMatrixCol2;

out vec3 v_WorldPosition;
out vec3 v_WorldNormal;

void main()
{
    vec4 worldPosition = u_Model * vec4(a_Position, 1.0);
    v_WorldPosition = worldPosition.xyz;

    mat3 normalMatrix = mat3(u_NormalMatrixCol0, u_NormalMatrixCol1, u_NormalMatrixCol2);
    v_WorldNormal = normalize(normalMatrix * a_Normal);

    gl_Position = u_ViewProjection * worldPosition;
}
