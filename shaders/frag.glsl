#version 330 core
out vec4 FragColor;
  

in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float time;
uniform float mixAmount;

void main()
{
	float offset = sin(time);
	float otheroffset = cos(time);
	//FragColor = texture(texture2, vec2(TexCoord.x+offset,TexCoord.y+time)) * vec4(ourColor, 1.0);
    FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), mixAmount);
	
}