
vec3 squareToDiskConcentric(vec2 xi) {
    // Map [0, 1) to [-1, 1)
    vec2 sampleOffset = 2.f * xi - vec2(1, 1);

    // Handle degeneracy case at origin
    if(sampleOffset.x == 0 && sampleOffset.y == 0)
    {
        return vec3(0,0,0);
    }

    // Perform the concentric mapping
    float theta, r;
    // Check if we're in the "horizontal" quadrants of the disk (-45 to 45, 135 to 215)
    if(abs(sampleOffset.x) > abs(sampleOffset.y))
    {
        r = sampleOffset.x;
        theta = PI / 4.f * (sampleOffset.y / sampleOffset.x);
    }
    // Else we're in the "vertical" quadrants of the disk (45 to 135, 215 to 315)
    else
    {
        r = sampleOffset.y;
        theta = PI / 2.f - PI / 4.f * (sampleOffset.x / sampleOffset.y);
    }

    return r * vec3(cos(theta), sin(theta), 0);
}

vec3 squareToHemisphereCosine(vec2 xi) {
    vec3 p = squareToDiskConcentric(xi);
    p.z = sqrt(max(0.f, 1 - p.x * p.x - p.y * p.y));
    return p;
}

float squareToHemisphereCosinePDF(vec3 sample) {
    float cosTheta = dot(sample, vec3(0,0,1));
    return cosTheta * INV_PI;
}
