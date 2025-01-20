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

#define GRAVITY_VALUE -980.0
#define MAX_PARTICLE_LIFE_TIME 1.0

#include "lighting.hlsl"

cbuffer CB_Object : register(b0) {
	float4x4 g_world;
    float4 g_color_multiplier;
    float3 g_additional_info;
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

Texture2D g_unicode_texture : register(t1);
SamplerState g_unicode_texture_sampler : register(s1);

Texture2D g_texture : register(t2);
SamplerState g_texture_sampler : register(s2);

struct Instance_Data {
    float4x4 instc_world_matrix;
    float4 additional_info;
};

StructuredBuffer<Instance_Data> g_instance_data : register(t0, space1);

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
    float2 uv : UV;
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

    return result;
}

float4 Silhouette_PS(Vertex_Out pixel_in) : SV_Target {
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

//
Vertex_Out Instance_VS(Vertex_In vertex_in, uint instance_id : SV_InstanceID) {
    Vertex_Out vertex_out = (Vertex_Out)0.0f;

    float4 position_world = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float3 normal_world = float3(0.0f, 0.0f, 0.0f);

    position_world = mul(float4(vertex_in.position_local, 1.0f), g_instance_data[instance_id].instc_world_matrix);

    vertex_out.position_world = position_world.xyz;
    vertex_out.normal_world = mul(vertex_in.normal_local, (float3x3)g_instance_data[instance_id].instc_world_matrix);
    vertex_out.position_screen = mul(position_world, g_view_projection);

    vertex_out.shadow_position = mul(position_world, g_shadow_transform);
    vertex_out.material_index = vertex_in.material_index;

    return vertex_out;
}

Vertex_Out Instance_Shadow_VS(Vertex_In vertex_in, uint instance_id : SV_InstanceID) {
    Vertex_Out vertex_out = (Vertex_Out)0.0f;

    float4 position_world = float4(0.0f, 0.0f, 0.0f, 0.0f);

    position_world = mul(float4(vertex_in.position_local, 1.0f), g_instance_data[instance_id].instc_world_matrix);

    vertex_out.position_world = position_world.xyz;
    vertex_out.position_screen = mul(position_world, g_view_projection);

    return vertex_out;
}

//
Vertex_Out Text_UI_VS(Vertex_In vertex_in, uint instance_id : SV_InstanceID) {
    Vertex_Out vertex_out = (Vertex_Out)0.0f;

    float4 position_screen = float4(0.0f, 0.0f, 0.0f, 0.0f);

    position_screen = mul(float4(vertex_in.position_local, 1.0f), g_instance_data[instance_id].instc_world_matrix);

    vertex_out.position_screen = position_screen;

    vertex_out.material_index = vertex_in.material_index;

    //vertex_out.uv =
    //    float2(g_instance_data[instance_id].additional_info.x +
    //        (g_instance_data[instance_id].additional_info.z - g_instance_data[instance_id].additional_info.x) * vertex_in.uv.x,
    //        g_instance_data[instance_id].additional_info.y +
    //        (g_instance_data[instance_id].additional_info.w - g_instance_data[instance_id].additional_info.y) * vertex_in.uv.y);

    vertex_out.uv = vertex_in.uv / 256.0 + g_instance_data[instance_id].additional_info.xy / 256.0;

    //vertex_out.uv = vertex_in.uv / 256.0;

    return vertex_out;
}

float4 Text_UI_PS(Vertex_Out pixel_in) : SV_Target{
    float4 texture_color;

    texture_color = g_unicode_texture.Sample(g_unicode_texture_sampler, pixel_in.uv);

    if (texture_color.r == 0.0) {
        discard;
    }

    Material material = g_material_array[pixel_in.material_index];

    float4 result = material.diffuse_albedo * g_color_multiplier;

    return result;
}

//
Vertex_Out UI_VS(Vertex_In vertex_in) {
    Vertex_Out vertex_out = (Vertex_Out)0.0f;

    float4 position_screen = float4(0.0f, 0.0f, 0.0f, 0.0f);

    float4x4 world_matrix = g_world;

    float top = world_matrix._14;
    float left = world_matrix._24;
    float bottom = world_matrix._34;
    float right = world_matrix._44;

    world_matrix._14_24_34_44 = float4(0.0, 0.0, 0.0, 1.0);

    position_screen = mul(float4(vertex_in.position_local, 1.0f), world_matrix);

    vertex_out.position_screen = position_screen;

    vertex_out.material_index = vertex_in.material_index;

    float texture_width = g_additional_info.x;
    float texture_height = g_additional_info.y;

    vertex_out.uv.x = left / texture_width + vertex_in.uv.x * (right - left) / texture_width;
    vertex_out.uv.y = top / texture_height + vertex_in.uv.y * (bottom - top) / texture_height;

    return vertex_out;
}

float4 UI_PS(Vertex_Out pixel_in) : SV_Target{
    float4 texture_color;

    texture_color = g_texture.Sample(g_texture_sampler, pixel_in.uv);

    float4 result = texture_color * g_color_multiplier;

    return result;
}

//
struct Particle_Vertex_Out {
    float3 delta_position : POSITION;
    float life_time : TIME;
    uint instance_id : INSTANCE_ID;
};

struct Geometry_Out {
    float4 position_screen : SV_POSITION;
    float4 color : COLOR;
    float2 uv : UV;
    uint primitive_id : SV_PrimitiveID;
};

Particle_Vertex_Out Particle_VS(Vertex_In vertex_in, uint instance_id : SV_InstanceID) {
    Particle_Vertex_Out vertex_out = (Particle_Vertex_Out)0.0f;

    float life_time = g_total_time - g_instance_data[instance_id].additional_info.w;

    if (life_time < MAX_PARTICLE_LIFE_TIME) {
        vertex_out.delta_position.x = g_instance_data[instance_id].additional_info.x * life_time;
        vertex_out.delta_position.z = g_instance_data[instance_id].additional_info.z * life_time;
        vertex_out.delta_position.y = g_instance_data[instance_id].additional_info.y * life_time
            + 0.5 * GRAVITY_VALUE * life_time * life_time;
    }

    vertex_out.life_time = life_time;
    vertex_out.instance_id = instance_id;

    return vertex_out;
}

[maxvertexcount(4)]
void Particle_GS(point Particle_Vertex_Out geometry_in[1],
    uint primitive_id : SV_PrimitiveID, inout TriangleStream<Geometry_Out> triangle_stream
) {
    if (geometry_in[0].life_time < MAX_PARTICLE_LIFE_TIME) {
        float3 camera_right = g_view._11_21_31 * 0.5;
        float3 camera_up = g_view._12_22_32 * 0.5;

        float4 vertex[4];
        vertex[0] = float4(-camera_right - camera_up, 1.0f);
        vertex[1] = float4(-camera_right + camera_up, 1.0f);
        vertex[2] = float4(+camera_right - camera_up, 1.0f);
        vertex[3] = float4(+camera_right + camera_up, 1.0f);

        float2 uv[4] = {
            float2(0.0, 1.0),
            float2(0.0, 0.0),
            float2(1.0, 1.0),
            float2(1.0, 0.0)
        };

        float4x4 world_matrix = g_instance_data[geometry_in[0].instance_id].instc_world_matrix;

        float4 color = world_matrix._14_24_34_44;
        world_matrix._14_24_34_44 = float4(0.0, 0.0, 0.0, 1.0);
        world_matrix._41_42_43 = world_matrix._41_42_43 + geometry_in[0].delta_position;

        Geometry_Out geometry_out;

        [unroll]
        for (int i = 0; i < 4; ++i) {
            geometry_out.position_screen = mul(mul(vertex[i], world_matrix), g_view_projection);
            geometry_out.color = color;
            geometry_out.uv = uv[i];
            geometry_out.primitive_id = primitive_id;

            triangle_stream.Append(geometry_out);
        }
    }
}

float4 Particle_PS(Geometry_Out pixel_in) : SV_Target{
    return pixel_in.color;
}