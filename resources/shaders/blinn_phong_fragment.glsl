#version 330 core

// 顶点着色器传入的插值变量
in vec3 vWorldPos;  // 世界空间位置
in vec3 vNormal;    // 法线
in vec3 vColor;     // 顶点颜色

// 光照相关 Uniform
uniform vec3 uViewPos;        // 摄像机位置 (世界空间)
uniform vec3 uLightPos;       // 光源位置 (世界空间)
uniform vec3 uLightColor;     // 光源颜色
uniform float uAmbientStrength;  // 环境光强度
uniform float uSpecularStrength; // 镜面反射强度
uniform float uShininess;        // 高光反射指数

// 输出颜色
out vec4 fragColor;

void main()
{
    // 1. 环境光 (ambient)
    vec3 ambient = uAmbientStrength * uLightColor * vColor;

    // 2. 漫反射 (diffuse)
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightPos - vWorldPos);
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * uLightColor * vColor;

    // 3. 镜面反射 (specular, Blinn-Phong)
    vec3 V = normalize(uViewPos - vWorldPos);
    vec3 H = normalize(L + V); // 半程向量
    float specAngle = max(dot(N, H), 0.0);
    float specFactor = pow(specAngle, uShininess);
    vec3 specular = uSpecularStrength * specFactor * uLightColor;

    // 4. 叠加光照
    vec3 finalColor = ambient + diffuse + specular;

    // 5. 输出最终颜色
    fragColor = vec4(finalColor, 1.0);
}
