#include "Scene.h"

class LevelA : public Scene {
public:
    int ENEMY_COUNT = 2;
    int PLATFORM_COUNT = 23;

    ~LevelA();

    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
};