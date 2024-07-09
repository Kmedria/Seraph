#include "Graphics.h"
#include <iostream>
#include <fstream>
#include <sstream>

Graphics::Graphics() {

}

Graphics::~Graphics() {

}

int Graphics::init() {

    if (!glfwInit()) {
        std::cout << (stderr, "Failed to initialise GLFW.\n");
        return -1;
    }

    //glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 

    window = glfwCreateWindow(screenWidth, screenHeight, "Mind your own business", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window); 

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }
    
    glfwSwapInterval(1);

    if (glewInit() != GLEW_OK) {
        std::cout << "error, Glew is not working Ok." << std::endl;
        return -1;
    };

    std::ifstream stream;
    stream.open(shaderSource);
    enum class ShaderType {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[3];
    ShaderType type = ShaderType::NONE;

    while (getline(stream, line)) {
        if (line.find("#shader") != std::string::npos) {
            if (line.find("vertex") != std::string::npos) {
                type = ShaderType::VERTEX;
            }
            else {
                if (line.find("fragment") != std::string::npos) {
                    type = ShaderType::FRAGMENT;
                }
            }
        }
        else {
            ss[(int)type] << line << '\n';
        }
    }

    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);

    const std::string& vertSTR = ss[0].str();
    const char* vert = vertSTR.c_str();

    glShaderSource(vertexShader, 1, &vert, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    const std::string& fragSTR = ss[1].str();
    const char* frag = fragSTR.c_str();

    glShaderSource(fragmentShader, 1, &frag, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errorss
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) * maxNumVerts, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertices) * maxNumVerts, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    projectionMat = Matrix(4, 4);
    
    projectionMat.setData(0, 0, aspectRatio* fieldOfView);
    projectionMat.setData(1, 1, fieldOfView);
    projectionMat.setData(2, 2, maxSightDistance / (maxSightDistance - minSightDistance));
    projectionMat.setData(3, 2, -(float)minSightDistance * (maxSightDistance / (maxSightDistance - minSightDistance)));
    projectionMat.setData(2, 3, 1);
    projectionMat.setData(3, 3, 0);


    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void Graphics::draw() {
    
    int numTriangles = 2;

    //glfwGetCursorPos(window, &nx, &ny);

    float vertex2[] = {
        1000, 1000, 1000,
        0, 1000, 1000,
        0, 0, 1000
    };
        

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex2), &vertex2, GL_DYNAMIC_DRAW);

    glClearColor(0, 0, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // draw our first triangle
    int proj_trans_mat = glGetUniformLocation(shaderProgram, "project_trans_matrix");
    int colourLocation = glGetUniformLocation(shaderProgram, "u_colour");

    glUseProgram(shaderProgram);
    glUniform4f(colourLocation, 1, 1, 1, 1.0f);
    glBindVertexArray(VAO);     
    //glDrawArrays(GL_TRIANGLES, 0, 6);
    glUniformMatrix4fv(proj_trans_mat, 1, GL_FALSE, projectionMat.data);
    glDrawElements(GL_TRIANGLES, 3 * 5, GL_UNSIGNED_INT, 0);
}

int Graphics::cleanup() {
    glfwTerminate();
	return 0;
}
