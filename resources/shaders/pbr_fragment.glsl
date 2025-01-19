#version 430 core

in vec3 vWorldPos;
in vec2 vTexCoords;
in mat3 vTBN;

out vec4 FragColor;

/////////////////////////////////////////////
// 1. 定义与 C++ 端对应的材质结构体
/////////////////////////////////////////////
struct MaterialParam
{
    vec4      value;   // 若是标量则通常只用 .x，如果是颜色则用 .rgb / .rgba
    bool      useMap;
    sampler2D map;
};

uniform struct
{
    MaterialParam baseColor;    // 颜色
    MaterialParam metallic;     // 金属度
    MaterialParam roughness;    // 粗糙度
    MaterialParam normal;       // 法线贴图
    MaterialParam ao;           // AO
    MaterialParam height;       // 视差或高度贴图
    MaterialParam emissive;     // 自发光
    MaterialParam opacity;      // 不透明度(可用 .value.a 或 .value.x)
    MaterialParam translucency; // 半透等其他属性(未在此示例中具体使用)
} uMaterial;

// 单个平行光参数 (示例)
uniform vec3 uLightPos; 
uniform vec3 uLightColor;
uniform vec3 uViewPos;

// IBL (可选)
uniform bool useIBL;
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D   brdfLUT;
uniform float       maxPrefilterMipLevel;

// 其它可选参数，如半透明度等，也可继续放到此

/////////////////////////////////////////////
// 常量 & 函数 (与原先相同)
/////////////////////////////////////////////
const float PI = 3.14159265359;

// Smith’s Schlick-GGX
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a  = roughness * roughness;
    float a2 = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return a2 / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;
    float denom = NdotV * (1.0 - k) + k;
    return NdotV / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// 从 NormalMap 获取切线空间法线，并转换到世界空间
vec3 getNormalFromMap()
{
    // 贴图的 RGB => [0,1], 需转换到 [-1,1]
    // uMaterial.normal.map 对应法线贴图
    vec3 normalColor = texture(uMaterial.normal.map, vTexCoords).rgb;
    vec3 N = normalColor * 2.0 - 1.0; // [0,1] -> [-1,1]
    return normalize(vTBN * N);
}

// 计算 IBL 贡献 (diffuse IBL + specular IBL)
vec3 computeIBL(vec3 N, vec3 V, float roughness, vec3 F0, vec3 baseColor, float metallic, float ao)
{
    // 漫反射 (irradianceMap)
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 kS = F0;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    vec3 diffuse = irradiance * baseColor;
    diffuse *= ao; // AO 影响环境光

    // 镜面部分 (prefilterMap + brdfLUT)
    vec3 R = reflect(-V, N);
    float mipLevel = roughness * maxPrefilterMipLevel;
    vec3 prefilteredColor = textureLod(prefilterMap, R, mipLevel).rgb;

    float NdotV = max(dot(N, V), 0.0);
    vec2 brdf  = texture(brdfLUT, vec2(NdotV, roughness)).rg;
    vec3 F = fresnelSchlick(NdotV, F0);
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    return kD * diffuse + specular;
}

/////////////////////////////////////////////
// 主片段着色器
/////////////////////////////////////////////
void main()
{
    /////////////////////////////////////
    // 1. 从 uMaterial 中获取各项参数
    /////////////////////////////////////
    // BaseColor
    vec4 baseColorVal = uMaterial.baseColor.value;  // 先取 .value
    if(uMaterial.baseColor.useMap)
    {
        // 若使用贴图，则使用贴图颜色
        baseColorVal = texture(uMaterial.baseColor.map, vTexCoords);
    }

    // Metallic (标量: 用 .value.x)
    float metallicVal = uMaterial.metallic.value.x;
    if(uMaterial.metallic.useMap)
    {
        metallicVal = texture(uMaterial.metallic.map, vTexCoords).r;
    }

    // Roughness (标量: 用 .value.x)
    float roughnessVal = uMaterial.roughness.value.x;
    if(uMaterial.roughness.useMap)
    {
        roughnessVal = texture(uMaterial.roughness.map, vTexCoords).r;
    }

    // AO
    float aoVal = uMaterial.ao.value.x;
    if(uMaterial.ao.useMap)
    {
        aoVal = texture(uMaterial.ao.map, vTexCoords).r;
    }

    // Emissive
    vec3 emissiveVal = uMaterial.emissive.value.rgb;
    if(uMaterial.emissive.useMap)
    {
        emissiveVal = texture(uMaterial.emissive.map, vTexCoords).rgb;
    }

    // Opacity
    float alphaVal = uMaterial.opacity.value.a; // 这里假设 a 通道存储不透明度
    if(uMaterial.opacity.useMap)
    {
        alphaVal = texture(uMaterial.opacity.map, vTexCoords).r; 
    }

    // Height / Translucency等，如有需求，可在此取样
    // ...

    /////////////////////////////////////
    // 2. 计算世界空间法线
    /////////////////////////////////////
    vec3 N = normalize(vTBN[2]);  // 默认法线
    if(uMaterial.normal.useMap)
    {
        N = getNormalFromMap();
    }

    /////////////////////////////////////
    // 3. 计算 V, L, H
    /////////////////////////////////////
    vec3 V = normalize(uViewPos - vWorldPos);
    vec3 L = normalize(uLightPos); // 平行光
    vec3 H = normalize(V + L);

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);

    /////////////////////////////////////
    // 4. 计算 F0
    /////////////////////////////////////
    vec3 F0 = vec3(0.04); 
    // 让金属区使用 baseColorVal 的值作为 F0
    F0 = mix(F0, baseColorVal.rgb, metallicVal);

    /////////////////////////////////////
    // 5. GGX/Cook-Torrance BRDF
    /////////////////////////////////////
    float D = DistributionGGX(N, H, roughnessVal);
    float G = GeometrySmith(N, V, L, roughnessVal);
    vec3  F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator    = D * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.0001;
    vec3 specular     = numerator / denominator;

    // kS (镜面系数) = F
    vec3 kS = F;
    // kD (漫反射系数) = (1 - kS)*(1 - metallicVal)
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallicVal);

    // 直接光
    vec3 diffuse = kD * baseColorVal.rgb / PI;
    vec3 Lo      = (diffuse + specular) * uLightColor * NdotL;
    Lo          *= aoVal; // AO 影响直接光

    /////////////////////////////////////
    // 6. IBL (可选)
    /////////////////////////////////////
    if(useIBL)
    {
        vec3 ibl = computeIBL(N, V, roughnessVal, F0, baseColorVal.rgb, metallicVal, aoVal);
        Lo += ibl;
    }

    /////////////////////////////////////
    // 7. 自发光
    /////////////////////////////////////
    Lo += emissiveVal;

    vec3 ambient = vec3(0.3) * baseColorVal.rgb; // 添加固定环境光
    Lo += ambient;

    /////////////////////////////////////
    // 8. 最终输出
    /////////////////////////////////////
    FragColor = vec4(Lo, alphaVal);
}
