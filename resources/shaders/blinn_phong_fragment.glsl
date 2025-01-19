#version 330 core

// 顶点着色器传入的插值变量
in vec3 vWorldPos;  // 世界空间位置
in vec3 vNormal;    // 法线
in vec3 vVertexColor;     // 顶点颜色
in vec2 vTexCoord;  // 纹理坐标

// 输出颜色
out vec4 fragColor;

struct MaterialParameter 
{
    vec4 value;    // 标量或颜色值
    bool useMap;   // 是否使用贴图
    sampler2D map; // 贴图
};

struct Material 
{
    MaterialParameter baseColor;
    MaterialParameter normal;
    MaterialParameter ao;
    MaterialParameter emissive;
};

// 材质相关 Uniform
uniform Material uMaterial;
uniform bool uUseMaterial;

// 光照相关 Uniform
uniform vec3 uViewPos;        // 摄像机位置 (世界空间)
uniform vec3 uLightPos;       // 光源位置 (世界空间)
uniform vec3 uLightColor;     // 光源颜色
uniform float uAmbientStrength;  // 环境光强度
uniform float uSpecularStrength; // 镜面反射强度
uniform float uShininess;        // 高光反射指数


void main()
{
    vec3 baseColor;
    // 使用顶点颜色或材质的 baseColor
    if (!uUseMaterial)
    {
        baseColor = vVertexColor; // 使用顶点颜色
    }
    else
    {
        baseColor = uMaterial.baseColor.useMap 
                    ? texture(uMaterial.baseColor.map, vTexCoord).rgb // 使用贴图
                    : uMaterial.baseColor.value.rgb;                  // 使用材质颜色
    }

    // 法线计算
    vec3 N = normalize(vNormal);
    if (uUseMaterial && uMaterial.normal.useMap) 
    {
        vec3 normalFromMap = texture(uMaterial.normal.map, vTexCoord).rgb * 2.0 - 1.0;
        N = normalize(normalFromMap); // 使用法线贴图修正法线
    }

    // 1. 环境光 (ambient)
    vec3 ambient = uAmbientStrength * uLightColor * baseColor;

    // 2. 漫反射 (diffuse)
    vec3 L = normalize(uLightPos - vWorldPos);
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * uLightColor * baseColor;

    // 3. 镜面反射 (specular, Blinn-Phong)
    vec3 V = normalize(uViewPos - vWorldPos);
    vec3 H = normalize(L + V); // 半程向量
    float specAngle = max(dot(N, H), 0.0);
    float specFactor = pow(specAngle, uShininess);
    vec3 specular = uSpecularStrength * specFactor * uLightColor;

    // 4. 自发光 (emissive)
    vec3 emissive = vec3(0.0); // 默认无自发光
    if (uUseMaterial && uMaterial.emissive.useMap) 
    {
        emissive = texture(uMaterial.emissive.map, vTexCoord).rgb;
    } 
    else if (uUseMaterial) 
    {
        emissive = uMaterial.emissive.value.rgb;
    }

    // 5. 叠加光照
    vec3 finalColor = ambient + diffuse + specular + emissive;

    // 6. 输出最终颜色
    fragColor = vec4(finalColor, 1.0);
}
