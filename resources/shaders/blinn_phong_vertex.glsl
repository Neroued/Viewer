#version 330 core

// 顶点输入 (与 VBO 绑定)
layout(location = 0) in vec3 aPosition; // 顶点位置
layout(location = 1) in vec3 aNormal;   // 顶点法线
layout(location = 2) in vec3 aColor;    // 顶点颜色

// 传递给片元着色器的插值变量
out vec3 vWorldPos;       // 世界空间位置
out vec3 vNormal;         // 法线
out vec3 vColor;          // 顶点颜色

// Uniform
uniform mat4 uModel;      // 模型矩阵
uniform mat4 uView;       // 视图矩阵
uniform mat4 uProjection; // 投影矩阵

void main()
{
    // 计算世界空间顶点位置
    vec4 worldPos = uModel * vec4(aPosition, 1.0);
    vWorldPos = worldPos.xyz;

    // 法线转换为世界空间 (假设 uModel 不含非uniform缩放)
    vNormal = mat3(transpose(inverse(uModel))) * aNormal;

    // 传递顶点颜色
    vColor = aColor;

    // 计算最终裁剪空间位置
    gl_Position = uProjection * uView * worldPos;
}
