#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Load.hpp"
#include "Mesh.hpp"
#include "data_path.hpp"
#include "gl_errors.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint hexapod_meshes_for_lit_color_texture_program = 0;
Load<MeshBuffer> hexapod_meshes(LoadTagDefault, []() -> MeshBuffer const* {
    MeshBuffer const* ret = new MeshBuffer(data_path("hexapod.pnct"));
    hexapod_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
    return ret;
});

Load<Scene> hexapod_scene(LoadTagDefault, []() -> Scene const* {
    return new Scene(data_path("hexapod.scene"), [&](Scene& scene, Scene::Transform* transform, std::string const& mesh_name) {
        Mesh const& mesh = hexapod_meshes->lookup(mesh_name);

        scene.drawables.emplace_back(transform);
        Scene::Drawable& drawable = scene.drawables.back();

        drawable.pipeline = lit_color_texture_program_pipeline;

        drawable.pipeline.vao = hexapod_meshes_for_lit_color_texture_program;
        drawable.pipeline.type = mesh.type;
        drawable.pipeline.start = mesh.start;
        drawable.pipeline.count = mesh.count;
    });
});

Load<Sound::Sample> action_sample(LoadTagDefault, []() -> Sound::Sample const* {
    return new Sound::Sample(data_path("alien.opus"));
});

PlayMode::PlayMode()
    : scene(*hexapod_scene)
{
    // get pointers to leg for convenience:
    for (auto& transform : scene.transforms) {
        if (transform.name == "Hip.FL")
            hip = &transform;
        else if (transform.name == "UpperLeg.FL")
            upper_leg = &transform;
        else if (transform.name == "LowerLeg.FL")
            lower_leg = &transform;
    }
    if (hip == nullptr)
        throw std::runtime_error("Hip not found.");
    if (upper_leg == nullptr)
        throw std::runtime_error("Upper leg not found.");
    if (lower_leg == nullptr)
        throw std::runtime_error("Lower leg not found.");

    hip_base_rotation = hip->rotation;
    upper_leg_base_rotation = upper_leg->rotation;
    lower_leg_base_rotation = lower_leg->rotation;

    // get pointer to camera for convenience:
    if (scene.cameras.size() != 1)
        throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
    camera = &scene.cameras.front();

    text.init();
}

PlayMode::~PlayMode()
{
}

bool PlayMode::handle_event(SDL_Event const& evt, glm::uvec2 const& window_size)
{

    if (evt.type == SDL_KEYDOWN) {
        if (evt.key.keysym.sym == SDLK_ESCAPE) {
            SDL_SetRelativeMouseMode(SDL_FALSE);
            return true;
        } else if (evt.key.keysym.sym == SDLK_a || evt.key.keysym.sym == SDLK_LEFT) {
            left.downs += 1;
            left.pressed = true;
            return true;
        } else if (evt.key.keysym.sym == SDLK_d || evt.key.keysym.sym == SDLK_RIGHT) {
            right.downs += 1;
            right.pressed = true;
            return true;
        } else if (evt.key.keysym.sym == SDLK_RETURN) {
            select.downs += 1;
            select.pressed = true;
            return true;
        }
    } else if (evt.type == SDL_KEYUP) {
        if (evt.key.keysym.sym == SDLK_a || evt.key.keysym.sym == SDLK_LEFT) {
            left.pressed = false;
            can_left = true;
            return true;
        } else if (evt.key.keysym.sym == SDLK_d || evt.key.keysym.sym == SDLK_RIGHT) {
            right.pressed = false;
            can_right = true;
            return true;
        } else if (evt.key.keysym.sym == SDLK_RETURN) {
            select.pressed = false;
            return true;
        }
    }

    return false;
}

void PlayMode::update(float elapsed)
{

    // inputs logic
    {
        glm::mat4x3 frame = camera->transform->make_local_to_parent();
        glm::vec3 cam_right = frame[0];
        // glm::vec3 up = frame[1];
        // glm::vec3 forward = -frame[2];
        glm::vec3 cameraRight = camera->transform->position + cam_right * 2.f;
        glm::vec3 cameraLeft = camera->transform->position + cam_right * -2.f;
        const float volume = 2.f;
        const float radius = 0.1f;
        if (left.pressed && !right.pressed && can_left) {
            action_sound = Sound::play_3D(*action_sample, volume, cameraLeft, radius);
            can_left = false;
            text.set_text("left");
        }
        if (!left.pressed && right.pressed && can_right) {
            action_sound = Sound::play_3D(*action_sample, volume, cameraRight, radius);
            can_right = false;
            text.set_text("right");
        }
    }

    { // update listener to camera position:
        glm::mat4x3 frame = camera->transform->make_local_to_parent();
        glm::vec3 right = frame[0];
        glm::vec3 at = frame[3];
        Sound::listener.set_position_right(at, right, 1.0f / 60.0f);
    }

    // reset button press counters:
    left.downs = 0;
    right.downs = 0;
    select.downs = 0;
}

void PlayMode::draw(glm::uvec2 const& drawable_size)
{
    // update camera aspect ratio for drawable:
    camera->aspect = float(drawable_size.x) / float(drawable_size.y);

    // set up light type and position for lit_color_texture_program:
    //  TODO: consider using the Light(s) in the scene to do this
    glUseProgram(lit_color_texture_program->program);
    glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
    glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, -1.0f)));
    glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
    glUseProgram(0);

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClearDepth(1.0f); // 1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); // this is the default depth comparison function, but FYI you can change it.

    scene.draw(*camera);

    { // use DrawLines to overlay some text:
        glDisable(GL_DEPTH_TEST);
        float aspect = float(drawable_size.x) / float(drawable_size.y);
        DrawLines lines(glm::mat4(
            1.0f / aspect, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f));

        constexpr float H = 0.09f;
        lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
            glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
            glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
            glm::u8vec4(0x00, 0x00, 0x00, 0x00));
        float ofs = 2.0f / drawable_size.y;
        lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
            glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + +0.1f * H + ofs, 0.0),
            glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
            glm::u8vec4(0xff, 0xff, 0xff, 0x00));
    }

    {
        float x = (float)drawable_size.x * (200.0f / 1280.0f);
        float y = (float)drawable_size.y * (200.0f / 720.0f);
        float width = (float)drawable_size.x * (800.0f / 1280.0f);
        text.draw(drawable_size, width, glm::vec2(x, y), 1.f, glm::vec3(1.0f, 1.0f, 1.0f));
    }

    GL_ERRORS();
}