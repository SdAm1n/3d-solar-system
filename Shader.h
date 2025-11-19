// ============================================================================
// SHADER.H - OpenGL Shader Program Management
// ============================================================================
// This class handles loading, compiling, and using GLSL shaders
// Shaders are small programs that run on the GPU to process vertices and pixels
//
// What are shaders?
//   - Vertex Shader: Processes each vertex (position transformation)
//   - Fragment Shader: Processes each pixel (color calculation)
//   - Program: Linked vertex + fragment shader combination
//
// What are uniforms?
//   - Variables sent from CPU to GPU (same value for all vertices/pixels)
//   - Examples: transformation matrices, light positions, time, etc.
// ============================================================================

#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>       // OpenGL function loader
#include <glm/glm.hpp>       // Math library for vectors and matrices

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

// ============================================================================
// CLASS: Shader - Manages OpenGL shader programs
// ============================================================================
class Shader
{
public:
    // ========================================================================
    // MEMBER VARIABLE: Shader Program ID
    // ========================================================================
    unsigned int ID;  // OpenGL ID for the compiled shader program
                      // Used to reference this shader when rendering
    
    // ========================================================================
    // CONSTRUCTOR - Load, compile, and link shaders from files
    // ========================================================================
    // Reads shader code from files, compiles them, and creates a program
    //
    // Parameters:
    //   vertexPath - Path to vertex shader file (.vs)
    //   fragmentPath - Path to fragment shader file (.fs)
    //
    // Process:
    //   1. Read shader source code from files
    //   2. Compile vertex shader
    //   3. Compile fragment shader  
    //   4. Link shaders into program
    //   5. Clean up individual shaders (program contains combined code)
    // ========================================================================
    Shader(const char* vertexPath, const char* fragmentPath)
    {
        // ====================================================================
        // STEP 1: Read Shader Source Code from Files
        // ====================================================================
        std::string vertexCode;    // Will store vertex shader code
        std::string fragmentCode;  // Will store fragment shader code
        std::ifstream vShaderFile; // File stream for vertex shader
        std::ifstream fShaderFile; // File stream for fragment shader
        
        // Enable exception throwing for file operations
        // Helps catch file reading errors
        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        
        try
        {
            // Open shader files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            
            // String streams to read file contents
            std::stringstream vShaderStream, fShaderStream;
            
            // Read file buffer contents into streams
            // rdbuf() reads entire file content
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            
            // Close file handlers (done reading)
            vShaderFile.close();
            fShaderFile.close();
            
            // Convert stream into string (shader source code)
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (std::ifstream::failure& e)
        {
            // Handle file reading errors
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        }
        
        // Convert std::string to C-string (char*) for OpenGL
        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();
        
        // ====================================================================
        // STEP 2: Compile Shaders
        // ====================================================================
        unsigned int vertex, fragment;  // Shader object IDs
        
        // --------------------------------------------------------------------
        // Compile Vertex Shader
        // --------------------------------------------------------------------
        // Vertex shader processes each vertex (transforms positions)
        vertex = glCreateShader(GL_VERTEX_SHADER);  // Create shader object
        glShaderSource(vertex, 1, &vShaderCode, NULL);  // Attach source code
        glCompileShader(vertex);  // Compile the code on GPU
        checkCompileErrors(vertex, "VERTEX");  // Check for errors
        
        // --------------------------------------------------------------------
        // Compile Fragment Shader
        // --------------------------------------------------------------------
        // Fragment shader processes each pixel (calculates colors)
        fragment = glCreateShader(GL_FRAGMENT_SHADER);  // Create shader object
        glShaderSource(fragment, 1, &fShaderCode, NULL);  // Attach source code
        glCompileShader(fragment);  // Compile the code on GPU
        checkCompileErrors(fragment, "FRAGMENT");  // Check for errors
        
        // ====================================================================
        // STEP 3: Link Shaders into Program
        // ====================================================================
        // A program combines vertex and fragment shaders
        // It's the complete pipeline for rendering
        ID = glCreateProgram();        // Create program object
        glAttachShader(ID, vertex);    // Attach compiled vertex shader
        glAttachShader(ID, fragment);  // Attach compiled fragment shader
        glLinkProgram(ID);             // Link them together
        checkCompileErrors(ID, "PROGRAM");  // Check for linking errors
        
        // ====================================================================
        // STEP 4: Clean Up Individual Shaders
        // ====================================================================
        // Once linked into program, individual shaders are no longer needed
        // The program contains all the compiled code
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }
    
    // ========================================================================
    // ACTIVATE SHADER - Make this shader active for rendering
    // ========================================================================
    // Tells OpenGL to use this shader program for subsequent draw calls
    // Only one shader program can be active at a time
    // ========================================================================
    void use()
    {
        glUseProgram(ID);  // Activate this shader program
    }
    
    // ========================================================================
    // UTILITY FUNCTIONS - Set Uniform Variables
    // ========================================================================
    // Uniforms are variables sent from CPU (C++) to GPU (shader)
    // They're constant for all vertices/pixels in a draw call
    //
    // Process:
    //   1. Get uniform location (ID) by name
    //   2. Set value at that location
    //
    // Common uses:
    //   - Transformation matrices (model, view, projection)
    //   - Light positions and colors
    //   - Material properties
    //   - Time for animations
    // ========================================================================
    
    // ------------------------------------------------------------------------
    // Set Boolean Uniform (true/false)
    // ------------------------------------------------------------------------
    void setBool(const std::string& name, bool value) const
    {
        // Get location of uniform variable in shader by name
        // Set as integer (0 = false, 1 = true in GLSL)
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    
    // ------------------------------------------------------------------------
    // Set Integer Uniform (whole numbers)
    // ------------------------------------------------------------------------
    // Used for: texture units, array indices, enum values
    void setInt(const std::string& name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    
    // ------------------------------------------------------------------------
    // Set Float Uniform (decimal numbers)
    // ------------------------------------------------------------------------
    // Used for: time, scale factors, single values
    void setFloat(const std::string& name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    
    // ------------------------------------------------------------------------
    // Set Vec2 Uniform (2D vector: x, y)
    // ------------------------------------------------------------------------
    // Used for: 2D positions, texture coordinates, screen sizes
    void setVec2(const std::string& name, const glm::vec2& value) const
    {
        // 2fv = 2 floats as vector (array)
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    
    void setVec2(const std::string& name, float x, float y) const
    {
        // 2f = 2 separate floats
        glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
    }
    
    // ------------------------------------------------------------------------
    // Set Vec3 Uniform (3D vector: x, y, z)
    // ------------------------------------------------------------------------
    // Used for: 3D positions, colors (RGB), normals, directions
    void setVec3(const std::string& name, const glm::vec3& value) const
    {
        // 3fv = 3 floats as vector (array)
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    
    void setVec3(const std::string& name, float x, float y, float z) const
    {
        // 3f = 3 separate floats
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
    }
    
    // ------------------------------------------------------------------------
    // Set Vec4 Uniform (4D vector: x, y, z, w)
    // ------------------------------------------------------------------------
    // Used for: colors with alpha (RGBA), homogeneous coordinates
    void setVec4(const std::string& name, const glm::vec4& value) const
    {
        // 4fv = 4 floats as vector (array)
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    
    void setVec4(const std::string& name, float x, float y, float z, float w) const
    {
        // 4f = 4 separate floats
        glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
    }
    
    // ------------------------------------------------------------------------
    // Set Mat2 Uniform (2x2 matrix)
    // ------------------------------------------------------------------------
    // Used for: 2D transformations, rotations
    void setMat2(const std::string& name, const glm::mat2& mat) const
    {
        // Matrix2fv = 2x2 float matrix
        // 1 = sending 1 matrix, GL_FALSE = don't transpose
        glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    
    // ------------------------------------------------------------------------
    // Set Mat3 Uniform (3x3 matrix)
    // ------------------------------------------------------------------------
    // Used for: normal transformations, 2D transformations with translation
    void setMat3(const std::string& name, const glm::mat3& mat) const
    {
        // Matrix3fv = 3x3 float matrix
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    
    // ------------------------------------------------------------------------
    // Set Mat4 Uniform (4x4 matrix)
    // ------------------------------------------------------------------------
    // Most common! Used for: model, view, and projection transformations
    // Essential for 3D rendering pipeline
    void setMat4(const std::string& name, const glm::mat4& mat) const
    {
        // Matrix4fv = 4x4 float matrix
        // &mat[0][0] = pointer to first element of matrix
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

private:
    // ========================================================================
    // CHECK COMPILE ERRORS - Verify shader compilation/linking
    // ========================================================================
    // Checks if shader compiled successfully or program linked successfully
    // Prints errors if any problems occurred
    //
    // Parameters:
    //   shader - Shader or program ID to check
    //   type - Type of shader ("VERTEX", "FRAGMENT", or "PROGRAM")
    //
    // Why this is important:
    //   - Shader errors are common during development
    //   - OpenGL won't tell you automatically - you must check!
    //   - Errors can be syntax errors, type mismatches, etc.
    // ========================================================================
    void checkCompileErrors(GLuint shader, std::string type)
    {
        GLint success;           // Success flag
        GLchar infoLog[1024];    // Error message buffer
        
        if (type != "PROGRAM")
        {
            // ------------------------------------------------------------
            // Check Shader Compilation
            // ------------------------------------------------------------
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                // Get error message
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" 
                          << infoLog << "\n -- --------------------------------------------------- -- " 
                          << std::endl;
            }
        }
        else
        {
            // ------------------------------------------------------------
            // Check Program Linking
            // ------------------------------------------------------------
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                // Get error message
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" 
                          << infoLog << "\n -- --------------------------------------------------- -- " 
                          << std::endl;
            }
        }
    }
};

#endif
