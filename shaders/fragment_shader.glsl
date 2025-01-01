#version 450 core

in vec3 vColor; // 从顶点着色器传递的颜色
out vec4 FragColorOutput;

uniform int drawmode; // 0: 线框模式，1: 填充模式

void main()
{
    if (drawmode == 0)
    {
        // 线框模式，绘制黑色
        FragColorOutput = vec4(0.0, 0.0, 0.0, 1.0);
    }
    else if (drawmode == 1)
    {
        // 填充模式，使用顶点颜色
        FragColorOutput = vec4(vColor, 1.0);
    }
}
