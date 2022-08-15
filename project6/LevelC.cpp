#include "LevelC.h"
#include "Utility.h"

#define LEVEL_WIDTH 10
#define LEVEL_HEIGHT 31

GLuint font_textureiD;

unsigned int LEVELC_DATA[] =
{
   30,80, 0, 0, 0, 0, 0, 0, 0, 0,
   30, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   30,30,48,48,48, 0, 0, 0, 0, 0,
   30,30, 0, 0, 0, 0,48, 0, 0, 0,
   30,30, 0, 0, 0, 0, 0, 0, 0, 0,
   30,30, 0, 0, 0,48, 0, 0, 0, 0,
   30,30, 0, 0, 0, 0, 0, 0, 0, 0,
   30,30, 0, 0,48, 0, 0, 0, 0, 0,
   30, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   30,30, 0,48, 0, 0, 0, 0, 0, 0,
   30,30, 0, 0, 0,48, 0, 0, 0, 0,
   30,30, 0, 0, 0, 0, 0, 0, 0, 0,
   30, 0, 0, 0,48, 0, 0, 0, 0, 0,
   30,30, 0, 0, 0, 0, 0, 0, 0, 0,
   30,30, 0,48, 0, 0, 0, 0, 0, 0,
   30, 0, 0, 0, 0,48, 0, 0, 0, 0,
   30, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   30,30,48,48,48, 0, 0, 0, 0, 0,
   30,30, 0, 0, 0, 0,48, 0, 0, 0,
   30,30, 0, 0, 0, 0, 0, 0, 0, 0,
   30,30, 0, 0, 0,48, 0, 0, 0, 0,
   30,30, 0, 0, 0, 0, 0, 0, 0, 0,
   30,30, 0, 0,48, 0, 0, 0, 0, 0,
   30, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   30,30, 0,48, 0, 0, 0, 0, 0, 0,
   30,30, 0, 0, 0,48, 0, 0, 0, 0,
   30,30, 0, 0, 0, 0, 0,48, 0, 0,
   30, 0, 0, 0, 48, 0, 0, 0, 0, 0,
   30,30, 0, 0, 0, 0, 0, 0, 0, 0,
   30,30, 0,48, 0, 0, 0, 0, 0, 0,
   30,30, 9,10, 8, 9,10, 8, 9,10
};

LevelC::~LevelC()
{
    delete[] this->state.enemies;
    delete    this->state.player;
    delete    this->state.map;
    Mix_FreeChunk(this->state.jump_sfx);
    Mix_FreeMusic(this->state.bgm);
}

void LevelC::initialise()
{
    state.next_scene_id = -1;

    GLuint map_texture_id = Utility::load_texture("sheet.png");
    this->state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELC_DATA, map_texture_id, 1.0f, 14, 7);

    // Code from main.cpp's initialise()
    /**
     George's Stuff
     */
     // Existing
    state.player = new Entity();
    state.player->set_entity_type(PLAYER);
    state.player->set_position(glm::vec3(2.8f, -25.0f, 0.0f));
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
    state.player->this_level = level3;

    // Jumping
    state.player->jumping_power = 5.0f;

    /**
     Enemies' stuff */
    GLuint enemy_texture_id = Utility::load_texture("ufo_game_enemy.png");

    state.enemies = new Entity[this->ENEMY_COUNT];
    for (int i = 0; i < this->ENEMY_COUNT; ++i) {
        state.enemies[i].set_entity_type(ENEMY);
        state.enemies[i].texture_id = enemy_texture_id;
        state.enemies[i].set_movement(glm::vec3(0.0f));
        state.enemies[i].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
        state.enemies[i].this_level = level3;
        state.enemies[i].set_ai_type(PATROLLER);
        state.enemies[i].set_ai_state(WALKING);
        state.enemies[i].speed = 0.4f;
    }
    state.enemies[0].set_position(glm::vec3(3.5f, -15.0f, 0.0f));
    state.enemies[1].set_position(glm::vec3(3.5f, 5.0f, 0.0f));

    font_textureiD = Utility::load_texture("FRONT.png");

    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    state.bgm = Mix_LoadMUS("Pleasant_Porridge.mp3");
    Mix_PlayMusic(state.bgm, -1);
    Mix_VolumeMusic(30.0f);

    state.jump_sfx = Mix_LoadWAV("SFX_Jump_01.wav");
}

void LevelC::update(float delta_time) {
    this->state.player->update(delta_time, state.player, state.enemies, this->ENEMY_COUNT, this->state.map);
    for (int i = 0; i < 2; i++) {
        this->state.enemies[i].update(delta_time, state.player, state.enemies, this->ENEMY_COUNT, this->state.map);
        this->state.enemies[i].update(delta_time, state.enemies, state.player, 1, this->state.map);
    }

    if ((state.player->collision == ENEMY && (state.player->collided_left || state.player->collided_right || state.player->collided_top))
        || (state.player->get_position().y < -29.0f) || (state.enemies[0].collision == PLAYER && (state.enemies[0].collided_left || state.enemies[0].collided_right || state.enemies[0].collided_bottom))
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
        state.player->set_position(glm::vec3(2.7f, -25.0f, 0.0f));
        state.player->collision = PLATFORM;
        state.enemies[0].collision = NONE;
        state.enemies[1].collision = NONE;
        state.number_of_lives -= 1;
        //state.enemies[0].set_position(glm::vec3(10.0f, 2.0f, 0.0f));
    }

    if (this->state.player->get_position().x >= 1.7f && this->state.player->get_position().x <= 2.2f && 
        !state.enemies[0].check_live_status() && !state.enemies[1].check_live_status() &&
        this->state.player->get_position().y >= -3.9f) {
        state.next_scene_id = 5;
    }
}

void LevelC::render(ShaderProgram* program)
{
    this->state.map->render(program);
    this->state.player->render(program);
    for (int i = 0; i < this->ENEMY_COUNT; ++i) {
        this->state.enemies[i].render(program);
    }

    if (state.number_of_lives == 0) {
        Utility::draw_text(program, font_textureiD, "You Lose", 1.0f, -0.6f, glm::vec3(1.5f, -3.0f, 0));
        Utility::draw_text(program, font_textureiD, "You Lose", 1.0f, -0.6f, glm::vec3(1.5f, -10.0f, 0));
        Utility::draw_text(program, font_textureiD, "You Lose", 1.0f, -0.6f, glm::vec3(1.5f, -25.0f, 0));
        Utility::draw_text(program, font_textureiD, "You Lose", 1.0f, -0.6f, glm::vec3(1.5f, -17.0f, 0));
        state.player->deactivate();
        state.enemies[0].deactivate();
        state.enemies[1].deactivate();
        return;
    }

    Utility::draw_text(program, font_textureiD, "Your lives now: ", 1.0f, -0.6f, glm::vec3(1.5f, -3.0f, 0));
    Utility::draw_text(program, font_textureiD, std::to_string(state.number_of_lives), 1.0f, -0.6f, glm::vec3(8.0f, -3.0f, 0));
    Utility::draw_text(program, font_textureiD, "Your lives now: ", 1.0f, -0.6f, glm::vec3(1.5f, -10.0f, 0));
    Utility::draw_text(program, font_textureiD, std::to_string(state.number_of_lives), 1.0f, -0.6f, glm::vec3(8.0f, -10.0f, 0));
    Utility::draw_text(program, font_textureiD, "Your lives now: ", 1.0f, -0.6f, glm::vec3(1.5f, -17.0f, 0));
    Utility::draw_text(program, font_textureiD, std::to_string(state.number_of_lives), 1.0f, -0.6f, glm::vec3(8.0f, -17.0f, 0));
    Utility::draw_text(program, font_textureiD, "Your lives now: ", 1.0f, -0.6f, glm::vec3(1.5f, -25.0f, 0));
    Utility::draw_text(program, font_textureiD, std::to_string(state.number_of_lives), 1.0f, -0.6f, glm::vec3(8.0f, -25.0f, 0));
}