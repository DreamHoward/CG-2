#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;


//out vec3 FragPos;
//out vec3 Normal;
//out vec2 TexCoord;
//out vec4 FragPosLightSpace;

out vec2 Texcoord;
uniform mat4 Model;
uniform mat4 lightProjection;
uniform mat4 lightView;
uniform mat4 view;
uniform mat4 projection;
void main()
{
	//mat4 lightSpaceMatrix = lightProjection * lightView;
    gl_Position = lightProjection * lightView * Model * vec4(position, 1.0f);
    //FragPos = vec3(Model * vec4(position, 1.0));
    //Normal = mat3(view*Model) * normal;      //transpose(inverse(mat3(Model))) * normal;
    
	Texcoord = texcoord;
    //FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
}