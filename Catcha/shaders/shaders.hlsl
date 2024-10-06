#ifndef DIRECTIONAL_LIGHTS_NUMBER
    #define DIRECTIONAL_LIGHTS_NUMBER 1
#endif

#ifndef POINT_LIGHTS_NUMBER
    #define POINT_LIGHTS_NUMBER 1
#endif

#ifndef SPOT_LIGHTS_NUMBER
    #define SPOT_LIGHTS_NUMBER 1
#endif

#include "lighting.hlsl"

cbuffer CB_Object : register(b0) {
	float4x4 g_world; 
};

cbuffer CB_Material : register(b1) {
    float4 g_diffuse_albedo;
    float3 g_fresnel;
    float g_roughness;
}

cbuffer CB_Pass : register(b2) {
    float4x4 g_view;
    float4x4 g_inverse_view;
    float4x4 g_projection;
    float4x4 g_inverse_projection;
    float4x4 g_view_projection;
    float4x4 g_inverse_view_projection;
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

struct Vertex_In {
	float3 position_local  : POSITION;
    float3 normal_local : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : UV;
};

struct Vertex_Out {
	float4 position_screen  : SV_POSITION;
    float3 position_world : POSITION;
    float3 normal_world : NORMAL;
};

Vertex_Out VS(Vertex_In vertex_in) {
    Vertex_Out vertex_out = (Vertex_Out)0.0f;

    float4 position_world = mul(float4(vertex_in.position_local, 1.0f), g_world);
    vertex_out.position_world = position_world.xyz;

    vertex_out.normal_world = mul(vertex_in.normal_local, (float3x3)g_world);
    vertex_out.position_screen = mul(position_world, g_view_projection);

    return vertex_out;
}

float4 PS(Vertex_Out pixel_in) : SV_Target {
    pixel_in.normal_world = normalize(pixel_in.normal_world);

    float3 to_eye_world = normalize(pixel_in.normal_world);

    float4 ambient = g_ambient_light * g_diffuse_albedo;

    float shininess = 1.0f - g_roughness;
    Material material = { g_diffuse_albedo, g_fresnel, shininess };
    float shadow_factor = 1.0f;
    
    float4 direct_light = Cpt_Lighting(g_lights, material, pixel_in.position_world, pixel_in.normal_world, to_eye_world, shadow_factor);

    float4 result = ambient + direct_light;

    result.a = g_diffuse_albedo.a;

    float luminance = dot(result.rgb, float3(0.299, 0.587, 0.114));
    result = float4(luminance, luminance, luminance, result.a);

    result = lerp(0.2, 1.0, result);

    return result;
}


