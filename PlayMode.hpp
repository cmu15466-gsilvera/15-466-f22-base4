#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"
#include "Story.hpp"
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

    // input tracking:
    struct Button {
        uint8_t downs = 0;
        uint8_t pressed = 0;
    } left, right, select;
    bool can_right, can_left;

    enum UserSelection {
        NONE = 0,
        RIGHT = 1,
        LEFT = 2
    };
    enum UserSelection selection = UserSelection::NONE;

    Story story;
    int selections_made = 0;
    float dt = 0.f; // deltatime (elapsed)

    Text context_text, left_text, right_text;

    // music coming from the tip of the leg (as a demonstration):
    std::shared_ptr<Sound::PlayingSample> action_sound;

    // camera:
    Scene::Camera* camera = nullptr;
};
