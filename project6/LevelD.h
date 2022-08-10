#include "Scene.h"

class LevelD : public Scene {
public:
    int ENEMY_COUNT = 0;

    ~LevelD();

    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
}; 
