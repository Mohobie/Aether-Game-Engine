#include "shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace vge {
Shader::Shader() : programId(0) {}
Shader::~Shader() { if (programId) glDeleteProgram(programId); }

bool Shader::LoadFromFile(const std::string& vPath, const std::string& fPath) {
    std::ifstream vFile(vPath), fFile(fPath);
    std::stringstream vBuf, fBuf;
    vBuf << vFile.rdbuf(); fBuf << fFile.rdbuf();
    
    uint32_t vs = glCreateShader(GL_VERTEX_SHADER);
    uint32_t fs = glCreateShader(GL_FRAGMENT_SHADER);
    std::string vSrc = vBuf.str(), fSrc = fBuf.str();
    const char* vC = vSrc.c_str(), *fC = fSrc.c_str();
    glShaderSource(vs, 1, &vC, nullptr);
    glShaderSource(fs, 1, &fC, nullptr);
    glCompileShader(vs); glCompileShader(fs);
    
    programId = glCreateProgram();
    glAttachShader(programId, vs); glAttachShader(programId, fs);
    glLinkProgram(programId);
    glDeleteShader(vs); glDeleteShader(fs);
    return programId != 0;
}

void Shader::Use() { if (programId) glUseProgram(programId); }
void Shader::SetUniform(const std::string& name, const Mat4& val) {
    int loc = glGetUniformLocation(programId, name.c_str());
    if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_FALSE, &val.m[0][0]);
}
}