#include "LevelD.h"
#include "Utility.h"

#define LEVEL_WIDTH 10
#define LEVEL_HEIGHT 8

GLuint font_texture_id4;

unsigned int LEVELD_DATA[] =
{
   0, 0, 0, 0, 0, 0, 0, 0, 0,94,
   0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
   0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
   0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
   0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
   0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
   0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

LevelD::~LevelD()
{
    delete[] this->state.enemies;
    delete    this->state.player;
    delete    this->state.map;
    Mix_FreeChunk(this->state.jump_sfx);
    Mix_FreeMusic(this->state.bgm);
}

void LevelD::initialise()
{
    state.next_scene_id = -1;

    GLuint map_texture_id = Utility::load_texture("sheet.png");
    this->state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELD_DATA, map_texture_id, 1.0f, 14, 7);

    // Code from main.cpp's initialise()
    /**
     George's Stuff
     */
     // Existing
    state.player = new Entity();
    state.player->set_entity_type(PLAYER);
    state.player->set_position(glm::vec3(2.0f, 0.0f, 0.0f));
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
    state.player->this_level = level4;

    // Jumping
    state.player->jumping_power = 5.0f;

    font_texture_id4 = Utility::load_texture("FRONT.png");

    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    state.bgm = Mix_LoadMUS("Pleasant_Porridge.mp3");
    Mix_PlayMusic(state.bgm, -1);
    Mix_VolumeMusic(30.0f);

    state.jump_sfx = Mix_LoadWAV("SFX_Jump_01.wav");
}

void LevelD::update(float delta_time) {
    this->state.player->update(delta_time, state.player, state.enemies, this->ENEMY_COUNT, this->state.map);
    if (this->state.player->get_position().x >= 9.0f && this->state.player->get_position().x <= 9.5f 
        && this->state.player->get_position().y <= 1.0f && this->state.player->get_position().y >= 0.0f) {
        state.next_scene_id = 2;
    }
}

void LevelD::render(ShaderProgram* program)
{
    this->state.map->render(program);
    this->state.player->render(program);

    Utility::draw_text(program, font_texture_id4, "WOW!!! ", 1.0f, -0.4f, glm::vec3(1.5f, -1.0f, 0));
    Utility::draw_text(program, font_texture_id4, "You find a secret place!", 1.0f, -0.7f, glm::vec3(0.8f, -3.0f, 0));
}