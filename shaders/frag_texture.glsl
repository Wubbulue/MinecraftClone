#version 330 core
out vec4 FragColor;
  

in vec2 TexCoord;
flat in uint faceType;
flat in uint lightLevel;

uniform sampler2D textureUniform;
uniform uint lightPacked;

vec2 shiftTexCoord(in uint index, in vec2 tex){
	int xIndex = int(mod(index,16));
	int yIndex = int(15u-index/16u);
	vec2 outTex = vec2(tex.x+0.0625*xIndex,tex.y+0.0625*yIndex);
	return outTex;
	
}

void main()
{
	
	float lightFloat = (float(lightLevel))/15;
	vec2 finalTex;
	vec3 lightColor = vec3(0.2,0.1,0.0);
	lightColor = normalize(lightColor);
	lightColor*=lightFloat;
	finalTex = shiftTexCoord(faceType,TexCoord);
	
	
	vec4 preDarken = texture(textureUniform, finalTex);
	FragColor = vec4(preDarken.rgb*lightColor,preDarken.a);

}
