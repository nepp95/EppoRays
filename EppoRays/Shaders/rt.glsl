#version 460 core

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

// Structs for use in layout inputs
struct Sphere
{
    vec3 Position;
    float Radius;

    uint MaterialIndex;
};

struct Material
{
    vec3 Albedo;
    float Roughness;

    vec3 Emission;
    float EmissionPower;
};

// Layout inputs
layout(std140, binding = 1) readonly buffer Spheres
{
    Sphere s_Spheres[];
};

layout(std140, binding = 2) readonly buffer Materials
{
    Material s_Materials[];
};

layout(std140, binding = 3) uniform Camera
{
	mat4 View;
    mat4 InverseView;
    mat4 Projection;
    mat4 InverseProjection;
    vec4 Position;
    vec4 Direction;
} u_Camera;

layout(binding = 0) buffer o_Buffer
{
    vec4 pixels[];
};
//

struct HitPayload
{
    float HitDistance;
    vec3 WorldPosition;
    vec3 WorldNormal;

    uint ObjectIndex;
};

struct Ray
{
    vec3 Origin;
    vec3 Direction;
};

uint FastRandom(uint seed)
{
    uint state = seed * uint(747796405) + uint(2891336453);
    uint word = ((state >> ((state >> uint(28)) + uint(4))) ^ state) * uint(277803737);
    return (word >> uint(22)) ^ word;
}

float RandomFloat(uint seed)
{
    return float(FastRandom(seed)) / float(uint(0xffffffff));
}

vec3 InUnitSphere(uint seed)
{
    return normalize(vec3(
        RandomFloat(seed) * 2.0 - 1.0,
        RandomFloat(seed) * 2.0 - 1.0,
        RandomFloat(seed) * 2.0 - 1.0
    ));
}

HitPayload ClosestHit(Ray ray, float hitDistance, int objectIndex)
{
    HitPayload payload;
    payload.HitDistance = hitDistance;
    payload.ObjectIndex = objectIndex;

    Sphere closestSphere = s_Spheres[objectIndex];

    vec3 origin = ray.Origin - closestSphere.Position;
    payload.WorldPosition = origin + ray.Direction * hitDistance;
    payload.WorldNormal = normalize(payload.WorldPosition);

    payload.WorldPosition += closestSphere.Position;

    return payload;
}

HitPayload Miss()
{
    HitPayload payload;
    payload.HitDistance = -1.0;

    return payload;
}

HitPayload TraceRay(Ray ray)
{
    int closestSphere = -1;
    float closestHit = 1000000.0;

    for (int i = 0; i < s_Spheres.length(); i++)
    {
        Sphere sphere = s_Spheres[i];

        vec3 origin = ray.Origin - sphere.Position;

        // (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
        // where
        // a = ray origin
        // b = ray direction
        // r = radius
        // t = hit distance
        float a = dot(ray.Direction, ray.Direction);
        float b = 2.0 * dot(origin, ray.Direction);
        float c = dot(origin, origin) - sphere.Radius * sphere.Radius;

        // Quadratic forumula discriminant:
        // b^2 - 4ac
        float discriminant = b * b - 4.0 * a * c;
        if (discriminant < 0)
            continue;

        // Quadratic formula:
        // (-b +- sqrt(discriminant)) / 2a
        float closestT = (-b - sqrt(discriminant)) / (2.0 * a);
        if (closestT < closestHit && closestT > 0.0)
        {
            closestHit = closestT;
            closestSphere = i;
        }
    }

    if (closestSphere < 0)
        return Miss();

    return ClosestHit(ray, closestHit, closestSphere);
}

vec3 RayGen(uint x, uint y, vec2 imageSize)
{
    // Construct the ray
    Ray ray;
    ray.Origin = u_Camera.Position.xyz;
    
    // Calculate ray direction
    vec2 coord = vec2(x / imageSize.x, y / imageSize.y);
    coord = coord * 2.0 - 1.0;

    vec4 target = u_Camera.InverseProjection * vec4(coord.xy, 1.0, 1.0);
    ray.Direction = vec3(u_Camera.InverseView * vec4(normalize(vec3(target) / target.w), 0.0));

    // Light and contribution
    vec3 light = vec3(0.0, 0.0, 0.0);
    vec3 contribution = vec3(1.0, 1.0, 1.0);

    uint seed = y * uint(imageSize.x) + x;
    seed *= uint(u_Camera.Position.w);

    // Bounce the ray around x times
    int bounces = 5;
    for (int i = 0; i < bounces; i++)
    {
        seed += i;

        HitPayload payload = TraceRay(ray);

        if (payload.HitDistance < 0.0)
        {
            // Obsolete
        }

        Sphere sphere = s_Spheres[payload.ObjectIndex];
        Material material = s_Materials[sphere.MaterialIndex];

        contribution *= material.Albedo;
        light += material.Emission * material.EmissionPower;

        ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001;
        ray.Direction = normalize(payload.WorldNormal + InUnitSphere(seed));
    }

    return light;
}

void main()
{
    vec2 index = vec2(gl_GlobalInvocationID.xy);
    vec2 imageSize = vec2(gl_NumWorkGroups.x * gl_WorkGroupSize.x, gl_NumWorkGroups.y * gl_WorkGroupSize.y);

    vec3 color = RayGen(uint(index.x), uint(index.y), imageSize);

    uint uIndex = uint(index.y) * uint(imageSize.x) + uint(index.x);
    pixels[uIndex] = vec4(color, 1.0);
}