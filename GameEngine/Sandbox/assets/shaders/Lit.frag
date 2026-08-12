#version 450 core
in vec3 v_WorldPosition;
in vec3 v_WorldNormal;

uniform vec3 u_ViewPosition;
uniform vec3 u_LightPosition;
uniform vec3 u_LightColor;
uniform vec3 u_ObjectColor;

out vec4 o_Color;

void main()
{
    vec3 normal = normalize(v_WorldNormal);

    vec3 ambient = 0.1 * u_LightColor;

    vec3 lightDir = normalize(u_LightPosition - v_WorldPosition);
    float diffuseStrength = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diffuseStrength * u_LightColor;

    vec3 viewDir = normalize(u_ViewPosition - v_WorldPosition);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float specularStrength = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = 0.5 * specularStrength * u_LightColor;

    vec3 result = (ambient + diffuse + specular) * u_ObjectColor;
    o_Color = vec4(result, 1.0);
}
