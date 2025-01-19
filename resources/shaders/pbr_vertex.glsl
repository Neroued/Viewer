#version 430 core

layout(location = 0) in vec3 aPos;       // 顶点位置
layout(location = 1) in vec3 aNormal;    // 法线
layout(location = 2) in vec3 aTangent;   // 切线
layout(location = 3) in vec3 aBitangent; // 副切线
layout(location = 5) in vec2 aTexCoords; // UV 坐标

// <------------------- uniform ------------------->
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

// <------------------- 输出给片段着色器 ------------------->
out vec3 vWorldPos;      // 世界坐标
out vec2 vTexCoords;     // 传递给片段的 UV
out mat3 vTBN;           // 传递给片段的 TBN 矩阵

void main()
{
    // 1. 顶点的世界坐标
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    vWorldPos = worldPos.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(uModel)));

    // 2. 法线、切线、副切线 先经过 normalMatrix(一般是 modelMatrix 的 3x3 逆转置)
    vec3 N = normalize(normalMatrix * aNormal);
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 B = normalize(normalMatrix * aBitangent);

    // (可选) 重新正交化，以避免 T、B 在数值计算中和 N 不完全正交导致的问题
    // T = normalize(T - dot(T, N)*N); 
    // B = cross(N, T);

    // 3. 构建 TBN 矩阵
    vTBN = mat3(T, B, N);

    // 4. 传递 UV
    vTexCoords = aTexCoords;

    // 5. 最终顶点位置
    gl_Position = uProjection * uView * worldPos;
}
