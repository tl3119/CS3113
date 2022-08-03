#include "LevelB.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

GLuint font_texture_idb;

unsigned int LEVELB_DATA[] =
{
    27, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    27, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    27, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 16,
    27, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 16, 16,
    27, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 16, 16, 16,
    27, 0, 0, 0, 0, 0, 0, 0, 0, 1, 16, 16, 16, 16,
    27, 50, 50, 50, 50, 50, 50, 0, 1, 16, 16, 16, 16, 16,
    27, 10, 10, 10, 10, 10, 10, 0, 77, 78, 79, 80, 81, 82
};

LevelB::~LevelB()
{
    delete[] this->state.enemies;
    delete    this->state.player;
    delete    this->state.map;
    Mix_FreeChunk(this->state.jump_sfx);
    Mix_FreeMusic(this->state.bgm);
}

void LevelB::initialise()
{
    state.next_scene_id = -1;

    GLuint map_texture_id = Utility::load_texture("sheet.png");
    this->state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELB_DATA, map_texture_id, 1.0f, 14, 7);

    // Code from main.cpp's initialise()
    /**
     George's Stuff
     */
     // Existing
    state.player = new Entity();
    state.player->set_entity_type(PLAYER);
    state.player->set_position(glm::vec3(5.0f, 0.0f, 0.0f));
    state.player->set_movement(glm::vec3(0.0f));
    state.player->speed = 2.5f;
    state.player->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    state.player->texture_id = Utility::load_texture("betty.png");

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
    state.player->set_height(0.8f);
    state.player->set_width(0.8f);

    // Jumping
    state.player->jumping_power = 5.0f;

    /**
     Enemies' stuff */
    GLuint enemy_texture_id = Utility::load_texture("ufo_game_enemy.png");

    state.enemies = new Entity[this->ENEMY_COUNT];
    state.enemies[0].set_entity_type(ENEMY);
    state.enemies[0].set_ai_type(GUARD);
    state.enemies[0].set_ai_state(IDLE);
    state.enemies[0].texture_id = enemy_texture_id;
    state.enemies[0].set_position(glm::vec3(8.0f, 0.0f, 0.0f));
    state.enemies[0].set_movement(glm::vec3(0.0f));
    state.enemies[0].speed = 1.0f;
    state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));

    font_texture_idb = Utility::load_texture("FRONT.png");

    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    state.bgm = Mix_LoadMUS("Pleasant_Porridge.mp3");
    Mix_PlayMusic(state.bgm, -1);
    Mix_VolumeMusic(10.0f);

    state.jump_sfx = Mix_LoadWAV("SFX_Jump_01.wav");
}

void LevelB::update(float delta_time) { 
    this->state.player->update(delta_time, state.player, state.enemies, this->ENEMY_COUNT, this->state.map); 
    for (int i = 0; i < 1; i++) {
        this->state.enemies[i].update(delta_time, state.player, state.enemies, this->ENEMY_COUNT, this->state.map);
    }
    if (state.player->collision == ENEMY && (state.player->collided_left || state.player->collided_right || state.player->collided_top)) {
        if (state.number_of_lives == 0) {
            state.player->set_movement(glm::vec3(0.0f));
            state.player->set_velocity(glm::vec3(0.0f));
            state.player->set_acceleration(glm::vec3(0.0f));
            state.player->speed = 0.0f;
            for (int i = 0; i < 1; i++)
            {
                state.enemies[i].set_movement(glm::vec3(0.0f));
                state.enemies[i].set_velocity(glm::vec3(0.0f));
                state.enemies[i].set_acceleration(glm::vec3(0.0f));
            }
            return;
        }
        state.player->set_position(glm::vec3(5.0f, 0.0f, 0.0f));
        state.player->collision = PLATFORM;
        state.number_of_lives -= 1;
        state.enemies[0].set_position(glm::vec3(10.0f, 2.0f, 0.0f));
    }

    if (this->state.player->get_position().y < -10.0f) state.next_scene_id = 3;
}

void LevelB::render(ShaderProgram* program)
{
    this->state.map->render(program);
    this->state.player->render(program);
    this->state.enemies->render(program);

    if (state.number_of_lives == 0) {
        Utility::draw_text(program, font_texture_idb, "You Lose", 1.0f, -0.6f, glm::vec3(1.5f, -3.0f, 0));
        state.player->deactivate();
        state.enemies[0].deactivate();
        return;
    }

    Utility::draw_text(program, font_texture_idb, "Your lives now: ", 1.0f, -0.6f, glm::vec3(1.5f, -3.0f, 0));
    Utility::draw_text(program, font_texture_idb, std::to_string(state.number_of_lives), 1.0f, -0.6f, glm::vec3(8.0f, -3.0f, 0));
}