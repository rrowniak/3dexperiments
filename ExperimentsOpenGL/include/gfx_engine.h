#ifndef __GFX_ENGINE_H__
#define __GFX_ENGINE_H__

#define GL_GLEXT_PROTOTYPES

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_opengl.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <memory>

class FPSCounter
{
public:
    void UpdateStateEvery(uint32_t msec)
    {
        recalculate_msec = msec;
    }

    void Tick(uint32_t currentTime)
    {
        ++frame_count;
        if (start_msec == 0)
        {
            start_msec = currentTime;
            return;
        }

        uint32_t dt = currentTime - start_msec;

        if (dt >= recalculate_msec)
        {
            fps = (double)frame_count / ((double)dt / 1000.0);
            start_msec = currentTime;
            frame_count = 0;
        }
    }

    double GetFPS() const
    {
        return fps;
    }
private:
    uint32_t recalculate_msec = 1000; // 1 sec
    uint32_t start_msec = 0;
    uint32_t frame_count = 0;
    double fps = 0.0;
};

// class TexturedFont
// {
// public:
//     TexturedFont(const char* fontTexture);

//     void RenderText(SDL_Renderer* renderer, 
//         int x, int y, const char* text);
// private:
//     Texture font;
//     static constexpr int ROWS = 16;
//     static constexpr int COLLS = 16;

//     int get_glyph_w() const { return font.GetW() / COLLS; }
//     int get_glyph_h() const { return font.GetH() / ROWS; }

//     SDL_Rect get_glyph_coord(char c) const;
// };

class GfxSystem
{
public:
    GfxSystem(int w, int h);
    ~GfxSystem();

    void ProcessEvents();
    void Clear();
    void Clear(float r, float g, float b);
    void Update();

    int GetW() const { return width; }
    int GetH() const { return height; }

    bool ExitRequest() const { return exitRequest; }

    const std::vector<SDL_Event> Events() const { return currentEvents; }
    const bool* key() const { return keys; }
    void ClearEvents() { currentEvents.clear(); }

    void Text(const char* text, int x, int y);

    void glEnableStencilTests();
private:
    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;
    SDL_Renderer* renderer = nullptr;
    unsigned int glClear_flags = 0;
    bool keys[255];
    int width, height;
    char text_buff[2014];

    std::vector<SDL_Event> currentEvents;

    bool exitRequest = false;

    FPSCounter fps;
};


// Default camera values
const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  3.5f;
const float SENSITIVITY =  0.5f;
const float ZOOM        =  45.0f;

class Camera
{
public:
    Camera(GfxSystem& eng, 
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), 
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), 
        float yaw = YAW, 
        float pitch = PITCH)
        : engine(eng)
        , Position(position)
        , Front(glm::vec3(0.0f, 0.0f, -1.0f))
        , WorldUp(up)
        , Yaw(yaw)
        , Pitch(pitch)
    {
        updateCameraVectors();
    }

    void update()
    {
        // check keys
        if (engine.key()['w']) {
            Position += Front * MovementSpeed;
        }

        if (engine.key()['s']) {
            Position -= Front * MovementSpeed;
        }

        if (engine.key()['a']) {
            Position -= Right * MovementSpeed;
        }

        if (engine.key()['d']) {
            Position += Right * MovementSpeed;
        }

        if (engine.key()['q']) {
            Yaw -= 5 * MouseSensitivity;
        }

        if (engine.key()['e']) {
            Yaw += 5 * MouseSensitivity;
        }

        if (engine.key()['r']) {
            Pitch += 5 * MouseSensitivity;
        }

        if (engine.key()['f']) {
            Pitch -= 5 * MouseSensitivity;
        }

        // mouse events
        for (auto e : engine.Events()) {
            if (e.type == SDL_MOUSEMOTION) {
                //Get mouse position
                int x, y;
                SDL_GetMouseState(&x, &y);
                int dx = (last_mouse_x == -1)? 0 : x - last_mouse_x;
                int dy = (last_mouse_y == -1)? 0 : y - last_mouse_y;
                last_mouse_x = x;
                last_mouse_y = y;

                // update params
                float xoffset = dx * MouseSensitivity;
                float yoffset = - dy * MouseSensitivity;

                Yaw   += xoffset;
                Pitch += yoffset;

                bool constrainPitch = true;
                // make sure that when pitch is out of bounds, 
                // screen doesn't get flipped
                if (constrainPitch)
                {
                    if (Pitch > 89.0f)
                        Pitch = 89.0f;
                    if (Pitch < -89.0f)
                        Pitch = -89.0f;
                }
            } else if (e.type == SDL_MOUSEWHEEL) {
                if (e.wheel.y > 0) {
                    Zoom += 1;
                }

                if (e.wheel.y < 0) {
                    Zoom -= 1;
                }

                if (Zoom < 1.0f)
                    Zoom = 1.0f;
                if (Zoom > 45.0f)
                    Zoom = 45.0f; 
            }
        }

        updateCameraVectors();
    }

    glm::mat4 lookAt() const
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    glm::mat4 projection() const
    {
        return glm::perspective(glm::radians(Zoom),
            (float)engine.GetW() / (float)engine.GetH(), 0.1f, 100.0f);
    }

    glm::vec3 position() const { return Position; }

    glm::vec3 front() const { return Front; }

    float zoom() const { return Zoom; }

    void setSpeed(float s) { MovementSpeed = s; }

    void setMouseSens(float s) { MouseSensitivity = s; }
private:
    GfxSystem& engine;
    int last_mouse_x = -1;
    int last_mouse_y = -1;
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // euler Angles
    float Yaw;
    float Pitch;
    // camera options
    float MovementSpeed = SPEED;
    float MouseSensitivity = SENSITIVITY;
    float Zoom = ZOOM;

    void updateCameraVectors()
    {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // also re-calculate the Right and Up vector
        // normalize the vectors, because their length gets closer to 0
        // the more you look up or down which results in slower movement.
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up    = glm::normalize(glm::cross(Right, Front));
    }
};

#endif // __GFX_ENGINE_H__