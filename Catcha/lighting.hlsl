#define MAX_LIGHTS 16

struct Light {
	float3 strength;
	float falloff_start;
	float3 direction;
	float falloff_end;
	float3 position;
	float spot_power;
};

struct Material {
	float4 diffuse_albedo;
	float3 fresnel;
	float shininess;
};

float Calc_Attenuation(float distance, float falloff_start, float falloff_end) {
	return saturate((falloff_end - distance) / (falloff_end - falloff_start));
}

float3 Schlick_Fresnel(float3 reflect_0, float3 light_vector, float3 normal) {
	float cosine = saturate(dot(normal, light_vector));

	float inverse_cosine = 1.0f - cosine;
	float3 reflect_percent = reflect_0 + (1.0f - reflect_0) *
		(inverse_cosine * inverse_cosine * inverse_cosine * inverse_cosine * inverse_cosine);

	return reflect_percent;
}

float3 Blinn_Phong(float3 light_strength, float3 light_vector, float3 normal, float3 to_eye, Material material) {
	float shininess = material.shininess * 256.0f;
	float3 half_vector = normalize(to_eye + light_vector);

	float roughness = (shininess + 8.0f) * pow(max(dot(half_vector, light_vector), 0.0f), shininess) / 8.0f;
	float3 fresnel_factor = Schlick_Fresnel(material.fresnel, light_vector, half_vector);

	float3 specular_albedo = fresnel_factor * roughness;

	specular_albedo = specular_albedo / (specular_albedo + 1.0f);

	return (material.diffuse_albedo.rgb + specular_albedo) * light_strength;
}

float3 Cpt_DL(Light light, Material material, float3 normal, float3 to_eye) {	// Compute Directional Light
	float3 light_vector = -light.direction;
	float3 light_strength = light.strength * max(dot(light_vector, normal), 0.0f);

	return Blinn_Phong(light_strength, light_vector, normal, to_eye, material);
}

float3 Cpt_PL(Light light, Material material,float3 position, float3 normal, float3 to_eye) {	// Compute Point Light
	float3 light_vector = light.position - position;
	float distance = length(light_vector);

	if (distance > light.falloff_end) {
		return 0.0f;
	}

	light_vector /= distance;

	float3 light_strength = light.strength * max(dot(light_vector, normal), 0.0f);

	return Blinn_Phong(light_strength, light_vector, normal, to_eye, material);
}

float3 Cpt_SL(Light light, Material material, float3 position, float3 normal, float3 to_eye) {	// Compute Spot Light
	float3 light_vector = light.position - position;
	float distance = length(light_vector);

	if (distance > light.falloff_end) {
		return 0.0f;
	}

	light_vector /= distance;

	float3 light_strength = light.strength * max(dot(light_vector, normal), 0.0f);

	float attenuation = Calc_Attenuation(distance, light.falloff_start, light.falloff_end);
	light_strength *= attenuation;

	float spot_factor = pow(max(dot(-light_vector, light.direction), 0.0f), light.spot_power);
	light_strength *= spot_factor;

	return Blinn_Phong(light_strength, light_vector, normal, to_eye, material);
}

float4 Cpt_Lighting(Light lights[MAX_LIGHTS], Material material, float3 position, float3 normal, float3 to_eye, float shadow_factor) {
	float3 result = 0.0f;

	int i = 0;

#if (DIRECTIONAL_LIGHTS_NUMBER > 0)
	for (i = 0; i < DIRECTIONAL_LIGHTS_NUMBER; ++i) {
		result += shadow_factor * Cpt_DL(lights[i], material, normal, to_eye);
	}
#endif

#if (POINT_LIGHTS_NUMBER > 0)
	for (i = DIRECTIONAL_LIGHTS_NUMBER; i < DIRECTIONAL_LIGHTS_NUMBER + POINT_LIGHTS_NUMBER; ++i) {
		result += Cpt_PL(lights[i], material, position, normal, to_eye);
	}
#endif

#if (SPOT_LIGHTS_NUMBER > 0)
	for (i = DIRECTIONAL_LIGHTS_NUMBER + POINT_LIGHTS_NUMBER; i < DIRECTIONAL_LIGHTS_NUMBER + POINT_LIGHTS_NUMBER + SPOT_LIGHTS_NUMBER; ++i) {
		result += Cpt_SL(lights[i], material, position, normal, to_eye);
	}
#endif

	return float4(result, 0.0f);
}
