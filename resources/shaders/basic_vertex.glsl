#version 450 core

// 顶点输入
layout(location = 0) in vec3 aPosition;  // 位置   
layout(location = 1) in vec3 aNormal;   // 法向量
layout(location = 4) in vec3 aColor;    // 颜色

// Uniform
uniform mat4 uModel;      // 模型矩阵
uniform mat4 uView;       // 视图矩阵
uniform mat4 uProjection; // 投影矩阵

// 输出到片段着色器
out vec3 vColor;

void main()
{
    // 计算最终的裁剪空间位置
    gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);

    // 将颜色传递给片段着色器
    vColor = aColor;
}
