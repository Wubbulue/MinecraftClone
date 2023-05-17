#version 330 core
out vec4 FragColor;
  

in vec2 TexCoord;
flat in uint faceType;
flat in uint lightLevel;

uniform sampler2D textureUniform;

vec2 shiftTexCoord(in uint index, in vec2 tex){
	int xIndex = int(mod(index,16));
	int yIndex = int(15u-index/16u);
	vec2 outTex = vec2(tex.x+0.0625*xIndex,tex.y+0.0625*yIndex);
	return outTex;
	
}

void main()
{
	uint otherLightLevel = uint(10);
	vec2 finalTex;
	finalTex = shiftTexCoord(faceType,TexCoord);
	float lightFloat = (float(lightLevel))/15;
	
	vec4 preDarken = texture(textureUniform, finalTex);
	FragColor = vec4(preDarken.r*lightFloat,preDarken.g*lightFloat,preDarken.b*lightFloat,preDarken.a);

}
