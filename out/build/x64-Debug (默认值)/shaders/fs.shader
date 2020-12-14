# version 330 core

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;
out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform sampler2D texture_diffuse;
uniform vec3 specular;
uniform float shinness;

void main() {
    vec3 diffuse = vec3(texture(texture_diffuse, TexCoords));

    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    float ambientStrength = 0.3;
  //  vec3 ambientVec = ambientStrength * lightColor * diffuse ;
    vec3 ambientVec = lightColor * diffuse;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuseVec = diff * lightColor * diffuse;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shinness);
    vec3 specularVec = specular * spec * lightColor;

    vec3 result = ambientVec + diffuseVec + specularVec;

    FragColor = vec4(result, 1.0);

}