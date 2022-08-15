#include "LevelE.h"
#include "Utility.h"
#include <ctime>

#define LEVEL_WIDTH 10
#define LEVEL_HEIGHT 31

GLuint font_texture_id5;

unsigned int LEVELE_DATA[] =
{
   0, 0, 0, 0, 0, 0, 0, 0,31,31,
   31, 0, 0, 0, 0, 0, 0, 0,31,31,
   31, 0, 0, 0, 0, 0, 0, 0,31,31,
   31,31,31, 0, 0, 0,31, 0,31,31,
   31,31,31, 0,31, 0, 0, 0,31,31,
   31,31,31, 0, 0,31,31,31,31,31,
   31,31,31,31, 0,31, 0, 0, 0,31,
   31,31, 0, 0, 0,31, 0,31, 0,31,
   31,31, 0,31,31, 0, 0,31, 0,31,
   31,31, 0, 0, 0, 0,31,31, 0,31,
   31,31,31,31,31, 0, 0, 0, 0,31,
   31,31,31,31,31, 0,31,31,31,31,
   31,31,31,31,31, 0,31,31,31,31,
   31,31,31,31,31, 0,31,31,31,31,
   31,31, 0, 0, 0, 0, 0, 0,31,31,
   31,31, 0,31,31,31,31,31,31,31,
   31, 0, 0, 0, 0, 0, 0, 0, 0,31,
   31,31, 0, 0, 0, 0, 0, 0,31,31,
   31,31, 0, 0, 0, 0,31,31,31,31,
   31,31, 0,31,31,31,31,31,31,31,
   31,31, 0, 0,31,31,31,31,31,31,
   31,31,31, 0,31,31,31,31,31,31,
   31,31,31, 0, 0, 0, 0, 0, 0,31,
   31,31,31,31,31,31,31,31, 0,31,
   31,31,31,31, 0, 0, 0, 0, 0,31,
   31,31,31, 0, 0,31,31,31,31,31,
   31,31,31, 0,31,31,31,31,31,31,
   31,31, 0, 0, 0, 0, 0, 0, 0,31,
   31,31,31,31,31,31,31,31, 0,31,
   31,31, 0, 0, 0, 0, 0, 0, 0,31,
   31,31,31,31,31,31,31,31,31,31
};

LevelE::~LevelE()
{
    delete[] this->state.enemies;
    delete    this->state.player;
    delete    this->state.map;
    Mix_FreeChunk(this->state.jump_sfx);
    Mix_FreeMusic(this->state.bgm);
}

void LevelE::initialise()
{
    state.next_scene_id = -1;

    GLuint map_texture_id = Utility::load_texture("sheet.png");
    this->state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELE_DATA, map_texture_id, 1.0f, 14, 7);

    // Code from main.cpp's initialise()
    /**
     George's Stuff
     */
     // Existing
    state.player = new Entity();
    state.player->set_entity_type(PLAYER);
    state.player->set_position(glm::vec3(2.5f, -29.0f, 0.0f));
    state.player->set_movement(glm::vec3(0.0f));
    state.player->speed = 2.5f;
    state.player->set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
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
    state.player->this_level = level5;
    state.player->collision = NONE;

    clock_t now = clock();

    // Jumping
    //state.player->jumping_power = 5.0f;

    /**
     Enemies' stuff*/
    GLuint enemy_texture_id = Utility::load_texture("ufo_game_enemy.png");

    state.enemies = new Entity[this->ENEMY_COUNT];
    for (int i = 0; i < this->ENEMY_COUNT; ++i) {
        state.enemies[i].set_entity_type(ENEMY);
        state.enemies[i].texture_id = enemy_texture_id;
        state.enemies[i].set_movement(glm::vec3(0.0f));
        state.enemies[i].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
        state.enemies[i].this_level = level5;
        state.enemies[i].set_ai_type(PATROLLER);
        state.enemies[i].set_ai_state(WALKING);
        state.enemies[i].speed = 0.2f;
    }
    state.enemies[0].set_position(glm::vec3(3.5f, -16.0f, 0.0f));
    state.enemies[1].set_position(glm::vec3(3.5f, -1.0f, 0.0f));

    font_texture_id5 = Utility::load_texture("FRONT.png");

    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    state.bgm = Mix_LoadMUS("Pleasant_Porridge.mp3");
    Mix_PlayMusic(state.bgm, -1);
    Mix_VolumeMusic(30.0f);

    state.jump_sfx = Mix_LoadWAV("SFX_Jump_01.wav");
}

void LevelE::update(float delta_time) {
    this->state.player->update(delta_time, state.player, state.enemies, this->ENEMY_COUNT, this->state.map);
    for (int i = 0; i < 2; i++) {
        this->state.enemies[i].update(delta_time, state.player, state.enemies, this->ENEMY_COUNT, this->state.map);
    }

    if ((state.player->collision == ENEMY && (state.player->collided_left || state.player->collided_right || state.player->collided_top))
        || (state.enemies[0].collision == PLAYER && (state.enemies[0].collided_left || state.enemies[0].collided_right || state.enemies[0].collided_bottom))
        || (state.enemies[1].collision == PLAYER && (state.enemies[1].collided_left || state.enemies[1].collided_right || state.enemies[1].collided_bottom))) {
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
        state.player->set_position(glm::vec3(2.5f, -29.0f, 0.0f));
        state.player->collision = NONE;
        state.enemies[0].collision = NONE;
        state.enemies[1].collision = NONE;
        state.number_of_lives -= 1;
        //state.enemies[0].set_position(glm::vec3(10.0f, 2.0f, 0.0f));
    }

}

void LevelE::render(ShaderProgram* program)
{
    this->state.map->render(program);
    this->state.player->render(program);
    for (int i = 0; i < this->ENEMY_COUNT; ++i) {
        this->state.enemies[i].render(program);
    }

    if (state.number_of_lives == 0) {
        Utility::draw_text(program, font_texture_id5, "You Lose", 1.0f, -0.6f, glm::vec3(1.5f, -3.0f, 0));
        Utility::draw_text(program, font_texture_id5, "You Lose", 1.0f, -0.6f, glm::vec3(1.5f, -10.0f, 0));
        Utility::draw_text(program, font_texture_id5, "You Lose", 1.0f, -0.6f, glm::vec3(1.5f, -28.0f, 0));
        Utility::draw_text(program, font_texture_id5, "You Lose", 1.0f, -0.6f, glm::vec3(1.5f, -17.0f, 0));
        state.player->deactivate();
        state.enemies[0].deactivate();
        state.enemies[1].deactivate();
        return;
    }

    if (!state.enemies[0].check_live_status() && !state.enemies[1].check_live_status()) {
        Utility::draw_text(program, font_texture_id5, "You Win", 1.0f, -0.6f, glm::vec3(3.5f, 0.0f, 0));
        return;
    }

    Utility::draw_text(program, font_texture_id5, "Your lives now: ", 1.0f, -0.6f, glm::vec3(1.5f, -3.0f, 0));
    Utility::draw_text(program, font_texture_id5, std::to_string(state.number_of_lives), 1.0f, -0.6f, glm::vec3(8.0f, -3.0f, 0));
    Utility::draw_text(program, font_texture_id5, "Your lives now: ", 1.0f, -0.6f, glm::vec3(1.5f, -10.0f, 0));
    Utility::draw_text(program, font_texture_id5, std::to_string(state.number_of_lives), 1.0f, -0.6f, glm::vec3(8.0f, -10.0f, 0));
    Utility::draw_text(program, font_texture_id5, "Your lives now: ", 1.0f, -0.6f, glm::vec3(1.5f, -17.0f, 0));
    Utility::draw_text(program, font_texture_id5, std::to_string(state.number_of_lives), 1.0f, -0.6f, glm::vec3(8.0f, -17.0f, 0));
    Utility::draw_text(program, font_texture_id5, "Your lives now: ", 1.0f, -0.6f, glm::vec3(1.5f, -25.0f, 0));
    Utility::draw_text(program, font_texture_id5, std::to_string(state.number_of_lives), 1.0f, -0.6f, glm::vec3(8.0f, -25.0f, 0));
}