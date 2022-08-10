#include "Scene.h"

class LevelB : public Scene {
public:
    int ENEMY_COUNT = 2;
    int PLATFORM_COUNT = 10;

    ~LevelB();

    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
};