#version 330 core
out vec4 FragColor;
  

in vec2 TexCoord;
flat in uint faceType;

uniform sampler2D textureUniform;

vec2 shiftTexCoord(in uint index, in vec2 tex){
	int xIndex = int(mod(index,16));
	int yIndex = int(15u-index/16u);
	vec2 outTex = vec2(tex.x+0.0625*xIndex,tex.y+0.0625*yIndex);
	return outTex;
	
}

void main()
{
	vec2 finalTex;
	finalTex = shiftTexCoord(faceType,TexCoord);
	
	FragColor = texture(textureUniform, finalTex);

}