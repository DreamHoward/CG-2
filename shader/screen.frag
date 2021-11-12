#version 330 core
uniform sampler2D shadowMap;
uniform sampler2D noiseTextureID;
uniform float dissolveFactor;   
uniform float ky;
in  vec2 TexCoord;



void main()
{           
    if(ky == 0.0){
	   if(texture(noiseTextureID, TexCoord).r < dissolveFactor)
			discard;
	}
}

