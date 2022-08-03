#include "Menu.h"

unsigned int menu_data[] =
{0};

GLuint font_textureID;
const float BG_RED = 0.1922f,
BG_BLUE = 0.549f,
BG_GREEN = 0.9059f,
BG_OPACITY = 1.0f;

void Menu::initialise() {
    state.next_scene_id = -1;
    state.check_main_menu = true;

    GLuint map_textureID = Utility::load_texture("sheet.png");
    state.map = new Map(1, 1, menu_data, map_textureID, 1.0f, 14, 7);

    font_textureID = Utility::load_texture("FRONT.png");
    state.player = new Entity();
    state.player->set_entity_type(PLAYER);
}
void Menu::update(float deltaTime) {
    state.player->update(deltaTime, state.player, state.enemies, 0, state.map);
}
void Menu::render(ShaderProgram* program) {
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    Utility::draw_text(program, font_textureID, "Avoid UFO", 1.0f, -0.5f, glm::vec3(1.5f, -2.0f, 0));
    Utility::draw_text(program, font_textureID, "Press Enter to Start", 1.0f, -0.6f, glm::vec3(1.5f, -4.0f, 0));
}