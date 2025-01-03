#ifndef DIRECTIONAL_LIGHTS_NUMBER
    #define DIRECTIONAL_LIGHTS_NUMBER 1
#endif

#ifndef POINT_LIGHTS_NUMBER
    #define POINT_LIGHTS_NUMBER 0
#endif

#ifndef SPOT_LIGHTS_NUMBER
    #define SPOT_LIGHTS_NUMBER 0
#endif

#define MAX_WEIGHT_BONE_COUNT 4
#define MAX_BONE_COUNT 64

#define MAX_MATERIAL_COUNT 32

#include "lighting.hlsl"

cbuffer CB_Object : register(b0) {
	float4x4 g_world;
    float4 g_color_multiplier;
    uint g_animated;
};

cbuffer CB_Material : register(b1) {
    Material g_material_array[MAX_MATERIAL_COUNT];
}

cbuffer CB_Pass : register(b2) {
    float4x4 g_view;
    float4x4 g_inverse_view;
    float4x4 g_projection;
    float4x4 g_inverse_projection;
    float4x4 g_view_projection;
    float4x4 g_inverse_view_projection;
    float4x4 g_shadow_transform;
    float3 g_camera_position;
    float2 g_render_target_size;
    float2 g_inverse_render_target_size;
    float g_near_z;
    float g_far_z;
    float g_total_time;
    float g_delta_time;
    float4 g_ambient_light;
    Light g_lights[MAX_LIGHTS];
};

cbuffer CB_Animation : register(b3) {
    float4x4 g_animation_transform_matrix[MAX_BONE_COUNT];
}

Texture2D g_shadow_map : register(t0);
SamplerComparisonState g_shadow_sampler : register(s0);

Texture2D g_texture : register(t1);
SamplerState g_texture_sampler : register(s1);

struct Vertex_In {
	float3 position_local : POSITION;
    float3 normal_local : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : UV;
    uint bone_count : BONECOUNT;
    uint4 bone_indices : BONEINDICES;
    float4 bone_weights : BONEWEIGHTS;
    uint material_index : MATERIAL;
};

struct Vertex_Out {
	float4 position_screen  : SV_POSITION;
    float4 shadow_position : POSITION0;
    float3 position_world : POSITION1;
    float3 normal_world : NORMAL;
    uint material_index : MATERIAL;
};

float Calc_Shadow_Factor(float4 shadow_position) {
    shadow_position.xyz /= shadow_position.w;

    float depth = shadow_position.z;

    uint width, height, mip_num;
    g_shadow_map.GetDimensions(0, width, height, mip_num);

    float dx = 1.0f / (float)width;

    float percent = 0.0f;
    const float2 offsets[9] = {
        float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
    };

    [unroll]
    for (int i = 0; i < 9; ++i) {
        percent += g_shadow_map.SampleCmpLevelZero(g_shadow_sampler, shadow_position.xy + offsets[i], depth).r;
    }

    return percent / 9.0f;
}

Vertex_Out VS(Vertex_In vertex_in) {
    Vertex_Out vertex_out = (Vertex_Out)0.0f;

    float4 position_world = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float3 normal_world = float3(0.0f, 0.0f, 0.0f);
    if (g_animated) {

        for (uint i = 0; i < MAX_WEIGHT_BONE_COUNT; ++i) {
            uint bone_index = vertex_in.bone_indices[i];

            if (bone_index != -1) {
                float4x4 bone_matrix = g_animation_transform_matrix[bone_index];
                float4 weighted_position = mul(float4(vertex_in.position_local, 1.0f), bone_matrix) * vertex_in.bone_weights[i];
                float3 weighted_normal = mul(vertex_in.normal_local, (float3x3)bone_matrix) * vertex_in.bone_weights[i];

                position_world += weighted_position;
                normal_world += weighted_normal;
            }
        }

        position_world = mul(position_world, g_world);
        normal_world = mul(normal_world, (float3x3)g_world);

        vertex_out.position_world = position_world.xyz;
        vertex_out.position_screen = mul(position_world, g_view_projection);
        vertex_out.normal_world = normalize(normal_world);
    }
    else {
        position_world = mul(float4(vertex_in.position_local, 1.0f), g_world);

        vertex_out.position_world = position_world.xyz;
        vertex_out.normal_world = mul(vertex_in.normal_local, (float3x3)g_world);
        vertex_out.position_screen = mul(position_world, g_view_projection);
    }

    vertex_out.shadow_position = mul(position_world, g_shadow_transform);
    vertex_out.material_index = vertex_in.material_index;

    return vertex_out;
}

float4 PS(Vertex_Out pixel_in) : SV_Target {
    pixel_in.normal_world = normalize(pixel_in.normal_world);

    float3 to_eye_world = normalize(g_camera_position - pixel_in.position_world);

    Material material = g_material_array[pixel_in.material_index];

    float4 ambient = g_ambient_light * material.diffuse_albedo;

    float shadow_factor = Calc_Shadow_Factor(pixel_in.shadow_position);
    
    float4 direct_light;
    
    direct_light = Cpt_Lighting(g_lights, material, pixel_in.position_world, pixel_in.normal_world, to_eye_world, shadow_factor);

    float4 result = ambient + direct_light;

    result.a = material.diffuse_albedo.a;

    //
    //float luminance = dot(result.rgb, float3(0.299, 0.587, 0.114));
    //result = float4(luminance, luminance, luminance, result.a);

    //
    //result = lerp(0.2, 1.0, result);

    result = result * g_color_multiplier;

    //
    float4 test_tex_color;
    test_tex_color = g_texture.Sample(g_texture_sampler, float2(0.5, 0.5));

    result += test_tex_color;
    result -= test_tex_color;

    return result;
}

float4 Silhouette_PS(Vertex_Out pixel_in) : SV_Target{
    float4 result = float4(1.0, 0.0, 0.0, g_color_multiplier.w);

    return result;
}


Vertex_Out Shadow_VS(Vertex_In vertex_in) {
    //Vertex_Out vertex_out = (Vertex_Out)0.0f;

    //float4 position_world = mul(float4(vertex_in.position_local, 1.0f), g_world);

    //vertex_out.position_world = position_world.xyz;
    //vertex_out.position_screen = mul(position_world, g_view_projection);

    //
    Vertex_Out vertex_out = (Vertex_Out)0.0f;

    float4 position_world = float4(0.0f, 0.0f, 0.0f, 0.0f);
    if (g_animated) {

        for (uint i = 0; i < MAX_WEIGHT_BONE_COUNT; ++i) {
            uint bone_index = vertex_in.bone_indices[i];

            if (bone_index != -1) {
                float4x4 bone_matrix = g_animation_transform_matrix[bone_index];
                float4 weighted_position = mul(float4(vertex_in.position_local, 1.0f), bone_matrix) * vertex_in.bone_weights[i];

                position_world += weighted_position;
            }
        }

        position_world = mul(position_world, g_world);

        vertex_out.position_world = position_world.xyz;
        vertex_out.position_screen = mul(position_world, g_view_projection);
    }
    else {
        position_world = mul(float4(vertex_in.position_local, 1.0f), g_world);

        vertex_out.position_world = position_world.xyz;
        vertex_out.position_screen = mul(position_world, g_view_projection);
    }

    return vertex_out;
}

void Shadow_PS(Vertex_Out pixel_in) {
    // nothing
}