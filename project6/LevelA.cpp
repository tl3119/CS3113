#include "LevelA.h"
#include "Utility.h"

#define LEVEL_WIDTH 10
#define LEVEL_HEIGHT 31

GLuint font_texture_iD;

unsigned int LEVELA_DATA[] =
{
   94, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   31, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   31,31, 0, 0, 0, 0, 0, 0, 0, 0,
   31,31, 0, 0, 0, 0, 0, 0, 0, 0,
   31,31, 0, 0, 0, 0, 0, 0, 0, 0,
   31,31, 0, 0, 0, 0, 0, 0, 0, 0,
   31,31, 0, 0, 0, 0, 0, 0, 0, 0,
   31,31, 0, 0, 0, 0, 0, 0, 0, 0,
   31, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   31,31, 0, 0, 0, 0, 0, 0, 0, 0,
   31,31, 0, 0, 0, 0, 0, 0, 0, 0,
   31,31, 0, 0, 0, 0, 0, 0, 0, 0,
   31, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   31,31, 0, 0, 0, 0, 0, 0, 0, 0,
   31,31, 0, 0, 0, 0, 0, 0, 0, 0,
   31,31, 0, 0, 0, 0, 0, 0, 0, 0,
   31,31, 0, 0, 0, 0, 0, 0, 0, 0,
   31,31, 0, 0, 0, 0, 0, 0, 0, 0,
   31,31, 0, 0, 0, 0, 0, 0, 0, 0,
   31,31, 0, 0, 0, 0, 0, 0, 0, 0,
   31,31, 0, 0, 0, 0, 0, 0, 0, 0,
   31,31, 0, 0, 0, 0, 0, 0, 0, 0,
   31,31, 0, 0, 0, 0, 0, 0, 0, 0,
   31,31, 0, 0, 0, 0, 0, 0, 0, 0,
   31,31, 0, 0, 0, 0, 0, 0, 0, 0,
   30,31, 0, 0, 0, 0, 0, 0, 0, 0,
   31,31, 0, 0, 0, 0, 0, 0, 0, 0,
   31,31, 0, 0, 0, 0, 0, 0, 0, 0,
   31,31, 0, 0, 0, 0, 0, 0, 0, 0,
   31,31, 0, 0, 0, 0, 0, 0, 0, 0,
   31,31,15,15,15,15,15,15,94,15
};

LevelA::~LevelA()
{
    delete[] this->state.enemies;
    delete[] this->state.platforms;
    delete    this->state.player;
    delete    this->state.map;
    Mix_FreeChunk(this->state.jump_sfx);
    Mix_FreeMusic(this->state.bgm);
}

void LevelA::initialise()
{
    state.next_scene_id = -1;

    GLuint map_texture_id = Utility::load_texture("sheet.png");
    this->state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELA_DATA, map_texture_id, 1.0f, 14, 7);

    // Code from main.cpp's initialise()
    /**
     George's Stuff
     */
     // Existing
    state.player = new Entity();
    state.player->set_entity_type(PLAYER);
    state.player->set_position(glm::vec3(2.5f, -29.0f, 0.0f));
    state.player->set_movement(glm::vec3(0.0f));
    state.player->speed = 2.0f;
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
    state.player->this_level = level1;
    state.player->collision = NONE;

    // Jumping
    //state.player->jumping_power = 6.5f;

    /**
     Enemies' stuff */
    GLuint enemy_texture_id = Utility::load_texture("ufo_game_enemy.png");

    state.enemies = new Entity[this->ENEMY_COUNT];
    for (int i = 0; i < this->ENEMY_COUNT; ++i) {
        state.enemies[i].set_entity_type(ENEMY);
        state.enemies[i].texture_id = enemy_texture_id;
        state.enemies[i].set_movement(glm::vec3(0.0f));
        state.enemies[i].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
        state.enemies[i].this_level = level1;
        state.enemies[i].set_ai_type(PATROLLER);
        state.enemies[i].set_ai_state(WALKING);
    }
    state.enemies[0].set_ai_type(PATROLLER);
    state.enemies[0].set_ai_state(WALKING);
    state.enemies[0].set_position(glm::vec3(9.0f, -13.0f, 0.0f));
    state.enemies[0].speed = 1.5f;
    state.enemies[1].set_ai_type(NEW_WALKER);
    state.enemies[1].set_ai_state(IDLE);
    state.enemies[1].set_position(glm::vec3(2.5f, -13.0f, 0.0f));
    state.enemies[1].speed = 0.8f;
    

    GLuint platform_texture_id = Utility::load_texture("stoneLarge.png");

    state.platforms = new Entity[this->PLATFORM_COUNT];
    for (int i = 0; i < this->PLATFORM_COUNT; ++i) {
        state.platforms[i].set_entity_type(PLATFORM);
        state.platforms[i].set_ai_state(IDLE);
        state.platforms[i].texture_id = platform_texture_id;
        state.platforms[i].speed = 0.0f;
        state.platforms[i].set_movement(glm::vec3(0.0f));
        state.platforms[i].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
        state.platforms[i].this_level = level1;
    }
    state.platforms[0].set_position(glm::vec3(5.0f, -27.0f, 0.0f));
    state.platforms[1].set_position(glm::vec3(3.5f, -23.0f, 0.0f));
    state.platforms[2].set_position(glm::vec3(6.5f, -21.0f, 0.0f));
    state.platforms[3].set_position(glm::vec3(8.5f, -17.0f, 0.0f));
    state.platforms[4].set_position(glm::vec3(4.5f, -15.0f, 0.0f));
    state.platforms[5].set_position(glm::vec3(5.0f, -12.0f, 0.0f));
    state.platforms[6].set_position(glm::vec3(5.5f, -9.0f, 0.0f));
    state.platforms[7].set_position(glm::vec3(4.0f, -7.0f, 0.0f));
    state.platforms[8].set_position(glm::vec3(3.5f, -5.0f, 0.0f));
    state.platforms[9].set_position(glm::vec3(7.0f, -4.0f, 0.0f));
    state.platforms[10].set_position(glm::vec3(7.0f, -25.0f, 0.0f));
    state.platforms[11].set_position(glm::vec3(5.0f, -22.0f, 0.0f));
    state.platforms[12].set_position(glm::vec3(8.0f, -20.0f, 0.0f));
    state.platforms[13].set_position(glm::vec3(9.0f, -19.0f, 0.0f));
    state.platforms[14].set_position(glm::vec3(6.0f, -16.0f, 0.0f));
    state.platforms[15].set_position(glm::vec3(8.0f, -12.0f, 0.0f));
    state.platforms[16].set_position(glm::vec3(7.5f, -8.0f, 0.0f));
    state.platforms[17].set_position(glm::vec3(2.0f, -6.0f, 0.0f));
    state.platforms[18].set_position(glm::vec3(5.5f, -4.0f, 0.0f));
    state.platforms[19].set_position(glm::vec3(3.0f, -3.0f, 0.0f));
    state.platforms[20].set_position(glm::vec3(2.5f, -10.0f, 0.0f));
    state.platforms[21].set_position(glm::vec3(8.5f, -7.0f, 0.0f));
    state.platforms[22].set_position(glm::vec3(3.0f, -14.0f, 0.0f));

    font_texture_iD = Utility::load_texture("FRONT.png");

    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    state.bgm = Mix_LoadMUS("Pleasant_Porridge.mp3");
    Mix_PlayMusic(state.bgm, -1);
    Mix_VolumeMusic(30.0f);

    state.jump_sfx = Mix_LoadWAV("SFX_Jump_01.wav");
}

void LevelA::update(float delta_time)
{
    this->state.player->update(delta_time, state.player, state.enemies, 2, this->state.map);
    this->state.player->update(delta_time, state.player, state.platforms, this->PLATFORM_COUNT, this->state.map);
    for (int i = 0; i < this->PLATFORM_COUNT; ++i) {
        this->state.platforms[i].update(delta_time, state.player, state.platforms, this->PLATFORM_COUNT, this->state.map);
    }
    for (int i = 0; i < 2; i++) {
        this->state.enemies[i].update(delta_time, state.player, state.enemies, 2, this->state.map);
    }
    
    if (state.enemies[0].collision == ENEMY || state.enemies[1].collision == ENEMY) {
        state.enemies[0].deactivate();
        state.enemies[1].deactivate();
    }

    if (state.player->collision == PLATFORM || state.player->collision == ENEMY || state.player->get_position().y >= 3.0f) {
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
        state.player->set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
        state.player->collision = NONE;
        state.number_of_lives -= 1;
        state.enemies[1].set_position(glm::vec3(2.5f, -13.0f, 0.0f));
        state.enemies[1].set_ai_state(IDLE);
    }
    //&& this->state.player->get_position().y <= -29.0f
    if (this->state.player->get_position().x >= 7.5f) {
        this->state.player->set_acceleration(glm::vec3(0.0f, 13.0f, 0.0f));
    }

    if (this->state.player->get_position().x >= 0.4f && this->state.player->get_position().x <= 0.6f &&
        !state.enemies[0].check_live_status() && this->state.player->get_position().y >= -2.5f &&
        !state.enemies[1].check_live_status() && this->state.player->get_position().y <= 1.8f) {
        state.next_scene_id = 2;
    }
}

void LevelA::render(ShaderProgram* program)
{
    this->state.map->render(program);
    this->state.player->render(program);
    for (int i = 0; i < this->ENEMY_COUNT; ++i) {
        this->state.enemies[i].render(program);
    }
    for (int i = 0; i < this->PLATFORM_COUNT; ++i) {
        this->state.platforms[i].render(program);
    }

    if (state.number_of_lives == 0) {
        Utility::draw_text(program, font_texture_iD, "You Lose", 1.0f, -0.6f, glm::vec3(1.5f, -3.0f, 0));
        Utility::draw_text(program, font_texture_iD, "You Lose", 1.0f, -0.6f, glm::vec3(1.5f, -10.0f, 0));
        Utility::draw_text(program, font_texture_iD, "You Lose", 1.0f, -0.6f, glm::vec3(1.5f, -29.0f, 0));
        Utility::draw_text(program, font_texture_iD, "You Lose", 1.0f, -0.6f, glm::vec3(1.5f, -17.0f, 0));
        state.player->deactivate();
        state.enemies[0].deactivate();
        state.enemies[1].deactivate();
        return;
    }

    Utility::draw_text(program, font_texture_iD, "Your lives now: ", 1.0f, -0.6f, glm::vec3(1.5f, -3.0f, 0));
    Utility::draw_text(program, font_texture_iD, std::to_string(state.number_of_lives), 1.0f, -0.6f, glm::vec3(8.0f, -3.0f, 0));
    Utility::draw_text(program, font_texture_iD, "Your lives now: ", 1.0f, -0.6f, glm::vec3(1.5f, -10.0f, 0));
    Utility::draw_text(program, font_texture_iD, std::to_string(state.number_of_lives), 1.0f, -0.6f, glm::vec3(8.0f, -10.0f, 0));
    Utility::draw_text(program, font_texture_iD, "Your lives now: ", 1.0f, -0.6f, glm::vec3(1.5f, -17.0f, 0));
    Utility::draw_text(program, font_texture_iD, std::to_string(state.number_of_lives), 1.0f, -0.6f, glm::vec3(8.0f, -17.0f, 0));
    Utility::draw_text(program, font_texture_iD, "Your lives now: ", 1.0f, -0.6f, glm::vec3(1.5f, -27.0f, 0));
    Utility::draw_text(program, font_texture_iD, std::to_string(state.number_of_lives), 1.0f, -0.6f, glm::vec3(8.0f, -27.0f, 0));
    Utility::draw_text(program, font_texture_iD, "Hint: Red Button ", 1.0f, -0.6f, glm::vec3(1.5f, -31.0f, 0));
}