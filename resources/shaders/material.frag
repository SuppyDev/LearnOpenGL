#version 460 core
out vec4 FragColor;

struct Material {
    vec3 color;
    float tintStrength;

    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light{
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform Material material;
uniform Light light;

uniform vec3 viewPos;

void main() {
    // ambient
    vec3 ambient = light.ambient * material.color * vec3(texture(material.diffuse, TexCoords));

    // diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);

    // mix diffuse texture colors with material
    vec3 texColor = vec3(texture(material.diffuse, TexCoords));
    vec3 finalColor = mix(texColor, texColor * material.color, material.tintStrength);
    vec3 diffuse = light.diffuse * diff * finalColor;

    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}