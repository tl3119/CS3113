#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>

#define LOG(argument) std::cout << argument << '\n'

const int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;

const float BG_RED = 0.1922f,
BG_BLUE = 0.549f,
BG_GREEN = 0.9059f,
BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;

const char PLAYER_SPRITE_FILEPATH[] = "1.jpg";

const float MINIMUM_COLLISION_DISTANCE = 1.0f;

SDL_Window* display_window;
bool game_is_running = true;
bool is_growing = true;

ShaderProgram program;
glm::mat4 view_matrix, model_matrix, projection_matrix, trans_matrix, other_model_matrix, ball_model_matrix;

float previous_ticks = 0.0f;

GLuint player_texture_id;
GLuint other_texture_id;

glm::vec3 player_position = glm::vec3(4.0f, 0.0f, 0.0f);
glm::vec3 player_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 other_position = glm::vec3(-4.0f, 0.0f, 0.0f);
glm::vec3 other_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 ball_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 ball_movement = glm::vec3(0.0f, 0.0f, 0.0f);

float player_speed = 1.0f;  // move 1 unit per second
float other_speed = 1.0f;
float ball_speed = 2.0f;
float player_width = 0.5f;
float player_height = 1.0f;
float ball_width = 0.4f;
float ball_height = 0.4f;

#define LOG(argument) std::cout << argument << '\n'

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero

SDL_Joystick* playerOneController;

GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);

    return textureID;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

    playerOneController = SDL_JoystickOpen(0);

    display_window = SDL_CreateWindow("Hello, Pong!",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(display_window);
    SDL_GL_MakeCurrent(display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    program.Load(V_SHADER_PATH, F_SHADER_PATH);

    model_matrix = glm::mat4(1.0f);
    other_model_matrix = glm::mat4(1.0f);
    ball_model_matrix = glm::mat4(1.0f);
    other_model_matrix = glm::translate(other_model_matrix, glm::vec3(1.0f, 1.0f, 0.0f));
    other_position += other_movement;

    view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.

    program.SetProjectionMatrix(projection_matrix);
    program.SetViewMatrix(view_matrix);

    glUseProgram(program.programID);

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    player_texture_id = load_texture(PLAYER_SPRITE_FILEPATH);
    other_texture_id = load_texture(PLAYER_SPRITE_FILEPATH);

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    player_movement = glm::vec3(0.0f);
    other_movement = glm::vec3(0.0f);

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            game_is_running = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_w:
                other_movement.y = 1.0f;
                break;

            case SDLK_s:
                // Move the player right
                other_movement.y = -1.0f;
                break;

            case SDLK_q:
                // Quit the game with a keystroke
                game_is_running = false;
                break;
            case SDLK_SPACE:
                // begin the game
                ball_movement.x = 2.0f;
                break;

            default:
                break;
            }

        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_S])
    {
        other_movement.y = -1.0f;
    }
    else if (key_state[SDL_SCANCODE_W])
    {
        other_movement.y = 1.0f;
    }

    if (key_state[SDL_SCANCODE_UP])
    {
        player_movement.y = 1.0f;
    }
    else if (key_state[SDL_SCANCODE_DOWN])
    {
        player_movement.y = -1.0f;
    }
    if (key_state[SDL_SCANCODE_SPACE]) {
        ball_movement.x = 2.0f;
    }

    // This makes sure that the player can't move faster diagonally
    if (glm::length(player_movement) > 1.0f)
    {
        player_movement = glm::normalize(player_movement);
    }

    if (glm::length(other_movement) > 1.0f)
    {
        other_movement = glm::normalize(other_movement);
    }
}

/**
 Uses distance formula.
 */
bool check_collision()
{

    if (ball_position.y <= -3.5 || ball_position.y >= 3.5) {
        ball_movement.y *= -1;
    }

    if (ball_position.x <= -5.0 || ball_position.x >= 4.8) {
        // Game stops
        ball_movement.x = 0;
        ball_movement.y = 0;
        //game_is_running = false;
    }

    // x and y distance for the first player 
    float diff_x_one = fabs(player_position.x - ball_position.x) - ((player_width + ball_width) / 2);
    float diff_y_one = fabs(player_position.y - ball_position.y) - ((player_height + ball_height) / 2);
    // x and y distance for other player
    float diff_x_two = fabs(other_position.x - ball_position.x) - ((player_width + ball_width) / 2);
    float diff_y_two = fabs(other_position.y - ball_position.y) - ((player_height + ball_height) / 2);

    // collision 
    if (diff_x_one < 0 && diff_y_one < 0) {
        ball_movement.x *= -1;
        ball_movement.y = (player_position.y - ball_position.y) * (-1);
        return true;
    }

    // collision 
    if (diff_x_two < 0 && diff_y_two < 0) {
        ball_movement.x *= -1;
        ball_movement.y = (other_position.y - ball_position.y) * (-1);
        return true;
    }
    return false;

}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - previous_ticks; // the delta time is the difference from the last frame
    previous_ticks = ticks;

    ball_model_matrix = glm::mat4(1.0f);

    if (player_movement.y >= 0 && player_position.y <= 3.3) {
        player_position += player_movement * player_speed * delta_time;
        model_matrix = glm::mat4(1.0f);
        model_matrix = glm::translate(model_matrix, player_position);
    }
    if (player_movement.y < 0 && player_position.y >= -3.3) {
        player_position += player_movement * player_speed * delta_time;
        model_matrix = glm::mat4(1.0f);
        model_matrix = glm::translate(model_matrix, player_position);
    }

    if (other_movement.y >= 0 && other_position.y <= 3.3) {
        other_position += other_movement * other_speed * delta_time;
        other_model_matrix = glm::mat4(1.0f);
        other_model_matrix = glm::translate(other_model_matrix, other_position);
    }
    if (other_movement.y < 0 && other_position.y >= -3.3) {
        other_position += other_movement * other_speed * delta_time;
        other_model_matrix = glm::mat4(1.0f);
        other_model_matrix = glm::translate(other_model_matrix, other_position);
    }

    if(check_collision()){}
    // Add direction * units per second * elapsed time
    ball_position += ball_movement * ball_speed * delta_time;
    ball_model_matrix = glm::translate(ball_model_matrix, ball_position);
}

void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id)
{
    program.SetModelMatrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Vertices
    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };

    float ball_vertices[] = { 
        -0.1f, -0.1f, 0.3f, -0.1f, 0.3f, 0.3f, 
        -0.1f, -0.1f, 0.3f, 0.3f, -0.1f, 0.3f 
    };

    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);

    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(program.texCoordAttribute);
    
    // Bind texture
    draw_object(model_matrix, player_texture_id);
    draw_object(other_model_matrix, other_texture_id);
    
    // We disable two attribute arrays now
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);

    // Set ball matrix
    program.SetModelMatrix(ball_model_matrix);
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, ball_vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(program.positionAttribute);

    SDL_GL_SwapWindow(display_window);
}

void shutdown() { 
    SDL_Quit();
    SDL_JoystickClose(playerOneController);
}


int main(int argc, char* argv[])
{
    initialise();

    while (game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}
