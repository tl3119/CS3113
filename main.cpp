#include <iostream>

// The old stuff
#define GL_SILENCE_DEPRECIATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>

// The new stuff
#include "glm/mat4x4.hpp"                // 4x4 Matrix
#include "glm/gtc/matrix_transform.hpp"  // Matrix transformation methods
#include "ShaderProgram.h"               // We'll talk about these later in the course

// Our window dimensions
const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;

// Background colours
const float BG_RED = 0.1922f, BG_BLUE = 0.549f, BG_GREEN = 0.9059f;
const float BG_OPACITY = 1.0f;

// Our viewport！or our "camera"'s！position and dimensions
const int VIEWPORT_X = 0;
const int VIEWPORT_Y = 0;
const int VIEWPORT_WIDTH = WINDOW_WIDTH;
const int VIEWPORT_HEIGHT = WINDOW_HEIGHT;

// Our shader filepaths; these are necessary for a number of things
// Not least, to actually draw our shapes 
// We'll have a whole lecture on these later
const char V_SHADER_PATH[] = "shaders/vertex.glsl";     // make sure not to use std::string objects for these!
const char F_SHADER_PATH[] = "shaders/fragment.glsl";



// Old stuff
SDL_Window* display_window;
bool game_is_running = true;

// New stuff
ShaderProgram program;

glm::mat4 view_matrix;        // Defines the position (location and orientation) of the camera
glm::mat4 model_matrix, model_matrix2;       // Defines every translation, rotation, and/or scaling applied to an object; we'll look at these next week
glm::mat4 projection_matrix;  // Defines the characteristics of your camera, such as clip panes, field of view, projection method, etc.

void initialise();
void process_input();
void update();
void render();
void shutdown();

// The game will reside inside the main
int main(int argc, char* argv[])
{
    // Part 1: Initialise our program！whatever that means
    initialise();

    while (game_is_running)
    {
        // Part 2: If the player did anything！press a button, move the joystick！process it
        process_input();

        // Part 3: Using the game's previous state, and whatever new input we have, update the game's state
        // update our model, rotate and translate finish in this part
        update();

        // Part 4: Once updated, render those changes onto the screen
        render();
    }

    // Part 5: The game is over, so let's perform any shutdown protocols
    shutdown();
    return 0;
}

void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
        {
            game_is_running = false;
        }
    }
}

void update() { 
    /* No updates, so this stays empty! */ 
    float scale_factor = 0.05f;
    glm::vec3 scale_vector = glm::vec3(scale_factor, scale_factor, 0.5f);
    glm::vec3 scale_vector2 = glm::vec3(0.0f, 0.0001f, 0.0f);
    model_matrix = glm::rotate(model_matrix, glm::radians(45.0f), scale_vector);
    model_matrix2 = glm::translate(model_matrix2, scale_vector2);
}

void shutdown()
{
    SDL_Quit();
}

void initialise()
{
    /* Old stuff */
    SDL_Init(SDL_INIT_VIDEO);
    display_window = SDL_CreateWindow("Hello, Triangle!",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(display_window);
    SDL_GL_MakeCurrent(display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    /* New stuff */
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_HEIGHT, VIEWPORT_HEIGHT);   // Initialise our camera

    program.Load(V_SHADER_PATH, F_SHADER_PATH);     // Load up our shaders

    // Initialise our view, model, and projection matrices
    view_matrix = glm::mat4(1.0f);
    model_matrix = glm::mat4(1.0f);
    model_matrix2 = glm::mat4(1.0f);
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Orthographic means perpendicular！meaning that our camera will be looking perpendicularly down to our triangle

    program.SetViewMatrix(view_matrix);
    program.SetProjectionMatrix(projection_matrix);
    // Notice we haven't set our model matrix yet!

    program.SetColor(1.0f, 0.44f, 0.38f, 1.0f);
    glUseProgram(program.programID);

    // Old stuff
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
}

void render() {
    // Step 1
    glClear(GL_COLOR_BUFFER_BIT);

    // Step 2
    program.SetModelMatrix(model_matrix);

    // Step 3
    float vertices[] =
    {
        0.5f, -0.5f,
        0.0f, 0.5f,
        -0.5f, -0.5f
    };

    float vertices2[] = 
    {
        3.0f, -3.0f,
        5.0f, -3.0f,
        5.0f, -2.0f,
        3.0f, -2.0f
    };

    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(program.positionAttribute);

    program.SetModelMatrix(model_matrix2);

    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
    glEnableVertexAttribArray(program.positionAttribute);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableVertexAttribArray(program.positionAttribute);

    // Step 4
    SDL_GL_SwapWindow(display_window);
}