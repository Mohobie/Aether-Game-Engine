#include "renderer.h"
#include <glad/gl.h>
#include <iostream>

namespace vge {

Renderer::Renderer() : vao(0), vbo(0), ebo(0), shaderProgram(0) {}

Renderer::~Renderer() {
    Cleanup();
}

bool Renderer::Initialize() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    shaderProgram = CreateDefaultShader();
    if (!shaderProgram) {
        std::cerr << "Failed to create default shader" << std::endl;
        return false;
    }
    
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    
    return true;
}

void Renderer::Shutdown() {
    Cleanup();
}

void Renderer::BeginFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::EndFrame() {}

void Renderer::SetClearColor(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
}

void Renderer::SetViewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
}

void Renderer::Cleanup() {
    if (vao) { glDeleteVertexArrays(1, &vao); vao = 0; }
    if (vbo) { glDeleteBuffers(1, &vbo); vbo = 0; }
    if (ebo) { glDeleteBuffers(1, &ebo); ebo = 0; }
    if (shaderProgram) { glDeleteProgram(shaderProgram); shaderProgram = 0; }
}

uint32_t Renderer::CreateDefaultShader() {
    const char* vertexSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aColor;
        uniform mat4 mvp;
        out vec3 vertexColor;
        void main() {
            gl_Position = mvp * vec4(aPos, 1.0);
            vertexColor = aColor;
        }
    )";
    
    const char* fragmentSource = R"(
        #version 330 core
        in vec3 vertexColor;
        out vec4 FragColor;
        void main() {
            FragColor = vec4(vertexColor, 1.0);
        }
    )";
    
    uint32_t vs = CompileShader(vertexSource, GL_VERTEX_SHADER);
    uint32_t fs = CompileShader(fragmentSource, GL_FRAGMENT_SHADER);
    
    if (!vs || !fs) return 0;
    
    uint32_t program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    
    glDeleteShader(vs);
    glDeleteShader(fs);
    
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader link error: " << infoLog << std::endl;
        glDeleteProgram(program);
        return 0;
    }
    
    return program;
}

uint32_t Renderer::CompileShader(const char* source, uint32_t type) {
    uint32_t shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compile error: " << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

} // namespace vge
