#include "gfx_engine.h"

#include <filesystem>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <chrono>

// TexturedFont::TexturedFont(const char* fontTexture)
//     : font(fontTexture)
// { }

// void TexturedFont::RenderText(SDL_Renderer* renderer, 
//     int x, int y, const char* text)
// {
//     SDL_Rect dst{x, y, get_glyph_w(), get_glyph_h()};

//     while (*text++ != '\0') {
//         auto src = get_glyph_coord(*text);
//         SDL_RenderCopy(renderer, font.GetTexture(), &src, &dst);
//         dst.x += get_glyph_w();
//     }
// }

// SDL_Rect TexturedFont::get_glyph_coord(char c) const
// {
//     int n = (int)c;
//     int row = n / COLLS;
//     int coll = n % COLLS;

//     SDL_Rect r;
//     r.x = coll * get_glyph_w();
//     r.y = row * get_glyph_h();
//     r.w = get_glyph_w();
//     r.h = get_glyph_h();
//     return r;
// }

GfxSystem::GfxSystem(int w, int h)
    : width(w)
    , height(h)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Error: " << SDL_GetError() << std::endl;
        throw std::runtime_error("SDL_Init failure");
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG | IMG_INIT_TIF | IMG_INIT_WEBP;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "Error: " << IMG_GetError() << std::endl;
        SDL_Quit();
        throw std::runtime_error("IMG_Init failure");
    }

    window = SDL_CreateWindow("3dengine", 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED, 
        width,
        height, 
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
 
    if (window == nullptr) {
        std::cerr << "Window could not be created! SDL_Error: " <<
            SDL_GetError() << std::endl;
        
        IMG_Quit();
        SDL_Quit();
        throw std::runtime_error("SDL_CreateWindow failure");
    }

    glContext = SDL_GL_CreateContext(window);
    if (glContext == NULL) {
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        throw std::runtime_error("OpenGL context could not be created!");
    }

    //Use Vsync
    if (SDL_GL_SetSwapInterval( 1 ) < 0) {
        printf( "Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError() );
    }

    fps.UpdateStateEvery(1000);
    
    std::fill_n(keys, 255, false);

    glEnable(GL_DEPTH_TEST);
    glClear_flags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
    // glEnable(GL_MULTISAMPLE);
    // sysFont = std::make_unique<TexturedFont>("fonts/256x256/charset-sys.png");

    glViewport(0, 0, GetW(), GetH());
}

GfxSystem::~GfxSystem()
{
    // if (sysFont) {
    //     sysFont.reset();
    // }

    // SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
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
    glClearColor(0, 0, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GfxSystem::Clear(float r, float g, float b)
{
    glClearColor(r, g, b, 1.0f);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClear(glClear_flags);
}

void GfxSystem::Update()
{
    // SDL_UpdateWindowSurface(window);
    // using namespace std::chrono;
    // auto time = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();

    // fps.Tick(time);
    // double v = fps.GetFPS();
    // sprintf(text_buff, "%.0f fps", v);
    // // Text(text_buff, 0, 0, 0);
    // sysFont->RenderText(renderer, 0, 0, text_buff);
    // SDL_RenderPresent(renderer);  
    SDL_GL_SwapWindow(window);
}

void GfxSystem::Text(const char* /*text*/, int /*x*/, int /*y*/)
{
    // sysFont->RenderText(renderer, x, y, text);
}

void GfxSystem::glEnableStencilTests()
{
    // SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
    glDepthFunc(GL_LESS);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glClear_flags |= GL_STENCIL_BUFFER_BIT;
}