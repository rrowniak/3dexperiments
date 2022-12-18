#include "gfx_engine.h"

#include "t3dlib1.h"

#include <filesystem>
#include <iostream>
#include <iterator>
#include <stdexcept>

SDL_Renderer* gRenderer = nullptr;
uint32_t gPixelFormat = 0;

SDL_Surface* LoadFromImg(const char* path)
{
    // SDL_Surface* s = nullptr;

    // auto ext = std::filesystem::path(path).extension();

    // if (ext == ".bmp") {
    //     s = SDL_LoadBMP(path);
    // }

    // return s;
    return IMG_Load(path);
}

Texture::Texture()
    : texture(nullptr)
    , width(0)
    , height(0)
{ }

Texture::Texture(Texture&& rhs)
    : texture(rhs.texture)
    , width(rhs.width)
    , height(rhs.height)
    , pixelFormat(rhs.pixelFormat)
    , pixels(nullptr)
    , pitch(rhs.pitch)
{
    rhs.texture = nullptr;
}

Texture::Texture(int w, int h)
    : width(w)
    , height(h)
{
    texture = SDL_CreateTexture(gRenderer, gPixelFormat, SDL_TEXTUREACCESS_STREAMING,
        width, height);

    if (texture == nullptr) {
        printf("Unable to create texture. SDL Error: %s\n", 
            SDL_GetError());

        throw std::logic_error("Cannot load texture");
    }

    SDL_QueryTexture(texture, &pixelFormat.format, nullptr, &width, &height);
}

Texture::Texture(const char* path)
{
    auto* s = LoadFromImg(path);
    if (s == nullptr) {
        printf("Unable to load image %s! SDL_image Error: %s\n", 
            path, IMG_GetError());
        
        throw std::logic_error("Cannot load texture");
    }

    texture = SDL_CreateTextureFromSurface(gRenderer, s);
    if (texture == nullptr) {
        printf("Unable to create texture from %s! SDL Error: %s\n", 
            path, SDL_GetError());
        
        SDL_FreeSurface(s);
        throw std::logic_error("Cannot load texture");
    }

    SDL_FreeSurface(s);

    SDL_QueryTexture(texture, &pixelFormat.format, nullptr, &width, &height);
}

Texture::~Texture()
{
    if (texture)
        SDL_DestroyTexture(texture);
}

Texture& Texture::operator=(Texture&& rhs)
{
    if (texture) {
        SDL_DestroyTexture(texture);
    }

    texture = rhs.texture;
    width = rhs.width;
    height = rhs.height;
    pixelFormat = rhs.pixelFormat;

    pixels = nullptr;
    pitch = rhs.pitch;

    rhs.texture = nullptr;

    return *this;
}

void Texture::ClearTexture(uint32_t col = 0)
{
    if (LockTexture()) {
        memset(pixels, col, pitch * GetH());
        UnlockTexture();
    }
}

uint32_t Texture::RGB(int r, int g, int b)
{
    return SDL_MapRGB(&pixelFormat, r, g, b);
}

bool Texture::LockTexture()
{
    if (SDL_LockTexture(texture, nullptr, (void**)&pixels, &pitch)) {
        // locking failed
        return false;
    }

    return true;
}

void Texture::UnlockTexture()
{
    SDL_UnlockTexture(texture);
}

void Texture::Putpixel(int x, int y, uint32_t col)
{
    uint32_t pixelPosition = y * (pitch / sizeof(unsigned int)) + x;
    pixels[pixelPosition] = col;
}

TexturedFont::TexturedFont(const char* fontTexture)
    : font(fontTexture)
{ }

void TexturedFont::RenderText(SDL_Renderer* renderer, 
    int x, int y, const char* text)
{
    SDL_Rect dst{x, y, get_glyph_w(), get_glyph_h()};

    while (*text++ != '\0') {
        auto src = get_glyph_coord(*text);
        SDL_RenderCopy(renderer, font.GetTexture(), &src, &dst);
        dst.x += get_glyph_w();
    }
}

SDL_Rect TexturedFont::get_glyph_coord(char c) const
{
    int n = (int)c;
    int row = n / COLLS;
    int coll = n % COLLS;

    SDL_Rect r;
    r.x = coll * get_glyph_w();
    r.y = row * get_glyph_h();
    r.w = get_glyph_w();
    r.h = get_glyph_h();
    return r;
}

GfxSystem::GfxSystem(int w, int h)
    : width(w)
    , height(h)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Error: " << SDL_GetError() << std::endl;
        throw std::runtime_error("SDL_Init failure");
    }

    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG | IMG_INIT_TIF | IMG_INIT_WEBP;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "Error: " << IMG_GetError() << std::endl;
        SDL_Quit();
        throw std::runtime_error("IMG_Init failure");
    }
    if (TTF_Init() < 0) {
        std::cerr << "Error while initializing TTF library" << std::endl;
        IMG_Quit();
        SDL_Quit();
        throw std::runtime_error("TTF_Init failure");
    }

    window = SDL_CreateWindow("test3d", 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED, 
        width,
        height, 
        SDL_WINDOW_SHOWN);
 
    if (window == nullptr) {
        std::cerr << "Window could not be created! SDL_Error: " <<
            SDL_GetError() << std::endl;
        
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        throw std::runtime_error("SDL_CreateWindow failure");
    }

    gRenderer = renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();        
        SDL_Quit();
        throw std::runtime_error("SDL_CreateRenderer failure");
    }

    surfaces.push_back(SDL_GetWindowSurface(window));
    gPixelFormat = surfaces[ScreenId]->format->format;
    currentEvents.reserve(16);

    RGB16Bit = RGB16Bit565;
    // back_buffer = (UCHAR*)surfaces[ScreenId]->pixels;
    // back_lpitch = surfaces[ScreenId]->pitch;

    screen_width = width;
    screen_height = height;
    screen_bpp = 32;

    max_clip_x = width - 1;
    max_clip_y = height - 1;

    textures.emplace_back(width, height);

    font = FC_CreateFont();
    if (font == nullptr) {
        throw std::runtime_error("FC_CreateFont failure");
    }
    FC_LoadFont(font, renderer, "fonts/FreeSans.ttf", 20, 
        FC_MakeColor(255,255,255,255), TTF_STYLE_NORMAL);

    fps.UpdateStateEvery(1000);
    
    std::fill_n(keys, 255, false);

    sysFont = std::make_unique<TexturedFont>("fonts/256x256/charset-sys.png");
}

GfxSystem::~GfxSystem()
{
    // destroy all surfaces except ScreenId

    auto it = surfaces.begin();
    std::advance(it, 1);

    for (;it != surfaces.end(); ++it) {
        if (*it != nullptr) {
            SDL_FreeSurface(*it);
        }
    }

    FC_FreeFont(font);

    if (sysFont) {
        sysFont.reset();
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

void GfxSystem::ProcessEvents()
{
    SDL_Event e;

    while (SDL_PollEvent(&e) != 0) {
        
        currentEvents.push_back(e);
        
        if (e.type == SDL_KEYDOWN) {
            int indx = e.key.keysym.sym & 0xff;
            keys[indx] = true;

        } else if (e.type == SDL_KEYUP) {
            int indx = e.key.keysym.sym & 0xff;
            keys[indx] = false;
        } else if (e.type == SDL_QUIT) {
            exitRequest = true;
        }
    }
}

void GfxSystem::Clear()
{
    SDL_RenderClear(renderer);
}

void GfxSystem::Update()
{
    // SDL_UpdateWindowSurface(window);
    fps.Tick(Get_Clock());
    double v = fps.GetFPS();
    sprintf(text_buff, "%.0f fps", v);
    // Text(text_buff, 0, 0, 0);
    sysFont->RenderText(renderer, 0, 0, text_buff);
    SDL_RenderPresent(renderer);    
}

void GfxSystem::RenderCopy(Texture& t, SDL_Rect& src, SDL_Rect& dst)
{
    SDL_RenderCopy(renderer, t.GetTexture(), &src, &dst);
}

void GfxSystem::RenderCopy(Texture& t, int x, int y)
{
    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    dst.w = t.GetW();
    dst.h = t.GetH();
    SDL_RenderCopy(renderer, t.GetTexture(), NULL, &dst);
}

void GfxSystem::RenderScreenTexture()
{
    SDL_RenderCopy(renderer, textures[0].GetTexture(), NULL, NULL);
}

SurfIdT GfxSystem::LoadFromImg(const char* path)
{
    if (path == nullptr) {
        return NullId;
    }

    auto* s = ::LoadFromImg(path);

    if (s != nullptr) {
        surfaces.push_back(s);
    }

    return surfaces.size() - 1;
}

uint32_t GfxSystem::RGB(int r, int g, int b)
{ 
    return SDL_MapRGB(surfaces[ScreenId]->format, r, g, b); 
}

SDL_Surface* GfxSystem::GetSurf(SurfIdT id)
{
    if (id >= (int)surfaces.size()) {
        return nullptr;
    }

    return surfaces[id];
}

void GfxSystem::DrawFilledRect(SurfIdT id, int x, int y, int w, int h, uint32_t col)
{
    SDL_Rect r {x, y, w, h};
    SDL_FillRect(surfaces[id], &r, col);
}

void GfxSystem::DrawFilledRectRend(int x, int y, int w, int h, int r, int g, int b)
{
    DrawFilledRectRend(x, y, w, h, RGB(r, g, b));
}

void GfxSystem::DrawFilledRectRend(int x, int y, int w, int h, int col)
{
    // clip
    w = std::min(GetW() - x, w);
    h = std::min(GetH() - y, h);
    int lpitch = back_lpitch >> 2;
    for (int j = y; j < y + h; ++j) {
        uint32_t* start = (uint32_t*)back_buffer + j * lpitch;
        for (int i = x; i < x + w; ++i) {
            start[i] = col;
        }
    }
}

bool GfxSystem::BeginRenderToBuffer(bool clear)
{
    if (clear) {
        Clear();
    }
    ScreenTexture().ClearTexture(0);
    if (!ScreenTexture().LockTexture()) {
        return false;
    }
    back_buffer = (UCHAR*)ScreenTexture().Buffer();
    back_lpitch = ScreenTexture().Pitch();

    return true;
}
    
void GfxSystem::EndRenderToBuffer()
{
    ScreenTexture().UnlockTexture();
    RenderScreenTexture();
}

void GfxSystem::Text(const char* text, int x, int y)
{
    // FC_Draw(font, renderer, 0, 0, "This is %s.\n It works.", "example text");
    // FC_Draw(font, renderer, x, y, text);
    sysFont->RenderText(renderer, x, y, text);
}