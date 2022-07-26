#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define PLATFORM_COUNT 14
#define ENEMY_COUNT 3
#define PLATFORM_COUNT_2 11

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"

/**
 STRUCTS AND ENUMS
 */
struct GameState
{
    Entity* player;
    Entity* platforms;
    Entity* enemies;

    Mix_Music* bgm;
    Mix_Chunk* jump_sfx;
};

/**
 CONSTANTS
 */
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
const char SPRITESHEET_FILEPATH[] = "betty.png";
const char PLATFORM_FILEPATH[] = "brown_platform.png";

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero

const float PLATFORM_OFFSET = 5.0f;
const int FONTBANK_SIZE = 16;
bool losegame = false;

/**
 VARIABLES
 */
GameState state;

SDL_Window* display_window;
bool game_is_running = true;

ShaderProgram program;
glm::mat4 view_matrix, projection_matrix;

float previous_ticks = 0.0f;
float accumulator = 0.0f;
GLuint the_texture_id;

/**
 GENERAL FUNCTIONS
 */
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

    // STEP 3: Setting our texture filter modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // STEP 4: Setting our texture wrapping modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // the last argument can change depending on what you are looking for
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // STEP 5: Releasing our file from memory and returning our texture id
    stbi_image_free(image);

    return textureID;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    display_window = SDL_CreateWindow("Hello, AI!",
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

    view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.

    program.SetProjectionMatrix(projection_matrix);
    program.SetViewMatrix(view_matrix);

    glUseProgram(program.programID);

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    the_texture_id = load_texture("FRONT.png");
    /**
     Platform stuff
     */
    GLuint platform_texture_id = load_texture(PLATFORM_FILEPATH);

    state.platforms = new Entity[PLATFORM_COUNT];

    for (int i = 0; i < PLATFORM_COUNT_2; i++)
    {
        state.platforms[i].set_entity_type(PLATFORM);
        state.platforms[i].texture_id = platform_texture_id;
        state.platforms[i].set_position(glm::vec3(i - PLATFORM_OFFSET, -3.0f, 0.0f));
        state.platforms[i].set_width(0.4f);
        state.platforms[i].update(0.0f, NULL, NULL, 0);
    }

    for (int i = 11; i < PLATFORM_COUNT; i++) {
        state.platforms[i].set_entity_type(PLATFORM);
        state.platforms[i].texture_id = platform_texture_id;
        state.platforms[i].set_position(glm::vec3(i - 9.0f, 2.0f, 0.0f));
        state.platforms[i].set_width(0.4f);
        state.platforms[i].update(0.0f, NULL, NULL, 0);
    }

    /**
     George's stuff
     */
     // Existing
    state.player = new Entity();
    state.player->set_entity_type(PLAYER);
    state.player->set_position(glm::vec3(-2.0f, 0.0f, 0.0f));
    state.player->set_movement(glm::vec3(0.0f));
    state.player->speed = 2.5f;
    state.player->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    state.player->texture_id = load_texture(SPRITESHEET_FILEPATH);

    // Walking
    state.player->walking[state.player->LEFT] = new int[4]{ 1, 5, 9,  13 };
    state.player->walking[state.player->RIGHT] = new int[4]{ 3, 7, 11, 15 };
    state.player->walking[state.player->UP] = new int[4]{ 2, 6, 10, 14 };
    state.player->walking[state.player->DOWN] = new int[4]{ 0, 4, 8,  12 };

    state.player->animation_indices = state.player->walking[state.player->RIGHT];  // start George looking left
    state.player->animation_frames = 4;
    state.player->animation_index = 0;
    state.player->animation_time = 0.0f;
    state.player->animation_cols = 4;
    state.player->animation_rows = 4;
    state.player->set_height(0.9f);
    state.player->set_width(0.9f);

    // Jumping
    state.player->jumping_power = 7.0f;

    /**
     Enemies' stuff
     */
    GLuint enemy_texture_id = load_texture("ufo_game_enemy.png");

    state.enemies = new Entity[ENEMY_COUNT];
    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        state.enemies[i].set_entity_type(ENEMY);
        state.enemies[i].texture_id = enemy_texture_id;
        state.enemies[i].set_movement(glm::vec3(0.0f));
        state.enemies[i].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    }
    state.enemies[0].set_position(glm::vec3(2.0f, -3.0f, 0.0f));
    state.enemies[1].set_position(glm::vec3(3.5f, -3.0f, 0.0f));
    state.enemies[2].set_position(glm::vec3(2.5f, 2.0f, 0.0f));

    state.enemies[0].set_ai_state(IDLE);
    state.enemies[1].set_ai_state(WALKING);
    state.enemies[2].set_ai_state(IDLE);

    state.enemies[0].speed = 0.5f;
    state.enemies[1].speed = 0.2f;
    state.enemies[2].speed = 1.0f;

    state.enemies[0].set_ai_type(WALKER);
    state.enemies[1].set_ai_type(PATROLLER);
    state.enemies[2].set_ai_type(GUARD);

    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    state.bgm = Mix_LoadMUS("Pleasant_Porridge.mp3");
    Mix_PlayMusic(state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 4.0f);

    state.jump_sfx = Mix_LoadWAV("SFX_Jump_01.wav");

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    state.player->set_movement(glm::vec3(0.0f));

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
            case SDLK_q:
                // Quit the game with a keystroke
                game_is_running = false;
                break;

            case SDLK_SPACE:
                // Jump
                if (state.player->collided_bottom)
                {
                    state.player->is_jumping = true;
                    Mix_PlayChannel(-1, state.jump_sfx, 0);
                }
                break;

            default:
                break;
            }

        default:
            break;
        }
    }
    
    //
    //如果walk 走过去的情况怎么办，没有defeat walk，也应该输掉? (现在是算赢）
    //GUARD如果啥都没动碰到的话，会自动吃掉
    if (state.player->collision == ENEMY && (state.player->collided_left || state.player->collided_right || state.player->collided_top)) {
        state.player->set_movement(glm::vec3(0.0f));
        state.player->set_velocity(glm::vec3(0.0f));
        state.player->set_acceleration(glm::vec3(0.0f));
        state.player->speed = 0.0f;
        for (int i = 0; i < ENEMY_COUNT; i++)
        {
            state.enemies[i].set_movement(glm::vec3(0.0f));
            state.enemies[i].set_velocity(glm::vec3(0.0f));
            state.enemies[i].set_acceleration(glm::vec3(0.0f));
        }
        losegame = true;
        return;
    }
    if (state.player->collision == ENEMY && state.player->collided_bottom == true) {
        (state.player->collision_enemy)->deactivate();
        return;
    }

    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        if (state.enemies[i].collision == PLAYER && (state.enemies[i].collided_left || state.enemies[i].collided_right || state.enemies[i].collided_bottom)) {
            state.player->set_movement(glm::vec3(0.0f));
            state.player->set_velocity(glm::vec3(0.0f));
            state.player->set_acceleration(glm::vec3(0.0f));
            state.player->speed = 0.0f;
            state.enemies[i].set_movement(glm::vec3(0.0f));
            state.enemies[i].set_velocity(glm::vec3(0.0f));
            state.enemies[i].set_acceleration(glm::vec3(0.0f));
            losegame = true;
            return;
        }
    }

    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        if (state.enemies[i].get_position().x < -5.0 || state.enemies[i].get_position().x > 5.0) {
            state.enemies[i].deactivate();
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT])
    {
        state.player->movement.x = -1.0f;
        state.player->animation_indices = state.player->walking[state.player->LEFT];
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        state.player->movement.x = 1.0f;
        state.player->animation_indices = state.player->walking[state.player->RIGHT];
    }

    // This makes sure that the player can't move faster diagonally
    if (glm::length(state.player->movement) > 1.0f)
    {
        state.player->movement = glm::normalize(state.player->movement);
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - previous_ticks;
    previous_ticks = ticks;

    delta_time += accumulator;

    if (delta_time < FIXED_TIMESTEP)
    {
        accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP) {
        // Update. Notice it's FIXED_TIMESTEP. Not deltaTime
        state.player->update(FIXED_TIMESTEP, state.player, state.platforms, PLATFORM_COUNT);
        state.player->update(FIXED_TIMESTEP, state.player, state.enemies, ENEMY_COUNT);

        for (int i = 0; i < ENEMY_COUNT; i++) state.enemies[i].update(FIXED_TIMESTEP, state.player, state.platforms, PLATFORM_COUNT);
        for (int i = 0; i < ENEMY_COUNT; i++) {
            if (state.enemies[i].get_ai_type() != GUARD) {
                state.enemies[i].update(FIXED_TIMESTEP, state.enemies, state.player, 1);
            }
        }
        //to do: make sure enemies can not move

        delta_time -= FIXED_TIMESTEP;
    }

    accumulator = delta_time;
}

void DrawText(ShaderProgram* program, GLuint font_texture_id, std::string text, float screen_size, float spacing, glm::vec3 position)
{
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;

    // Instead of having a single pair of arrays, we'll have a series of pairs—one for each character
    // Don't forget to include <vector>!
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    // For every character...
    for (int i = 0; i < text.size(); i++) {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their position
        //    relative to the whole sentence)
        int spritesheet_index = (int)text[i];  // ascii value of character
        float offset = (screen_size + spacing) * i;

        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float)(spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float)(spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
            offset + (-0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
            });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
            });
    }

    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);

    program->SetModelMatrix(model_matrix);
    glUseProgram(program->programID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(program->texCoordAttribute);

    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}


void render()
{
    bool check_status = false;
    glClear(GL_COLOR_BUFFER_BIT);

    state.player->render(&program);

    for (int i = 0; i < PLATFORM_COUNT; i++) state.platforms[i].render(&program);
    for (int i = 0; i < ENEMY_COUNT; i++) {
        state.enemies[i].render(&program);
        if (state.enemies[i].get_active()) {
            check_status = true;
        }
    }
    if (losegame) {
        // lose
        DrawText(&program, the_texture_id, "You Lose", 1.0f, -0.6f, glm::vec3(-3.5, 0.0, 0));
    }
    else if (check_status == false) {
        DrawText(&program, the_texture_id, "You Win", 1.0f, -0.6f, glm::vec3(-3.5, 0.0, 0));
    }

    SDL_GL_SwapWindow(display_window);
}

void shutdown()
{
    SDL_Quit();

    delete[] state.platforms;
    delete[] state.enemies;
    delete    state.player;
    Mix_FreeChunk(state.jump_sfx);
    Mix_FreeMusic(state.bgm);
}

/**
 DRIVER GAME LOOP
 */
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