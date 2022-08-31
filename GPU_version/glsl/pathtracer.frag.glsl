// Taken from https://github.com/erichlof/THREE.js-PathTracing-Renderer/blob/gh-pages/js/PathTracingCommon.js
// globals used in rand() function
vec4 randVec4; // samples and holds the RGBA blueNoise texture value for this pixel
float randNumber; // the final randomly generated number (range: 0.0 to 1.0)
float counter; // will get incremented by 1 on each call to rand()
int channel; // the final selected color channel to use for rand() calc (range: 0 to 3, corresponds to R,G,B, or A)
float rand() {
        counter++; // increment counter by 1 on every call to rand()
        // cycles through channels, if modulus is 1.0, channel will always be 0 (the R color channel)
        channel = int(mod(counter, 2.0));
        // but if modulus was 4.0, channel will cycle through all available channels: 0,1,2,3,0,1,2,3, and so on...
        randNumber = randVec4[channel]; // get value stored in channel 0:R, 1:G, 2:B, or 3:A
        return fract(randNumber); // we're only interested in randNumber's fractional value between 0.0 (inclusive) and 1.0 (non-inclusive)
}

float rng1D(float p) {
    return fract(sin(p * 127.1f) * 43758.5453f);
}
float rng1D(vec3 p) {
    return fract(sin(dot(p, vec3(127.1f, 311.7f, 420.69f)))
                           * 43758.5453f);
}
vec2 rng2D(float p) {
    return fract(sin(vec2(p * 127.1f, p * 269.5f)) * 43758.5453f);
}
vec2 rng2D(vec2 p) {
    return fract(sin(vec2(dot(p, vec2(127.1f, 311.7f)),
                          dot(p, vec2(269.5f, 183.3f)))) * 43758.5453f);
}
vec2 rng2D(vec3 p) {
    return fract(sin(vec2(dot(p, vec3(127.1f, 311.7f, 420.69f)),
                          dot(p, vec3(269.5f, 183.3f, 934.456f)))) * 43758.5453f);
}

// from ShaderToy https://www.shadertoy.com/view/4tXyWN
//uvec2 seed;
//float rng() {
//    seed += uvec2(1);
//    uvec2 q = 1103515245U * ( (seed >> 1U) ^ (seed.yx) );
//    uint  n = 1103515245U * ( (q.x) ^ (q.y >> 3U) );
//    return float(n) * (1.0 / float(0xffffffffU));
//}
//moved to bsdf

const float FOVY = 19.5f * PI / 180.0;
Ray rayCast() {
    vec2 offset = vec2(rng(), rng());
    vec2 ndc = (vec2(gl_FragCoord.xy) + offset) / vec2(u_ScreenDims);
    ndc = ndc * 2.f - vec2(1.f);

    float aspect = u_ScreenDims.x / u_ScreenDims.y;
    vec3 ref = u_Eye + u_Forward;
    vec3 V = u_Up * tan(FOVY * 0.5);
    vec3 H = u_Right * tan(FOVY * 0.5) * aspect;
    vec3 p = ref + H * ndc.x + V * ndc.y;

    return Ray(u_Eye, normalize(p - u_Eye));
}

vec3 DirectLightSample(vec3 view_point, vec3 nor, out vec3 wiW, out float pdf) {
    // TODO
    // Choose a light source to sample and return its radiance.
    // It is NOT required for you to do this with MIS; light source sampling
    // is sufficient. You may implement MIS for extra credit.

    // This will have to be somewhat hard-coded, unless you want to
    // add a list of Light structs to iterate over to allow for arbitrary
    // scene contents.

    // As far as we require, you must only sample the following
    // two light sources:
    // - The singular diffuse area light provided in all three Cornell Box type scenes
    // - The environment map provided via the u_EnvironmentMap sampler.
    //   You can sample it with UV coordinates obtained from the function
    //   sampleSphericalMap() provided in the "defines" file.

    // To sample the environment map, just use cosine-weighted hemisphere
    // sampling to choose a wi direction.

    vec2 xi = vec2(rng(),rng());

    if(rng()<0.5)
    {
        vec3 wi =squareToHemisphereCosine(xi);
        pdf = squareToHemisphereCosinePDF(wi);
        wi = LocalToWorld(nor) * wi;
        Ray testRay = SpawnRay(view_point, normalize(wi));
        wiW = testRay.direction;
        Intersection isect = sceneIntersect(testRay);
        if(isect.t == INFINITY)
        {
            vec2 uv = sampleSphericalMap(wi);
            return 2.0f * texture(u_EnvironmentMap, uv).rgb;
        }
    }
    else
    {
        vec3 color = rectangles[5].Le;
        vec3 pos = rectangles[5].pos;
        vec2 sideLength = rectangles[5].halfSideLengths;

        xi = xi * 2.0f - vec2(1.0f);
        vec3 tan, bit;
        coordinateSystem(rectangles[5].nor, tan, bit);
        vec3 sample = xi.x * tan * sideLength.x + xi.y * bit * sideLength.y + pos;

        Ray testRay = SpawnRay(view_point, normalize(sample - view_point));
        wiW = testRay.direction;

        float r2 = dot(sample - view_point, sample - view_point);
        pdf = r2 / (AbsDot(testRay.direction, rectangles[5].nor) * 4 * sideLength.x * sideLength.y);

        Intersection isect = sceneIntersect(testRay);
        if(dot(rectangles[5].nor, - wiW) <= 0)
        {
            return vec3(0.);
        }

        if(isect.material.type == AREA_LIGHT)
        {
            return 2.f * rectangles[5].Le;
        }
    }

    return vec3(0.);
}

// For now, just Naive integration
vec3 Li(Ray ray, out int objectHit, out vec3 objectNormal, out float pixelSharpness) {
    // TODO
    vec3 accumLight = vec3(0.f); // energy
    vec3 rayColor = vec3(1.f); //throughput
    bool prevSpecular = false;
    for(int i = 0; i <MAX_DEPTH; ++i)
    {
        Intersection isect = sceneIntersect(ray);
        objectNormal = isect.nor;
        if(isect.t == INFINITY)
        {
            if(i == 0)
            {
                vec2 uv = sampleSphericalMap(ray.direction);
                accumLight += rayColor * texture(u_EnvironmentMap,uv).rgb;
            }
            else if(prevSpecular)
            {
                vec2 uv = sampleSphericalMap(ray.direction);
                accumLight += rayColor * texture(u_EnvironmentMap,uv).rgb;
            }
            break;
        }
        if(isect.material.type == AREA_LIGHT)
        {
            if(i == 0 || prevSpecular)
            {
                accumLight += isect.Le * rayColor;
            }
            break;
        }

        vec3 wo = -ray.direction;
        vec3 wiW = vec3(0.f); // ray from intersect to light
        float pdf = 0.f;
        vec3 view_point = ray.origin + isect.t * ray.direction; // intersect

        if(isect.material.type == SPEC_REFL ||isect.material.type == SPEC_TRANS ||
                isect.material.type == GLASS ||isect.material.type == MICROFACET_REFL)
        {

            vec2 xi = vec2(rng(),rng());
            int sampledType = 0;
            vec3 gi_f = Sample_f(isect, wo,xi,wiW,pdf,sampledType);
            if((gi_f == vec3(0.f))|| pdf==0.)
            {
                break;
            }
            ray = SpawnRay(view_point,wiW);
            rayColor *= gi_f * AbsDot(wiW,isect.nor) /pdf;
            prevSpecular = true;
        }
        else if(isect.material.type == DIFFUSE_REFL)
        {
            prevSpecular = false;
            vec3 directLight = DirectLightSample(view_point,objectNormal, wiW, pdf);

            if(pdf != 0.f)
            {
                vec3 f = isect.material.albedo * INV_PI;
                accumLight += f * rayColor * directLight * AbsDot(wiW,isect.nor)/pdf;
            }

            //global illumination
            vec3 gi_woW = -ray.direction;
            float gi_pdf = 0.f;
            vec3 gi_W = vec3(0.f);

            vec2 xi = vec2(rng(),rng());
            //xi = xi * 2.0f -1.0f;

            int sampledType = 0;

            vec3 gi_f = Sample_f(isect, gi_woW, xi, gi_W, gi_pdf, sampledType);
            if((gi_f == vec3(0.f))|| gi_pdf==0.)
            {
                break;
            }
            gi_W = normalize(gi_W);

            ray = SpawnRay(view_point, gi_W);

            rayColor *= gi_f * AbsDot(gi_W, isect.nor) / gi_pdf;
        }
    }


    return accumLight;
}

void initializeRNGs() {
    // Initialize state for rand() function, which samples a noisy texture
    // to generate a sequence of random values.
    channel = 0;
    counter = -1.0; // will get incremented by 1 on each call to rand()
    randNumber = 0.;
    randVec4 = texelFetch(u_BlueNoiseTex,
                          ivec2(mod(gl_FragCoord.xy +
                                    floor(vec2(314.159, 690.420123) *
                                          256.0),
                                    256.0)), 0);
    seed = uvec2(u_Iterations, u_Iterations + 1) * uvec2(gl_FragCoord.xy);
}

void main() {

    initializeRNGs();

    // Changes at compile time based on which scene #define
    // is uncommented
    initializeScene();

    Ray ray = rayCast();

    int objectHit = 0;
    vec3 objectNormal = vec3(0.);
    float pixelSharpness = 0.f;
    vec3 thisColor = Li(ray, objectHit, objectNormal, pixelSharpness);

    out_Col = mix(texture(u_AccumImg,fs_UV), vec4(thisColor,1.0f), 1.0f/u_Iterations);
    //out_Col = vec4(thisColor, 1.);
}
