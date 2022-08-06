#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"
#include "Text.hpp"

#include <glm/glm.hpp>

#include <deque>
#include <vector>

struct PlayMode : Mode {
    PlayMode();
    virtual ~PlayMode();

    // functions called by main loop:
    virtual bool handle_event(SDL_Event const&, glm::uvec2 const& window_size) override;
    virtual void update(float elapsed) override;
    virtual void draw(glm::uvec2 const& drawable_size) override;

    //----- game state -----
    const std::string choice_graph = "choice-graph.txt";

    // input tracking:
    struct Button {
        uint8_t downs = 0;
        uint8_t pressed = 0;
    } left, right, select;
    bool can_right, can_left;

    // local copy of the game scene (so code can change it during gameplay):
    Scene scene;

    Text text;

    // hexapod leg to wobble:
    glm::quat hip_base_rotation;
    glm::quat upper_leg_base_rotation;
    glm::quat lower_leg_base_rotation;
    float wobble = 0.0f;

    glm::vec3 get_leg_tip_position();

    // music coming from the tip of the leg (as a demonstration):
    std::shared_ptr<Sound::PlayingSample> action_sound;

    // camera:
    Scene::Camera* camera = nullptr;
};
