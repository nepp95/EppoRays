#version 460 core

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(location = 0) uniform vec3 u_CameraPosition;
layout(location = 1) uniform vec3 u_CameraDirection;

layout(binding = 0) buffer o_Buffer
{
    vec4 pixels[];
};

struct HitPayload
{
    float HitDistance;
    vec3 WorldPosition;
    vec3 WorldNormal;
};

struct Ray
{
    vec3 Origin;
    vec3 Direction;
};

HitPayload ClosestHit(Ray ray, float hitDistance)
{
    HitPayload payload;
    payload.HitDistance = hitDistance;
    payload.WorldPosition = ray.Origin + ray.Direction * hitDistance;
    payload.WorldNormal = normalize(payload.WorldPosition);

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

    float radius = 0.5;

    // (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
    // where
    // a = ray origin
    // b = ray direction
    // r = radius
    // t = hit distance
    float a = dot(ray.Direction, ray.Direction);
    float b = 2.0 * dot(ray.Direction, ray.Origin);
    float c = dot(ray.Origin, ray.Origin) - radius * radius;

    // Quadratic forumula discriminant:
    // b^2 - 4ac
    float discriminant = b * b - 4.0 * a * c;
    if (discriminant < 0)
        return Miss();

    // Quadratic formula:
    // (-b +- sqrt(discriminant)) / 2a
    float closestT = (-b - sqrt(discriminant)) / (2.0 * a);
    
    return ClosestHit(ray, closestT);
}

void main()
{
    vec2 index = vec2(gl_GlobalInvocationID.xy);
    vec2 imageSize = vec2(gl_NumWorkGroups.x * gl_WorkGroupSize.x, gl_NumWorkGroups.y * gl_WorkGroupSize.y);

    Ray ray;
    ray.Origin = vec3(0.0, 0.0, 1.0);

    vec2 coord = vec2(index.x / imageSize.x, index.y / imageSize.y);
    coord = coord * 2.0 - 1.0;

    ray.Direction = vec3(coord, -1.0);

    vec4 color = vec4(0.0, 1.0, 0.0, 1.0);

    HitPayload payload = TraceRay(ray);
    if (payload.HitDistance < 0.0)
        color = vec4(0.0, 0.0, 0.0, 1.0);
    
    vec3 lightDirection = normalize(vec3(-1.0, -1.0, -1.0));
    float lightIntensity = max(dot(payload.WorldNormal, -lightDirection), 0.0);

    color *= lightIntensity;

    uint uIndex = uint(index.y) * uint(imageSize.x) + uint(index.x);
    pixels[uIndex] = color;
}