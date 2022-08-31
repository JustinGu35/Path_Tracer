
// Uncomment the #define that corresponds
// to the scene you want to display

//#define CORNELL_BOX
//#define GLASS_BALL_BOX
#define MICROFACET_TEST



#ifdef CORNELL_BOX
#define N_RECTANGLES 6
#define N_BOXES 2

// Scene shape arrays
Rectangle rectangles[N_RECTANGLES];
Box boxes[N_BOXES];

void initializeScene() {
    int objCount = 0;
    // Floor
    rectangles[0] = Rectangle(vec3(0, -2.5, 0),
                              vec3(0,1,0),
                              vec2(5, 5),
                              vec3(0,0,0),
                              objCount++,
                              Material(vec3(0.725, 0.71, 0.68),
                                       0.0,
                                       DIFFUSE_REFL));
    // Left wall
    rectangles[1] = Rectangle(vec3(5, 2.5, 0),
                              vec3(-1,0,0),
                              vec2(5, 5),
                              vec3(0,0,0),
                              objCount++,
                              Material(vec3(0.63, 0.065, 0.05),
                                       0.0,
                                       DIFFUSE_REFL));
    // Right wall
    rectangles[2] = Rectangle(vec3(-5, 2.5, 0),
                              vec3(1,0,0),
                              vec2(5, 5),
                              vec3(0,0,0),
                              objCount++,
                              Material(vec3(0.14, 0.45, 0.091),
                                       0.0,
                                       DIFFUSE_REFL));
    // Back wall
    rectangles[3] = Rectangle(vec3(0, 2.5, 5),
                              vec3(0,0,-1),
                              vec2(5, 5),
                              vec3(0,0,0),
                              objCount++,
                              Material(vec3(0.725, 0.71, 0.68),
                                       0.0,
                                       DIFFUSE_REFL));
    // Ceiling
    rectangles[4] = Rectangle(vec3(0, 7.5, 0),
                              vec3(0,-1,0),
                              vec2(5, 5),
                              vec3(0,0,0),
                              objCount++,
                              Material(vec3(0.725, 0.71, 0.68),
                                       0.0,
                                       DIFFUSE_REFL));
    // Light source
    rectangles[5] = Rectangle(vec3(0, 7.45, 0),
                              vec3(0,-1,0),
                              vec2(1.5, 1.5),
                              vec3(40,40,40),
                              objCount++,
                              Material(vec3(0,0,0),
                                       0.0,
                                       AREA_LIGHT));

    // Long box
    boxes[0] = Box(vec3(-0.5, -0.5, -0.5),
                   vec3(0.5, 0.5, 0.5),
                   makeTransform(vec3(2, 0, 3), vec3(0, 27.5, 0), vec3(3,6,3)),
                   vec3(0,0,0),
                   objCount++,
                   Material(vec3(0.725, 0.71, 0.68),
                            0.0,
                            DIFFUSE_REFL));

    // Short box
    boxes[1] = Box(vec3(-3.5, -2.5, -0.75),
                   vec3(-0.5, 0.5, 2.25),
                   makeTransform(vec3(0), vec3(0, -17.5, 0), vec3(1)),
                   vec3(0,0,0),
                   objCount++,
                   Material(vec3(0.725, 0.71, 0.68),
                            0.0,
                            DIFFUSE_REFL));
}
#endif

#ifdef GLASS_BALL_BOX
#define N_RECTANGLES 6
#define N_SPHERES 1

// Scene shape arrays
Rectangle rectangles[N_RECTANGLES];
Sphere spheres[N_SPHERES];

void initializeScene() {
    int objCount = 0;
    // Floor
    rectangles[0] = Rectangle(vec3(0, -2.5, 0),
                              vec3(0,1,0),
                              vec2(5, 5),
                              vec3(0,0,0),
                              objCount++,
                              Material(vec3(0.725, 0.71, 0.68),
                                       0.0,
                                       DIFFUSE_REFL));
    // Left wall
    rectangles[1] = Rectangle(vec3(5, 2.5, 0),
                              vec3(-1,0,0),
                              vec2(5, 5),
                              vec3(0,0,0),
                              objCount++,
                              Material(vec3(0.63, 0.065, 0.05),
                                       0.0,
                                       DIFFUSE_REFL));
    // Right wall
    rectangles[2] = Rectangle(vec3(-5, 2.5, 0),
                              vec3(1,0,0),
                              vec2(5, 5),
                              vec3(0,0,0),
                              objCount++,
                              Material(vec3(0.14, 0.45, 0.091),
                                       0.0,
                                       DIFFUSE_REFL));
    // Back wall
    rectangles[3] = Rectangle(vec3(0, 2.5, 5),
                              vec3(0,0,-1),
                              vec2(5, 5),
                              vec3(0,0,0),
                              objCount++,
                              Material(vec3(0.725, 0.71, 0.68),
                                       0.0,
                                       DIFFUSE_REFL));
    // Ceiling
    rectangles[4] = Rectangle(vec3(0, 7.5, 0),
                              vec3(0,-1,0),
                              vec2(5, 5),
                              vec3(0,0,0),
                              objCount++,
                              Material(vec3(0.725, 0.71, 0.68),
                                       0.0,
                                       DIFFUSE_REFL));
    // Light source
    rectangles[5] = Rectangle(vec3(0, 7.45, 0),
                              vec3(0,-1,0),
                              vec2(1.5, 1.5),
                              vec3(40,40,40),
                              objCount++,
                              Material(vec3(0,0,0),
                                       0.0,
                                       AREA_LIGHT));

    // Glass ball
    spheres[0] = Sphere(vec3(0, 1.25, 0),
                        3.,
                        vec3(0,0,0),
                        objCount++,
                        Material(vec3(0.9, 0.9, 1),
                                 0.0,
                                 GLASS));
}
#endif

#ifdef MICROFACET_TEST
#define N_RECTANGLES 6
#define N_BOXES 2

Rectangle rectangles[N_RECTANGLES];
Box boxes[N_BOXES];

void initializeScene() {
    int objCount = 0;
    // Floor
    rectangles[0] = Rectangle(vec3(0, -2.5, 0),
                              vec3(0,1,0),
                              vec2(5, 5),
                              vec3(0,0,0),
                              objCount++,
                              Material(vec3(0.725, 0.71, 0.68),
                                       0.0,
                                       DIFFUSE_REFL));
    // Left wall
    rectangles[1] = Rectangle(vec3(5, 2.5, 0),
                              vec3(-1,0,0),
                              vec2(5, 5),
                              vec3(0,0,0),
                              objCount++,
                              Material(vec3(0.63, 0.065, 0.05),
                                       0.0,
                                       DIFFUSE_REFL));
    // Right wall
    rectangles[2] = Rectangle(vec3(-5, 2.5, 0),
                              vec3(1,0,0),
                              vec2(5, 5),
                              vec3(0,0,0),
                              objCount++,
                              Material(vec3(0.14, 0.45, 0.091),
                                       0.0,
                                       DIFFUSE_REFL));
    // Back wall
    rectangles[3] = Rectangle(vec3(0, 2.5, 5),
                              vec3(0,0,-1),
                              vec2(5, 5),
                              vec3(0,0,0),
                              objCount++,
                              Material(vec3(1,1,1),
                                       0.05,
                                       MICROFACET_REFL));
    // Ceiling
    rectangles[4] = Rectangle(vec3(0, 7.5, 0),
                              vec3(0,-1,0),
                              vec2(5, 5),
                              vec3(0,0,0),
                              objCount++,
                              Material(vec3(0.725, 0.71, 0.68),
                                       0.0,
                                       DIFFUSE_REFL));
    // Light source
    rectangles[5] = Rectangle(vec3(0, 7.45, 0),
                              vec3(0,-1,0),
                              vec2(1.5, 1.5),
                              vec3(40,40,40),
                              objCount++,
                              Material(vec3(0,0,0),
                                       0.0,
                                       AREA_LIGHT));

    // Long box
    boxes[0] = Box(vec3(-0.5, -0.5, -0.5),
                   vec3(0.5, 0.5, 0.5),
                   makeTransform(vec3(2, 0, 3), vec3(0, 27.5, 0), vec3(3,6,3)),
                   vec3(0,0,0),
                   objCount++,
                   Material(vec3(0.725, 0.71, 0.68),
                            0.0,
                            DIFFUSE_REFL));

    // Short box
    boxes[1] = Box(vec3(-3.5, -2.5, -0.75),
                   vec3(-0.5, 0.5, 2.25),
                   makeTransform(vec3(0), vec3(0, -17.5, 0), vec3(1)),
                   vec3(0,0,0),
                   objCount++,
                   Material(vec3(0.725, 0.71, 0.68),
                            0.0,
                            DIFFUSE_REFL));
}
#endif
