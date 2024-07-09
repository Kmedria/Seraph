#pragma once
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#define _USE_MATH_DEFINES
#include <cmath> 
#include "Math/MyMath.h"

struct vertices {
    int pos[3];
    unsigned char colour[4];
};

class Graphics {
private:

    const char* shaderSource = "src/Graphics/Shaders/Basic.shader";

    GLFWwindow* window = NULL;
    unsigned int VBO = -1, VAO = -1, EBO = -1;
    unsigned int shaderProgram;

    int screenWidth = 1920; // to detect later and change
    int screenHeight = 1080; // can and will change later
    float fovMultiplier = 1.0f; // will add settiing to adjust.
    int maxNumVerts = 1000;


    float aspectRatio = (float)screenHeight / (float)screenWidth;

    int maxSightDistance = 1000000; // 1000m , values stored as integer millimeters.
    int minSightDistance = 1;         // 1mm , values stored as integer millimeters.
    
    float fieldOfView = tan(M_PI/2 * fovMultiplier);

    Matrix projectionMat;

public:
	Graphics();
	~Graphics();

	int init();
	void draw();
	int cleanup();
};