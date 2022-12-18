#ifndef __GFX_ENGINE_H__
#define __GFX_ENGINE_H__

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <vector>
#include <memory>

#include <SDL_FontCache.h>

#define RGB32(r, g, b) ((r << 16) + (g << 8) + b)

using SurfIdT = int;

constexpr int ScreenId = 0;
constexpr int NullId = -1;

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

class Texture
{
public:
    Texture();
    Texture(Texture&& rhs);
    Texture(int w, int h);
    // from file
    Texture(const char* path);
    // from font
    // Texture(const char* textureText, SDL_Color textColor);
    ~Texture();

    Texture& operator=(Texture&& rhs);

    void ClearTexture(uint32_t col);

    // Set color modulation
    void SetColor(uint8_t red, uint8_t green, uint8_t blue);

    // Set blending
    void SetBlendMode(SDL_BlendMode blending);

    // Set alpha modulation
    void SetAlpha(uint8_t alpha);
    
    // Renders texture at given point
    void Render(int x, int y, SDL_Rect* clip = NULL, 
        double angle = 0.0, SDL_Point* center = NULL, 
        SDL_RendererFlip flip = SDL_FLIP_NONE);

    int GetW() const { return width; }
    int GetH() const { return height; }
    SDL_Texture* GetTexture() { return texture; }
    uint32_t RGB(int r, int g, int b);
    // raw buffer access
    bool LockTexture();
    uint32_t* Buffer() { return pixels; }
    int Pitch() { return pitch; }
    void UnlockTexture();
    void Putpixel(int x, int y, uint32_t col);
private:
    //The actual hardware texture
    SDL_Texture* texture;    

    //Image dimensions
    int width;
    int height;
    SDL_PixelFormat pixelFormat;

    uint32_t* pixels = nullptr;
    int pitch = 0;
};

class TexturedFont
{
public:
    TexturedFont(const char* fontTexture);

    void RenderText(SDL_Renderer* renderer, 
        int x, int y, const char* text);
private:
    Texture font;
    static constexpr int ROWS = 16;
    static constexpr int COLLS = 16;

    int get_glyph_w() const { return font.GetW() / COLLS; }
    int get_glyph_h() const { return font.GetH() / ROWS; }

    SDL_Rect get_glyph_coord(char c) const;
};

class GfxSystem
{
public:
    GfxSystem(int w, int h);
    ~GfxSystem();

    void ProcessEvents();
    void Clear();
    void Update();
    void RenderCopy(Texture& t, SDL_Rect& src, SDL_Rect& dst);
    void RenderCopy(Texture& t, int x, int y);
    void RenderScreenTexture();

    SurfIdT LoadFromImg(const char* path);

    SDL_Surface* Screen() { return surfaces[ScreenId]; }
    Texture& ScreenTexture() { return textures[0]; }
    uint32_t RGB(int r, int g, int b);
    int GetW() const { return width; }
    int GetH() const { return height; }

    bool ExitRequest() const { return exitRequest; }

    const std::vector<SDL_Event> Events() const { return currentEvents; }
    const bool* key() const { return keys; }
    void ClearEvents() { currentEvents.clear(); }

    SDL_Surface* GetSurf(SurfIdT id);

    void DrawFilledRect(SurfIdT id, int x, int y, int w, int h, uint32_t col);
    void DrawFilledRectRend(int x, int y, int w, int h, int r, int g, int b);
    void DrawFilledRectRend(int x, int y, int w, int h, int col);

    bool BeginRenderToBuffer(bool clear = true);
    void EndRenderToBuffer();

    void Text(const char* text, int x, int y);
private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    FC_Font* font = nullptr;
    bool keys[255];
    int width, height;
    char text_buff[2014];

    std::vector<SDL_Surface*> surfaces;
    std::vector<Texture> textures;

    std::vector<SDL_Event> currentEvents;

    bool exitRequest = false;

    FPSCounter fps;

    std::unique_ptr<TexturedFont> sysFont;
};

#endif // __GFX_ENGINE_H__