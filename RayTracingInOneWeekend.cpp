#include <iostream>
#include <limits>
#include <functional>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "rtweekend.h"
#include "ray.h"
#include "hittable.h"
#include "camera.h"
#include "material.h"

using namespace std;

// configs
const int window_width = 400;
const int window_height = 400;
const double infinity = std::numeric_limits<double>::infinity();
const int samples = 4;
const int ray_depth = 10;

const float aspect_ratio = static_cast<float>(window_width) / window_height;

const char* vertexShaderSource =
"#version 330 core\n"
"layout(location = 0) in vec3 aPos;\n"
"layout(location = 1) in vec2 aTexCoord;\n"

"out vec2 TexCoord;\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(aPos, 1.0);\n"
"    TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
"}\n";

const char* fragmentShaderSource =
"#version 330 core\n"
"out vec4 FragColor;\n"

"in vec2 TexCoord;\n"

"uniform sampler2D texture1;\n"

"void main()\n"
"{\n"
"    FragColor = texture(texture1, TexCoord);\n"
"}\n";


int main() {

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(window_width, window_height, "Ray Tracing In OneWeekend", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return 1;
    }

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
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
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
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

    float quadVertices[] = {
     -1.0f,  1.0f,  0.0f, 1.0f,
     -1.0f, -1.0f,  0.0f, 0.0f,
      1.0f, -1.0f,  1.0f, 0.0f,

     -1.0f,  1.0f,  0.0f, 1.0f,
      1.0f, -1.0f,  1.0f, 0.0f,
      1.0f,  1.0f,  1.0f, 1.0f
    };

    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), static_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    auto* data = new unsigned char[window_height * window_width * 3];

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width, window_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    glDisable(GL_DEPTH_TEST);

    // helper functions
    auto setPixelColor = [](int h, int w, unsigned char* p, const glm::vec3& col)
    {
        int index = 3 * (h * window_width + w);
        unsigned char r = sqrt(col.r) * 255;
        unsigned char g = sqrt(col.g) * 255;
        unsigned char b = sqrt(col.b) * 255;
        p[index++] = r;
        p[index++] = g;
        p[index++] = b;
    };

	// shapes
    shared_ptr<lambertian> red = make_shared<lambertian>(vec3(0.9, 0.1, 0.1));
	shared_ptr<lambertian> blue = make_shared<lambertian>(vec3(0.3, 0.9, 1.0));
	shared_ptr<metal> greenMatel = make_shared<metal>(vec3(0.1, 0.9, 0.1));
	shared_ptr<metal> whiteMatel = make_shared<metal>(vec3(1., 1., 1.));
	shared_ptr<FuzzyMetal> fuzzyRed = make_shared<FuzzyMetal>(vec3(0.9, 0.1, 0.1), 0.8f);
	shared_ptr<dielectric> glass = make_shared<dielectric>(1.5f);
    hittable_list world;
    world.add(make_shared<sphere>(glm::vec3(-4, 0, -4), 2.0, blue));
    world.add(make_shared<sphere>(glm::vec3(0, 0, -3), 2.0, glass));
    world.add(make_shared<sphere>(glm::vec3(0, 0, -3), -1.8, glass));
    world.add(make_shared<sphere>(glm::vec3(4, 0, -4), 2.0, fuzzyRed));
    world.add(make_shared<sphere>(glm::vec3(0, -100.5, 0), 98.5, red));
	
    // rendering
    bool needUpdate = true;
    std::function<glm::vec3(const ray&, const hittable_list&, int)> ray_color = [&](const ray& r, const hittable_list& list, int depth)->glm::vec3
    {
        hit_record record;
        if (depth <= 0) return vec3(0.f);
    	if(list.hit(r, .001, infinity, record))
    	{
            ray scattered;
            vec3 attenuation;
            if (record.pMat->scatter(r, record, attenuation, scattered))
            {
                return attenuation * ray_color(scattered, list, depth - 1);
            }
            else
            {
                return vec3(0);
            }
    	}
        glm::vec3 normDir = glm::normalize(r.direction());
        float t = 0.5 * (normDir.y + 1);
        return t * glm::vec3(0.5, 0.7, 1.0) + (1 - t) * glm::vec3(1);
    };

	// camera
    glm::vec3 eye(0.f, 1.f, 2.f);
    glm::vec3 center(0.f, 0.f, -4.f);
    glm::vec3 up(0.f, 1.f, 0.f);
    camera cam(eye, center, up,1, 2, 2 * aspect_ratio);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

    	if(needUpdate)
    	{
            for (int j = window_height - 1; j >= 0; --j) {
                for (int i = 0; i < window_width; ++i) {
                    float u = static_cast<float>(j) / window_height;
                    float v = static_cast<float>(i) / window_width;
                    glm::vec3 color(0.f);
                	for(int s = 0; s < samples; ++s)
                	{
                        color += ray_color(cam.getRayFromScreenPos(u + rtweekend::random_double() / (window_height - 1), v + rtweekend::random_double() / (window_width - 1)), world, ray_depth);
                	}
                    color /= samples;
                    setPixelColor(j, i, data, color);
                }
            }
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width, window_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            //needUpdate = false;
    	}
        
        glUseProgram(shaderProgram);
        glBindTexture(GL_TEXTURE_2D, texture);
    	glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
}