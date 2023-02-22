#version 330 core
// TODO: define in/out and uniform variables.
out vec4 FragColor;
in vec2 TexCoord;


uniform sampler2D texture0;
uniform sampler2D texture1;

void main()
{
      vec4 Color=texture(texture0,TexCoord);
      if(Color.a<0.5)
        discard;

      FragColor=Color;
        


    // fill in
    // Hint) you can ignore transparent texture pixel by 
    // if(color.a < 0.5){discard;}

}