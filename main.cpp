#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <memory>
#include "Camera.h"
#include "Shader.h"
#include "Sphere.h"
#include "Planet.h"
#include "Skybox.h"

// ============================================================================
// WINDOW SETTINGS
// ============================================================================
const unsigned int SCR_WIDTH = 1920;   // Window width in pixels
const unsigned int SCR_HEIGHT = 1080;  // Window height in pixels

// ============================================================================
// CAMERA SYSTEM
// ============================================================================
// Camera object - handles view transformations and movement
// Initial position: x=0, y=50 (above), z=150 (back from center)
Camera camera(glm::vec3(0.0f, 50.0f, 150.0f));

// Mouse position tracking for camera rotation
float lastX = SCR_WIDTH / 2.0f;   // Last X position of mouse
float lastY = SCR_HEIGHT / 2.0f;   // Last Y position of mouse
bool firstMouse = true;            // Flag to prevent camera jump on first mouse movement

// ============================================================================
// TIMING SYSTEM
// ============================================================================
// Delta time - time between current frame and last frame
// Used to make movement frame-rate independent
float deltaTime = 0.0f;  // Time between current and last frame
float lastFrame = 0.0f;  // Time of last frame

// ============================================================================
// CONTROL VARIABLES
// =========================================================================]]
float cameraSpeed = 50.0f;      // Units per second camera moves
bool showOrbits = true;         // Toggle orbit lines visibility
float timeScale = 1.0f;         // Animation speed multiplier (1.0 = normal speed)
bool pauseAnimation = false;    // Pause/resume planetary motion

// ============================================================================
// FUNCTION DECLARATIONS (Callbacks and helpers)
// ============================================================================
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void renderPlanet(Planet& planet, Shader& shader, Sphere& sphere, float currentFrame);

// ============================================================================
// MAIN FUNCTION - Entry point of the program
// ============================================================================
int main()
{
    // ------------------------------------------------------------------------
    // STEP 1: Initialize GLFW (Graphics Library Framework)
    // ------------------------------------------------------------------------
    // Initialize GLFW library
    glfwInit();  
    
    // Tell GLFW what version of OpenGL we're using (3.3)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);  // OpenGL major version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);  // OpenGL minor version
    
    // Use Core Profile (modern OpenGL, no deprecated features)
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Enable MSAA (Multi-Sample Anti-Aliasing) for smooth edges
    // 4 samples per pixel reduces jagged edges
    glfwWindowHint(GLFW_SAMPLES, 4);

    // ------------------------------------------------------------------------
    // STEP 2: Create Window
    // ------------------------------------------------------------------------
    // Create the window object
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3D Solar System", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();  // Clean up GLFW resources
        return -1;
    }
    
    // Make the window's context current (all OpenGL calls will affect this window)
    glfwMakeContextCurrent(window);
    
    // Register callback functions (called automatically by GLFW on events)
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  // Window resize
    glfwSetCursorPosCallback(window, mouse_callback);                   // Mouse movement
    glfwSetScrollCallback(window, scroll_callback);                     // Mouse scroll

    // ------------------------------------------------------------------------
    // STEP 3: Capture Mouse (for camera control)
    // ------------------------------------------------------------------------
    // Hide cursor and capture it (mouse movements rotate camera)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // ------------------------------------------------------------------------
    // STEP 4: Load OpenGL Function Pointers (GLAD)
    // ------------------------------------------------------------------------
    // GLAD loads all OpenGL function pointers for the current context
    // Must be done after creating context but before calling OpenGL functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // ------------------------------------------------------------------------
    // STEP 5: Configure OpenGL State
    // ------------------------------------------------------------------------
    // Enable depth testing - objects further away are drawn behind closer ones
    // Essential for 3D rendering to look correct
    glEnable(GL_DEPTH_TEST);
    
    // Enable MSAA (Multi-Sample Anti-Aliasing) for smooth edges
    glEnable(GL_MULTISAMPLE);
    
    // Enable blending for transparency support (alpha channel)
    glEnable(GL_BLEND);
    // Set blend function: how to mix source and destination colors
    // SRC_ALPHA uses source's alpha, ONE_MINUS_SRC_ALPHA uses inverse
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ------------------------------------------------------------------------
    // STEP 6: Build and Compile Shaders
    // ------------------------------------------------------------------------
    // Shaders are small programs that run on the GPU
    // Vertex shader: transforms vertices (positions)
    // Fragment shader: calculates pixel colors
    
    Shader planetShader("planet.vs", "planet.fs");    // For rendering planets with procedural textures
    Shader orbitShader("orbit.vs", "orbit.fs");       // For rendering orbit lines
    Shader skyboxShader("skybox.vs", "skybox.fs");    // For rendering starfield background

    // ------------------------------------------------------------------------
    // STEP 7: Create Geometry
    // ------------------------------------------------------------------------
    // Sphere object generates vertices for a sphere mesh
    // Parameters: (horizontal segments, vertical segments)
    // More segments = smoother sphere but more GPU work
    Sphere sphere(50, 50);
    
    // Skybox is a cube that surrounds the scene for the background
    Skybox skybox;

    // ------------------------------------------------------------------------
    // STEP 8: Initialize Celestial Bodies (Planets and Moon)
    // ------------------------------------------------------------------------
    // Vector to store all planets (using smart pointers for automatic memory management)
    std::vector<std::shared_ptr<Planet>> planets;
    
    // Sun (center of solar system) - Type 0
    // Parameters: name, size, orbit radius, rotation speed, orbit speed, color, isSun, type
    planets.push_back(std::make_shared<Planet>("Sun", 10.0f, 0.0f, 0.0f, 0.0f, 
        glm::vec3(1.0f, 0.9f, 0.2f), true, 0));
    
    // Mercury - Type 1 (closest to sun, fastest orbit)
    planets.push_back(std::make_shared<Planet>("Mercury", 0.8f, 20.0f, 4.15f, 0.24f,
        glm::vec3(0.7f, 0.7f, 0.7f), false, 1));
    
    // Venus - Type 2 (second planet, thick atmosphere)
    planets.push_back(std::make_shared<Planet>("Venus", 1.5f, 30.0f, 1.62f, 0.615f,
        glm::vec3(0.9f, 0.7f, 0.4f), false, 2));
    
    // Earth - Type 3 (our home planet)
    // Saved to separate variable so we can attach the moon to it
    auto earth = std::make_shared<Planet>("Earth", 1.6f, 42.0f, 1.0f, 1.0f,
        glm::vec3(0.2f, 0.5f, 0.9f), false, 3);
    planets.push_back(earth);
    
    // Moon - Type 9 (orbits Earth, not the Sun)
    auto moon = std::make_shared<Planet>("Moon", 0.4f, 4.0f, 0.5f, 13.4f,
        glm::vec3(0.7f, 0.7f, 0.7f), false, 9);
    earth->addMoon(moon);  // Attach moon to Earth (parent-child relationship)
    
    // Mars - Type 4 (the red planet)
    planets.push_back(std::make_shared<Planet>("Mars", 1.2f, 55.0f, 0.53f, 1.88f,
        glm::vec3(0.9f, 0.3f, 0.2f), false, 4));
    
    // Jupiter - Type 5 (largest planet, gas giant)
    planets.push_back(std::make_shared<Planet>("Jupiter", 5.0f, 75.0f, 0.08f, 11.86f,
        glm::vec3(0.8f, 0.7f, 0.5f), false, 5));
    
    // Saturn - Type 6 (ringed planet, gas giant)
    planets.push_back(std::make_shared<Planet>("Saturn", 4.5f, 95.0f, 0.03f, 29.46f,
        glm::vec3(0.9f, 0.8f, 0.6f), false, 6));
    
    // Uranus - Type 7 (ice giant, tilted rotation)
    planets.push_back(std::make_shared<Planet>("Uranus", 2.5f, 115.0f, 0.01f, 84.01f,
        glm::vec3(0.5f, 0.8f, 0.9f), false, 7));
    
    // Neptune - Type 8 (furthest planet, ice giant)
    planets.push_back(std::make_shared<Planet>("Neptune", 2.4f, 130.0f, 0.006f, 164.79f,
        glm::vec3(0.2f, 0.3f, 0.9f), false, 8));

    // ------------------------------------------------------------------------
    // STEP 9: Generate Orbit Circles (for visualization)
    // ------------------------------------------------------------------------
    // Create circular paths showing where planets orbit
    std::vector<std::vector<glm::vec3>> orbits;  // Store orbit points
    
    // Loop through planets (skip Sun at index 0)
    for (size_t i = 1; i < planets.size(); i++) {
        std::vector<glm::vec3> orbitPoints;
        int segments = 200;  // Number of points in circle (higher = smoother)
        float radius = planets[i]->orbitRadius;  // Distance from sun
        
        // Generate points in a circle using trigonometry
        for (int j = 0; j <= segments; j++) {
            // Convert segment index to angle (0 to 2? radians = full circle)
            float angle = (float)j / segments * 2.0f * 3.14159265359f;
            
            // Calculate position using circle equation: x = cos(?) * r, z = sin(?) * r
            orbitPoints.push_back(glm::vec3(
                cos(angle) * radius,  // X position
                0.0f,                 // Y position (orbits are horizontal)
                sin(angle) * radius   // Z position
            ));
        }
        orbits.push_back(orbitPoints);  // Add this orbit to collection
    }
    
    // Generate orbit for the Moon (separate because it orbits Earth, not Sun)
    std::vector<glm::vec3> moonOrbit;
    int moonSegments = 100;  // Fewer segments (smaller circle)
    for (int j = 0; j <= moonSegments; j++) {
        float angle = (float)j / moonSegments * 2.0f * 3.14159265359f;
        moonOrbit.push_back(glm::vec3(
            cos(angle) * moon->orbitRadius,
            0.0f,
            sin(angle) * moon->orbitRadius
        ));
    }

    // ------------------------------------------------------------------------
    // STEP 10: Create GPU Buffers for Orbits
    // ------------------------------------------------------------------------
    // VAO (Vertex Array Object) - stores vertex attribute configuration
    // VBO (Vertex Buffer Object) - stores actual vertex data
    std::vector<unsigned int> orbitVAOs, orbitVBOs;
    
    // Create buffers for each planet's orbit
    for (const auto& orbit : orbits) {
        unsigned int VAO, VBO;
        
        // Generate buffer objects (get IDs from OpenGL)
        glGenVertexArrays(1, &VAO);  // Create VAO
        glGenBuffers(1, &VBO);       // Create VBO
        
        // Bind VAO (all subsequent vertex attribute calls will be stored in this VAO)
        glBindVertexArray(VAO);
        
        // Bind and fill VBO with orbit points
        glBindBuffer(GL_ARRAY_BUFFER, VBO);  // Bind VBO as array buffer
        // Upload orbit data to GPU
        // GL_STATIC_DRAW = data won't change (optimization hint)
        glBufferData(GL_ARRAY_BUFFER, orbit.size() * sizeof(glm::vec3), &orbit[0], GL_STATIC_DRAW);
        
        // Tell OpenGL how to interpret the vertex data
        // Index 0 = position attribute in vertex shader
        // 3 floats per vertex (x, y, z)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(0);  // Enable the attribute
        
        // Store VAO and VBO IDs for later use
        orbitVAOs.push_back(VAO);
        orbitVBOs.push_back(VBO);
    }
    
    // Create buffers for moon's orbit (same process as above)
    unsigned int moonOrbitVAO, moonOrbitVBO;
    glGenVertexArrays(1, &moonOrbitVAO);
    glGenBuffers(1, &moonOrbitVBO);
    glBindVertexArray(moonOrbitVAO);
    glBindBuffer(GL_ARRAY_BUFFER, moonOrbitVBO);
    glBufferData(GL_ARRAY_BUFFER, moonOrbit.size() * sizeof(glm::vec3), &moonOrbit[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    // ------------------------------------------------------------------------
    // STEP 11: MAIN RENDER LOOP
    // ------------------------------------------------------------------------
    // Loop continues until window should close (ESC pressed or X clicked)
    while (!glfwWindowShouldClose(window))
    {
        // ====================================================================
        // TIMING - Calculate frame time for smooth movement
        // ====================================================================
        // Get current time in seconds
        float currentFrame = glfwGetTime();        
        // Time since last frame
        deltaTime = currentFrame - lastFrame;      
        // Update last frame time
        lastFrame = currentFrame;                  

        // ====================================================================
        // INPUT - Process keyboard and mouse
        // ====================================================================
        processInput(window);

        // ====================================================================
        // UPDATE - Update planet positions (orbital mechanics)
        // ====================================================================
        if (!pauseAnimation) {
            // Update each planet's position based on elapsed time
            for (auto& planet : planets) {
                // timeScale allows speed control
                planet->update(deltaTime * timeScale);  
            }
        }

        // ====================================================================
        // RENDER - Draw everything to screen
        // ====================================================================
        
        // Clear the screen (color and depth buffers)
        // Color: pure black (skybox will provide stars)
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        // Clear both color and depth information from previous frame
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ====================================================================
        // CAMERA MATRICES - Set up view and projection
        // ====================================================================
        
        // Projection matrix - converts 3D to 2D (perspective)
        // Creates perspective view (objects further away appear smaller)
        // Parameters: FOV (field of view), aspect ratio, near plane, far plane
        glm::mat4 projection = glm::perspective(
            glm::radians(camera.Zoom),              // Field of view (angle)
            (float)SCR_WIDTH / (float)SCR_HEIGHT,   // Aspect ratio (width/height)
            0.1f,                                   // Near clipping plane
            1000.0f                                 // Far clipping plane
        );
        
        // View matrix - represents camera position and orientation
        glm::mat4 view = camera.GetViewMatrix();

        // ====================================================================
        // RENDER SKYBOX (Stars and Milky Way background)
        // ====================================================================
        // Rendered first but appears behind everything due to depth trick
        
        // Change depth function temporarily
        // LEQUAL allows skybox to be drawn at maximum depth
        glDepthFunc(GL_LEQUAL);
        
        // Activate skybox shader program
        skyboxShader.use();  
        
        // Remove translation from view matrix (skybox follows camera)
        // mat3 conversion removes translation component
        glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
        
        // Send matrices to shader
        skyboxShader.setMat4("view", skyboxView);
        skyboxShader.setMat4("projection", projection);
        skyboxShader.setFloat("time", currentFrame);  // For animation
        
        // Draw the skybox cube
        skybox.Draw();  
        
        // Restore default depth function
        glDepthFunc(GL_LESS);

        // ====================================================================
        // RENDER ORBIT LINES (if enabled)
        // ====================================================================
        if (showOrbits) {
            // Activate orbit shader
            orbitShader.use();  
            
            // Send matrices to shader
            orbitShader.setMat4("projection", projection);
            orbitShader.setMat4("view", view);
            orbitShader.setMat4("model", glm::mat4(1.0f));  // Identity (no transformation)
            orbitShader.setVec3("orbitColor", glm::vec3(0.3f, 0.3f, 0.5f));  // Blue-ish color

            // Draw each planet's orbit
            for (size_t i = 0; i < orbitVAOs.size(); i++) {
                glBindVertexArray(orbitVAOs[i]);  // Bind orbit VAO
                // GL_LINE_LOOP draws connected lines in a loop
                glDrawArrays(GL_LINE_LOOP, 0, 201);  // 201 vertices
            }
            
            // Render moon orbit (translated to Earth's position)
            glm::mat4 moonOrbitModel = glm::mat4(1.0f);
            // Translate orbit to Earth's current position
            moonOrbitModel = glm::translate(moonOrbitModel, earth->position);
            orbitShader.setMat4("model", moonOrbitModel);
            orbitShader.setVec3("orbitColor", glm::vec3(0.5f, 0.5f, 0.6f));  // Lighter color
            glBindVertexArray(moonOrbitVAO);
            glDrawArrays(GL_LINE_LOOP, 0, moonSegments + 1);
        }

        // ====================================================================
        // RENDER PLANETS AND MOONS
        // ====================================================================
        // Activate planet shader
        planetShader.use();  
        
        // Send matrices to shader
        planetShader.setMat4("projection", projection);
        planetShader.setMat4("view", view);
        
        // Send lighting information
        planetShader.setVec3("lightPos", glm::vec3(0.0f, 0.0f, 0.0f));  // Sun at origin
        planetShader.setVec3("viewPos", camera.Position);  // Camera position for specular
        planetShader.setFloat("time", currentFrame);  // For animated textures

        // Draw each planet (and their moons recursively)
        for (auto& planet : planets) {
            renderPlanet(*planet, planetShader, sphere, currentFrame);
        }

        // ====================================================================
        // SWAP BUFFERS AND POLL EVENTS
        // ====================================================================
        // Double buffering: draw to back buffer, display front buffer
        // Prevents flickering by showing completed frames
        glfwSwapBuffers(window);
        
        // Process pending events (keyboard, mouse, window events)
        glfwPollEvents();
    }

    // ------------------------------------------------------------------------
    // CLEANUP - Free GPU resources before exit
    // ------------------------------------------------------------------------
    // Delete all orbit VAOs and VBOs
    for (auto vao : orbitVAOs) glDeleteVertexArrays(1, &vao);
    for (auto vbo : orbitVBOs) glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &moonOrbitVAO);
    glDeleteBuffers(1, &moonOrbitVBO);

    // Cleanup GLFW resources and close window
    glfwTerminate();
    return 0;
}

// ============================================================================
// HELPER FUNCTION: Render a planet and all its moons recursively
// ============================================================================
void renderPlanet(Planet& planet, Shader& shader, Sphere& sphere, float currentFrame)
{
    // ========================================================================
    // TRANSFORMATION MATRIX - Position, rotate, and scale the planet
    // ========================================================================
    
    // Start with identity matrix (no transformation)
    glm::mat4 model = glm::mat4(1.0f);
    
    // 1. Translate to planet's orbital position
    model = glm::translate(model, planet.position);
    
    // 2. Rotate around Y-axis (planet's spin)
    // Rotation speed varies per planet, multiplied by time for continuous rotation
    model = glm::rotate(model, currentFrame * planet.rotationSpeed, 
        glm::vec3(0.0f, 1.0f, 0.0f));  // Rotate around Y axis
    
    // 3. Scale to planet's size
    model = glm::scale(model, glm::vec3(planet.size));

    // ========================================================================
    // SET SHADER UNIFORMS - Send data to GPU
    // ========================================================================
    shader.setMat4("model", model);                      // Transformation matrix
    shader.setVec3("objectColor", planet.color);         // Base color
    shader.setBool("isSun", planet.isSun);               // Is this the sun?
    shader.setInt("planetType", planet.planetType);      // Planet type for texturing

    // ========================================================================
    // DRAW THE SPHERE - Actually render the geometry
    // ========================================================================
    sphere.Draw();  // Draw sphere mesh (same mesh reused for all planets)
    
    // ========================================================================
    // RECURSIVELY RENDER MOONS - Draw any satellites
    // ========================================================================
    // Recursively call this function for each moon
    // Moons are planets that orbit other planets instead of the sun
    for (auto& moon : planet.moons) {
        renderPlanet(*moon, shader, sphere, currentFrame);
    }
}

// ============================================================================
// CALLBACK: Process keyboard and mouse input
// ============================================================================
void processInput(GLFWwindow* window)
{
    // ========================================================================
    // ESC KEY - Close window
    // ========================================================================
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // ========================================================================
    // CAMERA MOVEMENT - WASD + Space + Shift
    // ========================================================================
    // Movement speed scaled by deltaTime for frame-rate independence
    float velocity = cameraSpeed * deltaTime;
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);    // Move forward
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);   // Move backward
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);       // Move left
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);      // Move right
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);         // Move up
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);       // Move down

    // ========================================================================
    // TOGGLE ORBITS - O key
    // ========================================================================
    // Static variable remembers state between function calls
    static bool orbitKeyPressed = false;
    
    // Toggle on key press (not held)
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS && !orbitKeyPressed) {
        showOrbits = !showOrbits;  // Flip boolean
        orbitKeyPressed = true;     // Mark as pressed
    }
    // Reset flag when key released
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_RELEASE) {
        orbitKeyPressed = false;
    }

    // ========================================================================
    // PAUSE/RESUME ANIMATION - P key
    // ========================================================================
    static bool pauseKeyPressed = false;
    
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !pauseKeyPressed) {
        pauseAnimation = !pauseAnimation;  // Toggle pause state
        pauseKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
        pauseKeyPressed = false;
    }

    // ========================================================================
    // TIME CONTROL - Arrow keys
    // ========================================================================
    // Speed up time (Up arrow) - max 10x speed
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        timeScale = std::min(timeScale + 0.5f * deltaTime, 10.0f);
    
    // Slow down time (Down arrow) - min 0.1x speed
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        timeScale = std::max(timeScale - 0.5f * deltaTime, 0.1f);

    // ========================================================================
    // CAMERA SPEED CONTROL - +/- keys
    // ========================================================================
    // Increase camera speed (max 200 units/sec)
    if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS || 
        glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
        cameraSpeed = std::min(cameraSpeed + 10.0f * deltaTime, 200.0f);
    
    // Decrease camera speed (min 5 units/sec)
    if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS || 
        glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
        cameraSpeed = std::max(cameraSpeed - 10.0f * deltaTime, 5.0f);
}

// ============================================================================
// CALLBACK: Window resize - adjust viewport
// ============================================================================
// Called automatically by GLFW when window is resized
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // Viewport defines the region of the window used for rendering
    // (0,0) = bottom-left corner
    // Make viewport match window size (full window rendering)
    glViewport(0, 0, width, height);
}

// ============================================================================
// CALLBACK: Mouse movement - rotate camera
// ============================================================================
// Called automatically by GLFW when mouse moves
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    // ========================================================================
    // FIRST MOUSE MOVEMENT - Prevent camera jump
    // ========================================================================
    // On first mouse movement, just record position (don't calculate offset)
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    // ========================================================================
    // CALCULATE MOUSE OFFSET - How far mouse moved
    // ========================================================================
    float xoffset = xpos - lastX;       // Horizontal movement
    float yoffset = lastY - ypos;       // Vertical movement (reversed: y-coordinates go from bottom to top)

    // Update last position for next frame
    lastX = xpos;
    lastY = ypos;

    // ========================================================================
    // UPDATE CAMERA ORIENTATION - Apply rotation
    // ========================================================================
    // Camera class handles the actual rotation calculations
    camera.ProcessMouseMovement(xoffset, yoffset);
}

// ============================================================================
// CALLBACK: Mouse scroll - zoom in/out
// ============================================================================
// Called automatically by GLFW when mouse wheel scrolled
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // yoffset: positive = scroll up (zoom in), negative = scroll down (zoom out)
    // Camera class adjusts field of view for zoom effect
    camera.ProcessMouseScroll(yoffset);
}
