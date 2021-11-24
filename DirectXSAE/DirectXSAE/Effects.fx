struct Light
{
    float3 dir;
    float3 pos;
    float range;
    float3 att;
    float4 ambient;
    float4 diffuse;
};

cbuffer cbPerFrame
{
    Light light;
};

cbuffer cbPerObject
{
    float4x4 WVP;
    float4x4 worldMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4x4 inverseMatrix;
    float4x4 reflMatrix;
    float3 cameraPos;
};

Texture2D ObjTexture;
SamplerState ObjSamplerState;
TextureCube SkyMap;
Texture2D ReflTexture;
samplerCUBE SkyboxSampler; //How do i fill this


//REFLECTION WITH ENVIRONMENT MAPPING
struct RE_VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 ref : TEXCOORD;
};

RE_VS_OUTPUT RE_VS(float4 pos : POSITION, float3 normal : NORMAL)
{
    RE_VS_OUTPUT output;

    output.pos = mul(pos, WVP);
 
    float4 VertexPosition = mul(pos, worldMatrix);
    float3 ViewDirection = cameraPos - VertexPosition;
 
    float3 Normal = normalize(mul(float4(normal, 0.0f), inverseMatrix).xyz);
    output.ref = reflect(-normalize(ViewDirection), normalize(Normal));
    return output;
}

float4 RE_PS(RE_VS_OUTPUT input) : SV_TARGET
{
    return texCUBE(SkyboxSampler, normalize(input.ref));
    //return float4(1.0f, 1.0f, 1.0f, 1.0f);

}


//SKYBOX SHADER

struct SKYMAP_VS_OUTPUT   
{
    float4 Pos : SV_POSITION;
    float3 texCoord : TEXCOORD;
};

SKYMAP_VS_OUTPUT SKYMAP_VS(float3 inPos : POSITION, float2 inTexCoord : TEXCOORD, float3 normal : NORMAL)
{
    SKYMAP_VS_OUTPUT output;

    output.Pos = mul(float4(inPos, 1.0f), WVP).xyww;

    output.texCoord = inPos;

    return output;
}

float4 SKYMAP_PS(SKYMAP_VS_OUTPUT input) : SV_Target
{
    return SkyMap.Sample(ObjSamplerState, input.texCoord);
}

//Reflection with second camera

struct REFL_VS_INPUT
{
    float4 pos : POSITION;
    float2 tex : TEXCOORD;
};

struct REFL_VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD0;
    float4 reflPos : TEXCOORD1;
};

REFL_VS_OUTPUT REFL_VS(REFL_VS_INPUT input)
{
    REFL_VS_OUTPUT output;
    float4x4 reflectProjectWorld;

    input.pos.w = 1.0f;

    output.pos = mul(input.pos, WVP);
    
    output.tex = input.tex;

    reflectProjectWorld = mul(reflMatrix, projectionMatrix);
    reflectProjectWorld = mul(worldMatrix, reflectProjectWorld);

    output.reflPos = mul(input.pos, reflectProjectWorld);

    return output;
}

float4 REFL_PS(REFL_VS_OUTPUT input) : SV_TARGET
{
    float4 textureColor;
    float2 reflectTexCoord;
    float4 reflectionColor;
    float4 color;

    textureColor = ObjTexture.Sample(ObjSamplerState, input.tex);

   
    reflectTexCoord.x = input.reflPos.x / input.reflPos.w / 2.0f + 0.5f;
    reflectTexCoord.y = -input.reflPos.y / input.reflPos.w / 2.0f + 0.5f;

  
 
    reflectionColor = ReflTexture.Sample(ObjSamplerState, reflectTexCoord);

 


    color = lerp(textureColor, reflectionColor, 0.15f);

    return color;
    
}

//DIFFUSE A

struct VS_INPUT
{
    float4 vertex : POSITION;
    float2 tex : TEXCOORD;
    float3 normal : NORMAL;
};


struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 worldPos : POSITION;
    float2 tex : TEXCOORD;
    float3 normalDir : NORMAL;

};


VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;

    output.pos = mul(input.vertex, WVP);

    output.tex = input.tex;
    
    output.normalDir = normalize(mul(input.normal, (float3x3) inverseMatrix));

    return output;
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{

    float4 texColor;
    float3 lightDir;
    float lightIntesity;
    float4 color;
    float4 diffColor;

    texColor = ObjTexture.Sample(ObjSamplerState, input.tex);

    diffColor = float4(1.0f, 1.0f, 1.0f, 1.0f);

    lightIntesity = saturate(dot(input.normalDir, light.dir));
    lightIntesity = max(lightIntesity, 0.3f);

    color = saturate(diffColor * lightIntesity) * texColor;

    return color;
}

//RGB Shader
struct RGB_VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 col : TEXCOORD;
};


RGB_VS_OUTPUT RGB_VS(float4 inPos : POSITION, float2 inTex : TEXCOORD)
{
    RGB_VS_OUTPUT output;

    output.pos = mul(inPos, WVP);
    output.col.rgb = inPos.rgb + 0.5;
    output.col.a = 1.0;

    return output;
}

float4 RGB_PS(RGB_VS_OUTPUT input) : SV_TARGET
{
    return input.col;
}

//DIFFUSE B
struct VO_Diff
{
    float4 pos : SV_POSITION;
    float2 tex : COLOR;
    float3 normal : NORMAL;

};

VO_Diff VSDiff(float4 position : POSITION, float2 tex : TEXCOORD, float3 normal : NORMAL)
{
    VO_Diff output;

    float3 normalDirection = normalize(mul(float4(normal, 0.0f), inverseMatrix).xyz);

    output.tex = tex;
    output.pos = mul(position, WVP);
    output.normal = normalDirection;
    return output;
}

float4 PSDiff(VO_Diff input) : SV_TARGET
{
    float3 lightDirection = normalize(float3(0.0f, 3.0f, 0.0f));
    float4 objDiff = ObjTexture.Sample(ObjSamplerState, input.tex);
    float3 diffRefl = float3(1.0f, 1.0f, 1.0f) * objDiff.rgb * max(0.0f, dot(input.normal, lightDirection));


    return float4(diffRefl, 1.0f);
}

