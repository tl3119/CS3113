#pragma once
#include "Scene.h"
class Menu : public Scene {
public:
	void initialise() override;
	void update(float deltaTime) override;
	void render(ShaderProgram* program) override;
}; 
