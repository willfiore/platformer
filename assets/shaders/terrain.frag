#version 330 core
layout (std140) uniform Matrices
{
  mat4 projection;
  mat4 view;
};
uniform float time;

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

highp float rand(vec2 co)
{
    highp float a = 12.9898;
    highp float b = 78.233;
    highp float c = 43758.5453;
    highp float dt= dot(co.xy ,vec2(a,b));
    highp float sn= mod(dt,3.14);
    return fract(sin(sn) * c);
}

void main() {
  vec3 lightColor = vec3(1.0, 1.0, 1.0) * 0.4;
  vec3 norm = normalize(Normal);
  vec3 lightDir = vec3(0.0, -1.0, 0.1);

  // Ambient
  float ambientStrength = 0.0;
  vec3 ambient = vec3(1.0, 1.0, 1.0);
  ambient *= ambientStrength;

  // Diffuse
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = diff * lightColor;

  // Specular
  float specularStrength = 0.1;
  vec3 viewPos = vec3(inverse(view)[3]);
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 reflectDir = reflect(lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
  vec3 specular = specularStrength * spec * lightColor;

  vec3 result = (ambient + diffuse + specular) * vec3(1.0, 1.0, 1.0);

  FragColor = vec4(result, 1.0);
}
