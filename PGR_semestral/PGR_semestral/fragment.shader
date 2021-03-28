#version 330 core

in vec3 fragNormal;
in vec3 fragPos;

uniform vec3 lightPos;

out vec4 color;

void main()
{
	vec3 norm = normalize(fragNormal);
	vec3 lightDir = normalize(lightPos - fragPos);

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 ambient = vec3(0.2, 0.2, 0.2);
	vec3 diffuse = diff * vec3(1.0, 1.0, 1.0f);
	vec3 result = (ambient + diffuse) * vec3(0.5, 0.5, 0.5);
	color = vec4(result, 1.0);
}
