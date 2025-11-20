// ============================================================================
// CAMERA.H - First-Person Camera System for 3D Navigation
// ============================================================================
// This class implements a free-flying camera using Euler angles (yaw/pitch)
// Supports WASD movement, mouse look, and scroll zoom
// ============================================================================

#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// ============================================================================
// ENUM: Camera Movement Directions
// ============================================================================
// Defines all possible movement directions for the camera
enum Camera_Movement {
    FORWARD,    // Move in the direction camera is facing
    BACKWARD,   // Move opposite to camera direction
    LEFT,       // Strafe left (perpendicular to forward)
    RIGHT,      // Strafe right (perpendicular to forward)
    UP,         // Move up in world space (fly up)
    DOWN        // Move down in world space (fly down)
};

// ============================================================================
// CONSTANTS: Default Camera Values
// ============================================================================
const float YAW = -90.0f;        // Initial horizontal rotation (faces -Z axis)
const float PITCH = 0.0f;        // Initial vertical rotation (level horizon)
const float SPEED = 50.0f;       // Default movement speed (units per second)
const float SENSITIVITY = 0.1f;  // Mouse sensitivity (degrees per pixel)
const float ZOOM = 45.0f;        // Default field of view in degrees

// ============================================================================
// CLASS: Camera - Handles all camera operations
// ============================================================================
// Processes input and calculates view matrix for 3D rendering
// Uses Euler angles (yaw/pitch) for rotation
// ============================================================================
class Camera
{
public:
    // ========================================================================
    // CAMERA VECTORS - Define camera's position and orientation in 3D space
    // ========================================================================
    glm::vec3 Position;   // Where the camera is located (x, y, z)
    glm::vec3 Front;      // Direction camera is facing (normalized)
    glm::vec3 Up;         // Camera's up direction (changes when looking up/down)
    glm::vec3 Right;      // Camera's right direction (for strafing)
    glm::vec3 WorldUp;    // World's up direction (usually 0,1,0 for Y-up)
    
    // ========================================================================
    // EULER ANGLES - Rotation represented by two angles
    // ========================================================================
    // Euler angles are simpler than quaternions but can have gimbal lock
    float Yaw;    // Horizontal rotation (left/right) in degrees
                  // -90° faces -Z, 0° faces +X, 90° faces +Z
    
    float Pitch;  // Vertical rotation (up/down) in degrees
                  // -90° looks straight down, 0° level, +90° straight up
    
    // ========================================================================
    // CAMERA OPTIONS - Adjustable parameters
    // ========================================================================
    float MovementSpeed;      // How fast camera moves (units/second)
    float MouseSensitivity;   // Mouse movement to rotation conversion
    float Zoom;               // Field of view in degrees (smaller = more zoom)

    // ========================================================================
    // CONSTRUCTOR - Initialize camera with position and orientation
    // ========================================================================
    // Parameters:
    //   position - Starting position in 3D space
    //   up - World up direction (default: positive Y)
    //   yaw - Initial horizontal rotation (default: -90° to face -Z)
    //   pitch - Initial vertical rotation (default: 0° for level)
    // ========================================================================
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), 
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), 
           float yaw = YAW, float pitch = PITCH) 
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)),    // Initially face -Z
          MovementSpeed(SPEED),                    // Set default speed
          MouseSensitivity(SENSITIVITY),           // Set default sensitivity
          Zoom(ZOOM)                               // Set default FOV
    {
        Position = position;  // Set camera position
        WorldUp = up;         // Set world up vector
        Yaw = yaw;           // Set horizontal rotation
        Pitch = pitch;       // Set vertical rotation
        updateCameraVectors(); // Calculate Front, Right, and Up vectors
    }

    // ========================================================================
    // GET VIEW MATRIX - Returns camera transformation matrix
    // ========================================================================
    // The view matrix transforms world coordinates to camera space
    // In OpenGL: vertex_camera_space = view_matrix * vertex_world_space
    //
    // glm::lookAt creates a view matrix from:
    //   - Camera position (where we are)
    //   - Target position (where we're looking = position + front)
    //   - Up vector (which way is "up" for the camera)
    // ========================================================================
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // ========================================================================
    // PROCESS KEYBOARD - Handle camera movement
    // ========================================================================
    // Moves camera based on input direction
    // Movement is frame-rate independent (uses deltaTime)
    //
    // Parameters:
    //   direction - Which direction to move (enum)
    //   deltaTime - Time since last frame (for smooth movement)
    // ========================================================================
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        // Calculate velocity for this frame
        // velocity = speed * time ensures same distance regardless of FPS
        float velocity = MovementSpeed * deltaTime;
        
        // Move based on direction
        if (direction == FORWARD)
            Position += Front * velocity;    // Move in facing direction
        if (direction == BACKWARD)
            Position -= Front * velocity;    // Move opposite to facing direction
        if (direction == LEFT)
            Position -= Right * velocity;    // Strafe left (perpendicular)
        if (direction == RIGHT)
            Position += Right * velocity;    // Strafe right (perpendicular)
        if (direction == UP)
            Position += Up * velocity;       // Move up in world/camera space
        if (direction == DOWN)
            Position -= Up * velocity;       // Move down in world/camera space
    }

    // ========================================================================
    // PROCESS MOUSE MOVEMENT - Handle camera rotation (look around)
    // ========================================================================
    // Rotates camera based on mouse movement
    // Prevents camera flip by constraining pitch angle
    //
    // Parameters:
    //   xoffset - Horizontal mouse movement (pixels)
    //   yoffset - Vertical mouse movement (pixels)
    //   constrainPitch - Prevent camera from flipping upside down
    //
    // How it works:
    //   1. Convert pixel movement to degrees using sensitivity
    //   2. Add to current yaw/pitch angles
    //   3. Clamp pitch to prevent gimbal lock/flipping
    //   4. Recalculate camera direction vectors
    // ========================================================================
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        // Scale mouse movement by sensitivity
        // Smaller sensitivity = slower rotation
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        // Update rotation angles
        Yaw += xoffset;    // Horizontal rotation (left/right)
        Pitch += yoffset;  // Vertical rotation (up/down)

        // Constrain pitch to prevent screen flip (gimbal lock)
        // Limit pitch to just below ±90° to avoid singularity
        if (constrainPitch)
        {
            if (Pitch > 89.0f)    // Looking too far up
                Pitch = 89.0f;
            if (Pitch < -89.0f)   // Looking too far down
                Pitch = -89.0f;
        }

        // Recalculate Front, Right, and Up vectors from new angles
        updateCameraVectors();
    }

    // ========================================================================
    // PROCESS MOUSE SCROLL - Handle zoom (field of view adjustment)
    // ========================================================================
    // Adjusts field of view to simulate zoom effect
    // Smaller FOV = zoomed in (telephoto lens)
    // Larger FOV = zoomed out (wide angle lens)
    //
    // Parameters:
    //   yoffset - Scroll wheel movement (positive = scroll up = zoom in)
    //
    // How it works:
    //   - Decrease FOV when scrolling up (zoom in)
    //   - Increase FOV when scrolling down (zoom out)
    //   - Clamp FOV between 1° and 90° for reasonable limits
    // ========================================================================
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;  // Scroll up decreases FOV (zoom in)
        
        // Clamp zoom to reasonable values
        if (Zoom < 1.0f)         // Maximum zoom in (narrow FOV)
            Zoom = 1.0f;
        if (Zoom > 90.0f)        // Maximum zoom out (wide FOV)
            Zoom = 90.0f;
    }

private:
    // ========================================================================
    // UPDATE CAMERA VECTORS - Recalculate direction vectors from Euler angles
    // ========================================================================
    // Converts Yaw and Pitch angles into Front, Right, and Up vectors
    // Called whenever rotation changes (mouse movement)
    //
    // Math explanation:
    //   Using spherical to Cartesian conversion:
    //   x = cos(yaw) * cos(pitch)   - Horizontal component
    //   y = sin(pitch)               - Vertical component
    //   z = sin(yaw) * cos(pitch)   - Depth component
    //
    // Then calculates Right and Up using cross products:
    //   Right = Front × WorldUp (perpendicular to both)
    //   Up = Right × Front (perpendicular to both)
    // ========================================================================
    void updateCameraVectors()
    {
        // ====================================================================
        // CALCULATE FRONT VECTOR - Direction camera is facing
        // ====================================================================
        glm::vec3 front;
        
        // Convert Euler angles (degrees) to direction vector
        // cos(yaw) * cos(pitch) gives X component
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        
        // sin(pitch) gives Y component (vertical look)
        front.y = sin(glm::radians(Pitch));
        
        // sin(yaw) * cos(pitch) gives Z component
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        
        // Normalize to unit vector (length = 1)
        // Essential for consistent movement speed
        Front = glm::normalize(front);
        
        // ====================================================================
        // CALCULATE RIGHT VECTOR - Camera's right direction (for strafing)
        // ====================================================================
        // Cross product of Front and WorldUp gives perpendicular vector
        // This is the direction "to the right" of the camera
        Right = glm::normalize(glm::cross(Front, WorldUp));
        
        // ====================================================================
        // CALCULATE UP VECTOR - Camera's up direction
        // ====================================================================
        // Cross product of Right and Front gives camera's local up
        // This changes when looking up/down (unlike WorldUp which is constant)
        Up = glm::normalize(glm::cross(Right, Front));
    }
};

#endif
