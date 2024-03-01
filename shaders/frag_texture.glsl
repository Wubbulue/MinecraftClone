#version 330 core
out vec4 FragColor;
  

in vec2 TexCoord;
flat in uint faceType;
flat in uint lightPacked;

uniform sampler2D textureUniform;
//uniform uint lightPacked;

vec2 shiftTexCoord(in uint index, in vec2 tex){
	int xIndex = int(mod(index,16));
	int yIndex = int(15u-index/16u);
	vec2 outTex = vec2(tex.x+0.0625*xIndex,tex.y+0.0625*yIndex);
	return outTex;
	
}

vec3 unpackLight(){
	uint mask = uint(0xFF);
	//uint redInt = (uint(lightPacked >> 24)&0xFF);
	uint redInt = (lightPacked >> 24)&mask;
	uint greenInt = (lightPacked >> 16)&mask;
	uint blueInt = (lightPacked >> 8)&mask;
	uint intensity = lightPacked&mask;
	
	float redFloat = (float(redInt))/15;
	float greenFloat = (float(greenInt))/15;
	float blueFloat = (float(blueInt))/15;
	float intensityFloat = (float(intensity))/15;
	
	vec3 lightColor = vec3(redFloat,greenFloat,blueFloat);
	lightColor = normalize(lightColor);
	lightColor*=intensityFloat;
	
	return lightColor;
	
}

void main()
{
	
	//float lightFloat = (float(lightLevel))/15;
	vec2 finalTex;
	//vec3 lightColor = vec3(0.2,0.1,0.0);
	vec3 lightColor = unpackLight();
	//lightColor = normalize(lightColor);
	//lightColor*=lightFloat;
	finalTex = shiftTexCoord(faceType,TexCoord);
	
	
	vec4 preDarken = texture(textureUniform, finalTex);
	FragColor = vec4(preDarken.rgb*lightColor,preDarken.a);

}
