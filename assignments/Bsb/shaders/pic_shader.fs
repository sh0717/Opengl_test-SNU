#version 330 core
out vec4 FragColor;



uniform int Catch;

void main()
{   
    if(Catch==0){
        FragColor = vec4(1.0f);
    }
   else if(Catch==1){
        FragColor = vec4(0.0f,1.0f,0.0f,1.0f);
    }
    else{
        FragColor = vec4(1.0f,0.0f,0.0f,1.0f);
    }
}
