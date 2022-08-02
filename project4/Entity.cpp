#include "Entity.h"

Entity::Entity()
{
    position = glm::vec3(0.0f);
    velocity = glm::vec3(0.0f);
    acceleration = glm::vec3(0.0f);

    movement = glm::vec3(0.0f);

    speed = 0;
    model_matrix = glm::mat4(1.0f);
}

Entity::~Entity()
{
    delete[] animation_up;
    delete[] animation_down;
    delete[] animation_left;
    delete[] animation_right;
    delete[] walking;
}

void Entity::draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index)
{
    // Step 1: Calculate the UV location of the indexed frame
    float u_coord = (float)(index % animation_cols) / (float)animation_cols;
    float v_coord = (float)(index / animation_cols) / (float)animation_rows;

    // Step 2: Calculate its UV size
    float width = 1.0f / (float)animation_cols;
    float height = 1.0f / (float)animation_rows;

    // Step 3: Just as we have done before, match the texture coordinates to the vertices
    float tex_coords[] =
    {
        u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width, v_coord,
        u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord
    };

    float vertices[] =
    {
        -0.5, -0.5, 0.5, -0.5,  0.5, 0.5,
        -0.5, -0.5, 0.5,  0.5, -0.5, 0.5
    };

    // Step 4: And render
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->texCoordAttribute);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void Entity::activate_ai(Entity* player)
{
    switch (ai_type)
    {
    case WALKER:
        ai_walker();
        break;

    case GUARD:
        ai_guard(player);
        break;

    case PATROLLER:
        ai_patroller();
        break;

    default:
        break;
    }
}

void Entity::ai_patroller() {
    switch (ai_state) {

    case WALKING:
        if (position.x > 3.0f) {
            movement = glm::vec3(-1.5f, 0.0f, 0.0f);
        }
        else {
            //movement = glm::vec3(1.0f, 0.0f, 0.0f);
            ai_state = WALKING_ANOTHER_WAY;
        }
        break;

    case WALKING_ANOTHER_WAY:
        if (position.x < 1.5f) {
            movement = glm::vec3(1.5f, 0.0f, 0.0f);
        }
        else {
            ai_state = WALKING;
        }
        break;

    default:
        break;
    }
}
void Entity::ai_walker()
{
    movement = glm::vec3(-1.0f, 0.0f, 0.0f);
}

void Entity::ai_guard(Entity* player)
{
    switch (ai_state) {
    case IDLE:
        if (glm::distance(position, player->position) < 3.0f) ai_state = WALKING;
        break;

    case WALKING:
        if (position.x > player->get_position().x) {
            movement = glm::vec3(-1.0f, 0.0f, 0.0f);
        }
        else {
            movement = glm::vec3(1.0f, 0.0f, 0.0f);
        }
        break;

    case WALKING_ANOTHER_WAY:
        break;

    default:
        break;
    }
}

void Entity::update(float delta_time, Entity* player, Entity* collidable_entities, int collidable_entity_count)
{
    if (!is_active) return;
    //player != nullptr && player->entity_type == PLAYER &&
    if (player != nullptr && player->entity_type == PLAYER && collidable_entities != nullptr && collidable_entities->entity_type != ENEMY) {
        collided_top = false;
        collided_bottom = false;
        collided_left = false;
        collided_right = false;
    }

    if (player != nullptr && player->entity_type == ENEMY && collidable_entities != nullptr && collidable_entities->entity_type != PLAYER) {
        collided_top = false;
        collided_bottom = false;
        collided_left = false;
        collided_right = false;
    }

    if (collidable_entities == nullptr) {
        collided_top = false;
        collided_bottom = false;
        collided_left = false;
        collided_right = false;
    }

    // ����Ҫ��
    if (entity_type == ENEMY && player->entity_type == PLAYER && player->speed != 0) activate_ai(player);
    //else if (entity_type == ENEMY && player->entity_type == ENEMY) activate_ai(player);

    if (entity_type == PLAYER) {
        if (animation_indices != NULL)
        {
            if (glm::length(movement) != 0)
            {
                animation_time += delta_time;
                float frames_per_second = (float)1 / SECONDS_PER_FRAME;

                if (animation_time >= frames_per_second)
                {
                    animation_time = 0.0f;
                    animation_index++;

                    if (animation_index >= animation_frames)
                    {
                        animation_index = 0;
                    }
                }
            }
        }
    }

    // Our character moves from left to right, so they need an initial velocity
    velocity.x = movement.x * speed;

    // Now we add the rest of the gravity physics
    velocity += acceleration * delta_time;

    position.y += velocity.y * delta_time;
    check_collision_y(collidable_entities, collidable_entity_count);

    position.x += velocity.x * delta_time;
    check_collision_x(collidable_entities, collidable_entity_count);

    // Jump
    if (is_jumping)
    {
        // STEP 1: Immediately return the flag to its original false state
        is_jumping = false;

        // STEP 2: The player now acquires an upward velocity
        velocity.y += jumping_power;
    }

    model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);
}

void const Entity::check_collision_y(Entity* collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity* collidable_entity = &collidable_entities[i];

        if (check_collision(collidable_entity))
        {
            float y_distance = fabs(position.y - collidable_entity->position.y);
            float y_overlap = fabs(y_distance - (height / 2.0f) - (collidable_entity->height / 2.0f));
            if (velocity.y > 0) {
                position.y -= y_overlap;
                velocity.y = 0;
                collided_top = true;
            }
            else if (velocity.y < 0) {
                position.y += y_overlap;
                velocity.y = 0;
                collided_bottom = true;
            }
        }
    }
}

void const Entity::check_collision_x(Entity* collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity* collidable_entity = &collidable_entities[i];

        if (check_collision(collidable_entity))
        {
            float x_distance = fabs(position.x - collidable_entity->position.x);
            float x_overlap = fabs(x_distance - (width / 2.0f) - (collidable_entity->width / 2.0f));
            if (velocity.x > 0) {
                position.x -= x_overlap;
                velocity.x = 0;
                collided_right = true;
            }
            else if (velocity.x < 0) {
                position.x += x_overlap;
                velocity.x = 0;
                collided_left = true;
            }
        }
    }
}

void Entity::render(ShaderProgram* program)
{
    if (!is_active) return;

    program->SetModelMatrix(model_matrix);

    if (animation_indices != NULL)
    {
        draw_sprite_from_texture_atlas(program, texture_id, animation_indices[animation_index]);
        return;
    }

    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float tex_coords[] = { 0.0,  1.0, 1.0,  1.0, 1.0, 0.0,  0.0,  1.0, 1.0, 0.0,  0.0, 0.0 };

    glBindTexture(GL_TEXTURE_2D, texture_id);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->texCoordAttribute);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

bool Entity::check_collision(Entity* other)
{
    // If we are checking with collisions with ourselves, this should be false
    if (other == this) return false;

    // If either entity is inactive, there shouldn't be any collision
    if (!is_active || !other->is_active) return false;

    float x_distance = fabs(position.x - other->position.x) - ((width + other->width) / 2.0f);
    float y_distance = fabs(position.y - other->position.y) - ((height + other->height) / 2.0f);

    if (x_distance < 0.0f && y_distance < 0.0f) {
        collision = other->entity_type;
        collision_enemy = other;
    }

    return x_distance < 0.0f && y_distance < 0.0f;
}