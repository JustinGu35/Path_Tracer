// optimized algorithm for solving quadratic equations developed by Dr. Po-Shen Loh -> https://youtu.be/XKBX0r3J-9Y
// Adapted to root finding (ray t0/t1) for all quadric shapes (sphere, ellipsoid, cylinder, cone, etc.) by Erich Loftis
void solveQuadratic(float A, float B, float C, out float t0, out float t1) {
    float invA = 1.0 / A;
    B *= invA;
    C *= invA;
    float neg_halfB = -B * 0.5;
    float u2 = neg_halfB * neg_halfB - C;
    float u = u2 < 0.0 ? neg_halfB = 0.0 : sqrt(u2);
    t0 = neg_halfB - u;
    t1 = neg_halfB + u;
}

float sphereIntersect(Ray ray, float radius, vec3 pos) {
    float t0, t1;
    vec3 diff = ray.origin - pos;
    float a = dot(ray.direction, ray.direction);
    float b = 2.0 * dot(ray.direction, diff);
    float c = dot(diff, diff) - (radius * radius);
    solveQuadratic(a, b, c, t0, t1);
    return t0 > 0.0 ? t0 : t1 > 0.0 ? t1 : INFINITY;
}

float planeIntersect( vec4 pla, vec3 rayOrigin, vec3 rayDirection ) {
    vec3 n = pla.xyz;
    float denom = dot(n, rayDirection);

    vec3 pOrO = (pla.w * n) - rayOrigin;
    float result = dot(pOrO, n) / denom;
    return (result > 0.0) ? result : INFINITY;
}

float rectangleIntersect(vec3 pos, vec3 normal,
                         float radiusU, float radiusV,
                         vec3 rayOrigin, vec3 rayDirection) {
        float dt = dot(-normal, rayDirection);
        // use the following for one-sided rectangle
        if (dt < 0.0) return INFINITY;
        float t = dot(-normal, pos - rayOrigin) / dt;
        if (t < 0.0) return INFINITY;

        vec3 hit = rayOrigin + rayDirection * t;
        vec3 vi = hit - pos;
        vec3 U = normalize( cross( abs(normal.y) < 0.9 ? vec3(0, 1, 0) : vec3(1, 0, 0), normal ) );
        vec3 V = cross(normal, U);
        return (abs(dot(U, vi)) > radiusU || abs(dot(V, vi)) > radiusV) ? INFINITY : t;
}

float boxIntersect(vec3 minCorner, vec3 maxCorner,
                   mat4 invT, mat3 invTransT,
                   vec3 rayOrigin, vec3 rayDirection,
                   out vec3 normal, out bool isRayExiting) {
        rayOrigin = vec3(invT * vec4(rayOrigin, 1.));
        rayDirection = vec3(invT * vec4(rayDirection, 0.));
        vec3 invDir = 1.0 / rayDirection;
        vec3 near = (minCorner - rayOrigin) * invDir;
        vec3 far  = (maxCorner - rayOrigin) * invDir;
        vec3 tmin = min(near, far);
        vec3 tmax = max(near, far);
        float t0 = max( max(tmin.x, tmin.y), tmin.z);
        float t1 = min( min(tmax.x, tmax.y), tmax.z);
        if (t0 > t1) return INFINITY;
        if (t0 > 0.0) // if we are outside the box
        {
                normal = -sign(rayDirection) * step(tmin.yzx, tmin) * step(tmin.zxy, tmin);
                normal = normalize(invTransT * normal);
                isRayExiting = false;
                return t0;
        }
        if (t1 > 0.0) // if we are inside the box
        {
                normal = -sign(rayDirection) * step(tmax, tmax.yzx) * step(tmax, tmax.zxy);
                normal = normalize(invTransT * normal);
                isRayExiting = true;
                return t1;
        }
        return INFINITY;
}

Intersection sceneIntersect(Ray ray) {
    float t = INFINITY;
    Intersection result;
    result.t = INFINITY;

    for(int i = 0; i < N_RECTANGLES; ++i) {
        float d = rectangleIntersect(rectangles[i].pos, rectangles[i].nor,
                                     rectangles[i].halfSideLengths.x,
                                     rectangles[i].halfSideLengths.y,
                                     ray.origin, ray.direction);
        if(d < t) {
            t = d;
            result.t = t;
            result.nor = rectangles[i].nor;
            result.Le = rectangles[i].Le;
            result.obj_ID = rectangles[i].ID;
            result.material = rectangles[i].material;
        }
    }

#ifdef CORNELL_BOX
    for(int i = 0; i < N_BOXES; ++i) {
        vec3 nor;
        bool isExiting;
        float d = boxIntersect(boxes[i].minCorner, boxes[i].maxCorner,
                               boxes[i].transform.invT, boxes[i].transform.invTransT,
                               ray.origin, ray.direction,
                               nor, isExiting);
        if(d < t) {
            t = d;
            result.t = t;
            result.nor = nor;
            result.Le = boxes[i].Le;
            result.obj_ID = boxes[i].ID;
            result.material = boxes[i].material;
        }
    }
#endif
#ifdef MICROFACET_TEST
    for(int i = 0; i < N_BOXES; ++i) {
        vec3 nor;
        bool isExiting;
        float d = boxIntersect(boxes[i].minCorner, boxes[i].maxCorner,
                               boxes[i].transform.invT, boxes[i].transform.invTransT,
                               ray.origin, ray.direction,
                               nor, isExiting);
        if(d < t) {
            t = d;
            result.t = t;
            result.nor = nor;
            result.Le = boxes[i].Le;
            result.obj_ID = boxes[i].ID;
            result.material = boxes[i].material;
        }
    }
#endif
#ifdef GLASS_BALL_BOX
    for(int i = 0; i < N_SPHERES; ++i) {
        vec3 nor;
        bool isExiting;
        float d = sphereIntersect(ray, spheres[i].radius, spheres[i].pos);
        if(d < t) {
            t = d;
            vec3 p = ray.origin + t * ray.direction;
            result.t = t;
            result.nor = normalize(p - spheres[i].pos);
            result.Le = spheres[i].Le;
            result.obj_ID = spheres[i].ID;
            result.material = spheres[i].material;
        }
    }
#endif
    return result;
}
