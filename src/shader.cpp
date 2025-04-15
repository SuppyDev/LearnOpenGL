//
// Created by Niek Melet on 3/27/2025.
//

#include "shader.h"

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

#include <glad/glad.h>

Shader::Shader(const char *vertexPath, const char *fragmentPath) {
    // 1. retrieve the vertex/fragment source code from the filepath
    std::string vertexCode;
    std::string fragmentCode;

    try {
        std::ifstream fShaderFile;
        std::ifstream vShaderFile;

        // open files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;

        // read files
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        // convert stream info to str
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    } catch (std::ifstream::failure &e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << e.what() << std::endl;
    }

    const char *vShaderCode = vertexCode.c_str();
    const char *fShaderCode = fragmentCode.c_str();

    // 2. compile shader
    unsigned int vertex, fragment;

    // vert shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    glCompileShader(vertex);
    checkCompileErrors(static_cast<GLint>(vertex), "VERTEX");

    // frag shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);
    checkCompileErrors(static_cast<GLint>(fragment), "FRAGMENT");

    // 3. create the program
    program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    checkCompileErrors(static_cast<GLint>(program), "PROGRAM");

    // delete shaders; already linked to program
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::~Shader() {
    glDeleteProgram(program);
}

void Shader::checkCompileErrors(const GLint shader, const std::string &type) {
    GLint success;
    GLchar info[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, info);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << info << "\n -- -------------------------------------- --\n";
        }
    }
    else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, nullptr, info);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << info << "\n -- ----------------------------------------- --\n";
        }
    }
}

/** Activate the shader */
void Shader::use() const {
    glUseProgram(program);
}

// Utility uniform functions
// ---------------------------------------------------------------------------------------------------------------------
void Shader::setBool(const std::string &name, const bool value) const {
    glUniform1i(glGetUniformLocation(program, name.c_str()), static_cast<int>(value));
}

// ---------------------------------------------------------------------------------------------------------------------

void Shader::setInt(const std::string &name, const int value) const {
    glUniform1i(glGetUniformLocation(program, name.c_str()), value);
}

// ---------------------------------------------------------------------------------------------------------------------

void Shader::setFloat(const std::string &name, const float value) const {
    glUniform1f(glGetUniformLocation(program, name.c_str()), value);
}

// ---------------------------------------------------------------------------------------------------------------------

void Shader::setVec2(const std::string &name, const glm::vec2 &value) const {
    glUniform2fv(glGetUniformLocation(program, name.c_str()), 1, &value[0]);
}

// ---------------------------------------------------------------------------------------------------------------------

void Shader::setVec2(const std::string &name, float x, float y) const {
    glUniform2f(glGetUniformLocation(program, name.c_str()), x, y);
}

// ---------------------------------------------------------------------------------------------------------------------

void Shader::setVec3(const std::string &name, const glm::vec3 &value) const {
    glUniform3fv(glGetUniformLocation(program, name.c_str()), 1, &value[0]);
}

// ---------------------------------------------------------------------------------------------------------------------

void Shader::setVec3(const std::string &name, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(program, name.c_str()), x, y, z);
}

// ---------------------------------------------------------------------------------------------------------------------

void Shader::setVec4(const std::string &name, const glm::vec4 &value) const {
    glUniform4fv(glGetUniformLocation(program, name.c_str()), 1, &value[0]);
}

// ---------------------------------------------------------------------------------------------------------------------

void Shader::setVec4(const std::string &name, float x, float y, float z, float w) const {
    glUniform4f(glGetUniformLocation(program, name.c_str()), x, y, z, w);
}

// ---------------------------------------------------------------------------------------------------------------------

void Shader::setMat2(const std::string &name, const glm::mat2 &mat) const {
    glUniformMatrix2fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

// ---------------------------------------------------------------------------------------------------------------------

void Shader::setMat3(const std::string &name, const glm::mat3 &mat) const {
    glUniformMatrix3fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

// ---------------------------------------------------------------------------------------------------------------------

void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const {
    glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
