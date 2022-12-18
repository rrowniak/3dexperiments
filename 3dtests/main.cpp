#include <SDL.h>
#include <iostream>
#include <thread>

#include "gfx_engine.h"
#include "t3dlib1.h"
#include "t3dlib4.h"
#include "t3dlib5.h"
#include "t3dlib6.h"
#include "t3dlib7.h"
#include "t3dlib8.h"
#include "t3dlib9.h"
#include "t3dlib10.h"
#include "t3dlib11.h"
#include "t3dlib12.h"
#include "t3dlib13.h"

void t3dTests()
{
    OBJECT4DV1 obj;
    VECTOR4D scale = {1, 1, 1, 1}, pos = {0, 0, 0, 1}, rot = {0, 0, 0, 1};
    Load_OBJECT4DV1_PLG(&obj, "media/cube1.plg", &scale, &pos, &rot);
}

void Demo1_(GfxSystem& engine)
{
    t3dTests();
    std::cout << std::endl;

    SDL_FillRect(engine.Screen(), NULL, 
        SDL_MapRGB(engine.Screen()->format, 0xFF, 0xFF, 0xFF));

    auto bmp = engine.LoadFromImg("./hello_world.bmp");

    while (!engine.ExitRequest()) {
        engine.ProcessEvents();
        
        if (bmp != NullId) {
            auto* surf = engine.GetSurf(bmp);
            SDL_BlitSurface(surf, NULL, engine.Screen(), NULL);
        }
                
        engine.Update();
        std::this_thread::yield();
    }
}

void Demo1(GfxSystem& engine)
{
    // load background image
    BITMAP_IMAGE background_bmp;
    Load_Bitmap_File(&bitmap32bit, "media/cloud03.bmp");
    Create_Bitmap(&background_bmp, 0, 0, 800, 600, 32);
    Load_Image_Bitmap32(&background_bmp, &bitmap32bit, 0, 0, BITMAP_EXTRACT_MODE_ABS);
    Unload_Bitmap_File(&bitmap32bit);

    // load in all the text images

    // intro
    BOB intro_image;
    Load_Bitmap_File(&bitmap32bit, "media/zbufferwr_intro01.bmp");
    Create_BOB(&intro_image, engine.GetW()/2 - 560/2,40,560,160,1, 
        BOB_ATTR_VISIBLE | BOB_ATTR_SINGLE_FRAME, 0, 0, 32); 
    Load_Frame_BOB32(&intro_image, &bitmap32bit,0,0,0,BITMAP_EXTRACT_MODE_ABS);
    Unload_Bitmap_File(&bitmap32bit);

    Texture t1{"media/Omptxt128_24.bmp"};

    constexpr int TILES_W = 5;
    constexpr int TILES_H = 3;
    SDL_Rect SrcQuads[TILES_W * TILES_H];
    int dx = t1.GetW() / TILES_W;
    int dy = t1.GetH() / TILES_H;
    for (int i = 0; i < TILES_W; ++i) {
        for (int j = 0; j < TILES_H; ++j) {
            int indx = i + j * TILES_W;
            SrcQuads[indx].w = dx; SrcQuads[indx].h = dy;
            SrcQuads[indx].x = i * dx; SrcQuads[indx].y = j * dy;
        }
    }

    SDL_Rect DstQuad{0, 0, dx, dy};

    while (!engine.ExitRequest()) {
        engine.ProcessEvents();
        engine.ClearEvents();

        engine.Clear();

        // engine.Text("Demo", 0, 0, 0);
        
        DstQuad.x = 100; DstQuad.y = 100;
        engine.RenderCopy(t1, SrcQuads[8], DstQuad);

        DstQuad.x = 300; DstQuad.y = 100;
        engine.RenderCopy(t1, SrcQuads[0], DstQuad);

        engine.BeginRenderToBuffer(false);
        background_bmp.x = 0;
        background_bmp.y = 0;
        Draw_Bitmap32(&background_bmp, back_buffer, back_lpitch, 0);
        engine.EndRenderToBuffer();
                
        engine.Update();
        std::this_thread::yield();
    }

    std::this_thread::yield();
}

void Demo2(GfxSystem& engine)
{
    // initialize camera position and direction
    POINT4D  cam_pos = {0,0,-100,1};
    VECTOR4D cam_dir = {0,0,0,1};
    // all your initialization code goes here...
    // VECTOR4D vscale={.5,.5,.5,1},
    //      vpos = {0,0,0,1}, 
    //      vrot = {0,0,0,1};

    CAM4DV1        cam;                     // the single camera
    RENDERLIST4DV1 rend_list;               // the single renderlist
    POLYF4DV1      poly1;                   // our lonely polygon
    POINT4D        poly1_pos = {0,0,100,1}; // world position of polygon

    // initialize math engine
    Build_Sin_Cos_Tables();
    // initialize a single polygon
    poly1.state  = POLY4DV1_STATE_ACTIVE;
    poly1.attr   =  0; 
    poly1.color  = RGB16Bit(0,255,0);
    
    poly1.vlist[0].x = 0;
    poly1.vlist[0].y = 50;
    poly1.vlist[0].z = 0;
    poly1.vlist[0].w = 1;

    poly1.vlist[1].x = 50;
    poly1.vlist[1].y = -50;
    poly1.vlist[1].z = 0;
    poly1.vlist[1].w = 1;

    poly1.vlist[2].x = -50;
    poly1.vlist[2].y = -50;
    poly1.vlist[2].z = 0;
    poly1.vlist[2].w = 1;

    poly1.next = poly1.prev = NULL;

    // initialize the camera with 90 FOV, normalized coordinates
    Init_CAM4DV1(&cam,      // the camera object
                CAM_MODEL_EULER, // euler camera model
                &cam_pos,  // initial camera position
                &cam_dir,  // initial camera angles
                NULL,      // no initial target
                50.0,      // near and far clipping planes
                500.0,
                90.0,      // field of view in degrees
                engine.GetW(),   // size of final screen viewport
                engine.GetH());

    // main loop
    while (!engine.ExitRequest()) {        
        static MATRIX4X4 mrot; // general rotation matrix
        static float ang_y = 0;      // rotation angle

        // int index; // looping var

        engine.ProcessEvents();

        Start_Clock();
        
        SDL_FillRect(engine.Screen(), NULL, 
            SDL_MapRGB(engine.Screen()->format, 0x0, 0x0, 0x0));
        // Engine pipeline start

        // initialize the renderlist
        Reset_RENDERLIST4DV1(&rend_list);

        // insert polygon into the renderlist
        Insert_POLYF4DV1_RENDERLIST4DV1(&rend_list, &poly1);

        // generate rotation matrix around y axis
        Build_XYZ_Rotation_MATRIX4X4(0, ang_y, 0, &mrot);

        // rotate polygon slowly
        if (++ang_y >= 360.0) ang_y = 0;

        // rotate the local coords of single polygon in renderlist
        Transform_RENDERLIST4DV1(&rend_list, &mrot, TRANSFORM_LOCAL_ONLY);

        // perform local/model to world transform
        Model_To_World_RENDERLIST4DV1(&rend_list, &poly1_pos);

        // generate camera matrix
        Build_CAM4DV1_Matrix_Euler(&cam, CAM_ROT_SEQ_ZYX);

        // apply world to camera transform
        World_To_Camera_RENDERLIST4DV1(&rend_list, &cam);

        // apply camera to perspective transformation
        Camera_To_Perspective_RENDERLIST4DV1(&rend_list, &cam);

        // apply screen transform
        Perspective_To_Screen_RENDERLIST4DV1(&rend_list, &cam);

        // render the polygon list
        engine.BeginRenderToBuffer();
        Draw_RENDERLIST4DV1_Wire32(&rend_list, back_buffer, back_lpitch);
        engine.EndRenderToBuffer();

        engine.Update();

        Wait_Clock(30);
        std::this_thread::yield();
    }
}

void Demo3(GfxSystem& engine)
{
    // initialize camera position and direction
    POINT4D  cam_pos = {0,0,0,1};
    VECTOR4D cam_dir = {0,0,0,1};
    // all your initialization code goes here...
    VECTOR4D vscale={5.0,5.0,5.0,1},
         vpos = {0,0,0,1}, 
         vrot = {0,0,0,1};

    CAM4DV1        cam;                     // the single camera
    OBJECT4DV1     obj;

    // initialize math engine
    Build_Sin_Cos_Tables();

    // initialize the camera with 90 FOV, normalized coordinates
    Init_CAM4DV1(&cam,      // the camera object
                CAM_MODEL_EULER, // euler camera model
                &cam_pos,  // initial camera position
                &cam_dir,  // initial camera angles
                NULL,      // no initial target
                50.0,      // near and far clipping planes
                500.0,
                90.0,      // field of view in degrees
                engine.GetW(),   // size of final screen viewport
                engine.GetH());

    Load_OBJECT4DV1_PLG(&obj, "media/cube2.plg", &vscale, &vpos, &vrot);

    // set the position of the cube in the world
    obj.world_pos.x = 0;
    obj.world_pos.y = 0;
    obj.world_pos.z = 100;

    // main loop
    while (!engine.ExitRequest()) {        
        static MATRIX4X4 mrot; // general rotation matrix
        // static float x_ang = 0, y_ang = 0, z_ang = 0;      // rotation angle

        // int index; // looping var

        engine.ProcessEvents();

        Start_Clock();
        
        SDL_FillRect(engine.Screen(), NULL, 
            SDL_MapRGB(engine.Screen()->format, 0x0, 0x0, 0x0));
        // Engine pipeline start

        // initialize the renderlist
        Reset_OBJECT4DV1(&obj);

        // generate rotation matrix around y axis
        Build_XYZ_Rotation_MATRIX4X4(0, 5, 0, &mrot);

        // rotate the local coords of single polygon in renderlist
        Transform_OBJECT4DV1(&obj, &mrot, TRANSFORM_LOCAL_ONLY, 1);

        // perform local/model to world transform
        Model_To_World_OBJECT4DV1(&obj);

        // generate camera matrix
        Build_CAM4DV1_Matrix_Euler(&cam, CAM_ROT_SEQ_ZYX);

        Remove_Backfaces_OBJECT4DV1(&obj, &cam);

        // apply world to camera transform
        World_To_Camera_OBJECT4DV1(&obj, &cam);

        // apply camera to perspective transformation
        Camera_To_Perspective_OBJECT4DV1(&obj, &cam);

        // apply screen transform
        Perspective_To_Screen_OBJECT4DV1(&obj, &cam);

        // render the polygon list
        engine.BeginRenderToBuffer();
        Draw_OBJECT4DV1_Wire32(&obj, back_buffer, back_lpitch);
        // Draw_Clip_Line32(0, 0, engine.GetW(), engine.GetH(), 23423, back_buffer, 
        //     back_lpitch);
        engine.EndRenderToBuffer();

        engine.Update();

        Wait_Clock(30);
        std::this_thread::yield();
    }
}

void Demo4(GfxSystem& engine)
{
    // initialize camera position and direction
    POINT4D  cam_pos = {0,200,0,1};
    VECTOR4D cam_dir = {0,0,0,1};
    // all your initialization code goes here...
    VECTOR4D vscale={1.0,1.0,1.0,1},
         vpos = {0,0,0,1}, 
         vrot = {0,0,0,1};

    CAM4DV1        cam;       // the single camera
    OBJECT4DV1     obj;
    RENDERLIST4DV1 rend_list; // the render list

    // initialize math engine
    Build_Sin_Cos_Tables();

    // initialize the camera with 90 FOV, normalized coordinates
    Init_CAM4DV1(&cam,      // the camera object
                CAM_MODEL_EULER, // euler camera model
                &cam_pos,  // initial camera position
                &cam_dir,  // initial camera angles
                NULL,      // no initial target
                50.0,      // near and far clipping planes
                1000.0,
                90.0,      // field of view in degrees
                engine.GetW(),   // size of final screen viewport
                engine.GetH());

    Load_OBJECT4DV1_PLG(&obj, "media/tank1.plg", &vscale, &vpos, &vrot);

    // set the position of the cube in the world
    obj.world_pos.x = 0;
    obj.world_pos.y = 0;
    obj.world_pos.z = 400;

    // main loop
    while (!engine.ExitRequest()) {        
        static MATRIX4X4 mrot; // general rotation matrix
        static float x_ang = 0, y_ang = 0, z_ang = 0;      // rotation angle

        // int index; // looping var

        engine.ProcessEvents();

        Start_Clock();
        
        SDL_FillRect(engine.Screen(), NULL, 
            SDL_MapRGB(engine.Screen()->format, 0x0, 0x0, 0x0));
        // Engine pipeline start

        // initialize the renderlist
        Reset_RENDERLIST4DV1(&rend_list);

        // reset angles
        x_ang = 0;
        y_ang = 1;
        z_ang = 0;

        // process key events
        for (const auto& ev : engine.Events()) {
            if (ev.type == SDL_KEYDOWN) {
                switch (ev.key.keysym.sym)
                {
                case SDLK_UP:
                    cam.dir.x -= 1;
                    break;
                case SDLK_DOWN:
                    cam.dir.x += 1;
                    break;
                case SDLK_LEFT:
                    cam.dir.y += 1;
                    break;
                case SDLK_RIGHT:
                    cam.dir.y -= 1;
                    break;
                default:
                    break;
                }
            }
        }
        engine.ClearEvents();

        // generate rotation matrix around y axis
        Build_XYZ_Rotation_MATRIX4X4(x_ang, y_ang, z_ang, &mrot);

        // rotate the local coords of single polygon in renderlist
        Transform_OBJECT4DV1(&obj, &mrot, TRANSFORM_LOCAL_ONLY, 1);

        constexpr int NUM_OBJECTS = 2;
        constexpr int OBJECT_SPACING = 250;

        for (int x=-NUM_OBJECTS/2; x < NUM_OBJECTS/2; x++)
            for (int z=-NUM_OBJECTS/2; z < NUM_OBJECTS/2; z++) {
                Reset_OBJECT4DV1(&obj);

                // set position of object
                obj.world_pos.x = x * OBJECT_SPACING + OBJECT_SPACING / 2;
                obj.world_pos.y = 0;
                obj.world_pos.z = 500 + z * OBJECT_SPACING + OBJECT_SPACING / 2;

                // attempt to cull object   
                if (!Cull_OBJECT4DV1(&obj, &cam, CULL_OBJECT_XYZ_PLANES)) {
                    // if we get here then the object is visible at this world position
                    // so we can insert it into the rendering list
                    // perform local/model to world transform
                    Model_To_World_OBJECT4DV1(&obj);

                    // insert the object into render list
                    Insert_OBJECT4DV1_RENDERLIST4DV1(&rend_list, &obj);
                } else {
                }
            }

        // generate camera matrix
        Build_CAM4DV1_Matrix_Euler(&cam, CAM_ROT_SEQ_ZYX);

        // remove backfaces
        Remove_Backfaces_RENDERLIST4DV1(&rend_list, &cam);

        // apply world to camera transform
        World_To_Camera_RENDERLIST4DV1(&rend_list, &cam);

        // apply camera to perspective transformation
        Camera_To_Perspective_RENDERLIST4DV1(&rend_list, &cam);

        // apply screen transform
        Perspective_To_Screen_RENDERLIST4DV1(&rend_list, &cam);

        // render the polygon list
        engine.BeginRenderToBuffer();
        Draw_RENDERLIST4DV1_Wire32(&rend_list, back_buffer, back_lpitch);
        engine.EndRenderToBuffer();

        engine.Update();

        Wait_Clock(30);
        std::this_thread::yield();
    }
}

void Demo5(GfxSystem& engine)
{
    // initialize camera position and direction
    POINT4D  cam_pos = {0,0,0,1};
    POINT4D  cam_target = {0,0,0,1};
    VECTOR4D cam_dir = {0,0,0,1};
    // all your initialization code goes here...
    VECTOR4D vscale={1.0,1.0,1.0,1},
         vpos = {0,0,0,1}, 
         vrot = {0,0,0,1};

    CAM4DV1        cam;       // the single camera
    OBJECT4DV1     obj;
    RENDERLIST4DV1 rend_list; // the render list

    // initialize math engine
    Build_Sin_Cos_Tables();

    // initialize the camera with 90 FOV, normalized coordinates
    Init_CAM4DV1(&cam,      // the camera object
                CAM_MODEL_UVN,
                &cam_pos,  // initial camera position
                &cam_dir,  // initial camera angles
                &cam_target,
                50.0,      // near and far clipping planes
                8000.0,
                90.0,      // field of view in degrees
                engine.GetW(),   // size of final screen viewport
                engine.GetH());

    Load_OBJECT4DV1_PLG(&obj, "media/tank1.plg", &vscale, &vpos, &vrot);

    // set the position of the cube in the world
    obj.world_pos.x = 0;
    obj.world_pos.y = 0;
    obj.world_pos.z = 0;

    // main loop
    while (!engine.ExitRequest()) {        
        static MATRIX4X4 mrot; // general rotation matrix
        static float x_ang = 0, y_ang = 5, z_ang = 0;      // rotation angle

        // these are used to create a circling camera
        static float view_angle = 0; 
        static float camera_distance = 1750;
        // static VECTOR4D pos = {0,0,0,0};

        engine.ProcessEvents();

        Start_Clock();
        
        SDL_FillRect(engine.Screen(), NULL, 
            SDL_MapRGB(engine.Screen()->format, 0x0, 0x0, 0x0));
        // Engine pipeline start

        // initialize the renderlist
        Reset_RENDERLIST4DV1(&rend_list);

        // reset angles
        x_ang = 0;
        y_ang = 1;
        z_ang = 0;

        // generate rotation matrix around y axis
        Build_XYZ_Rotation_MATRIX4X4(x_ang, y_ang, z_ang, &mrot);

        // rotate the local coords of single polygon in renderlist
        Transform_OBJECT4DV1(&obj, &mrot, TRANSFORM_LOCAL_ONLY, 1);

        // compute new camera position, rides on a circular path 
        // with a sinudoid modulation
        cam.pos.x = camera_distance*Fast_Cos(view_angle);
        cam.pos.y = camera_distance*Fast_Sin(view_angle);
        cam.pos.z = 2*camera_distance*Fast_Sin(view_angle);

        // advance angle to circle around
        if ((view_angle+=1)>=360)
            view_angle = 0;

        // generate camera matrix
        Build_CAM4DV1_Matrix_UVN(&cam, UVN_MODE_SIMPLE);

        constexpr int NUM_OBJECTS = 16;
        constexpr int OBJECT_SPACING = 250;

        for (int x=-NUM_OBJECTS/2; x < NUM_OBJECTS/2; x++)
            for (int z=-NUM_OBJECTS/2; z < NUM_OBJECTS/2; z++) {
                Reset_OBJECT4DV1(&obj);

                // set position of object
                obj.world_pos.x = x * OBJECT_SPACING + OBJECT_SPACING / 2;
                obj.world_pos.y = 0;
                obj.world_pos.z = 500 + z * OBJECT_SPACING + OBJECT_SPACING / 2;

                // attempt to cull object   
                if (!Cull_OBJECT4DV1(&obj, &cam, CULL_OBJECT_XYZ_PLANES)) {
                    // if we get here then the object is visible at this world position
                    // so we can insert it into the rendering list
                    // perform local/model to world transform
                    Model_To_World_OBJECT4DV1(&obj);

                    // insert the object into render list
                    Insert_OBJECT4DV1_RENDERLIST4DV1(&rend_list, &obj);
                } else {
                }
            }

        // remove backfaces
        Remove_Backfaces_RENDERLIST4DV1(&rend_list, &cam);

        // apply world to camera transform
        World_To_Camera_RENDERLIST4DV1(&rend_list, &cam);

        // apply camera to perspective transformation
        Camera_To_Perspective_RENDERLIST4DV1(&rend_list, &cam);

        // apply screen transform
        Perspective_To_Screen_RENDERLIST4DV1(&rend_list, &cam);

        // render the polygon list
        engine.BeginRenderToBuffer();
        Draw_RENDERLIST4DV1_Wire32(&rend_list, back_buffer, back_lpitch);
        engine.EndRenderToBuffer();

        engine.Update();

        Wait_Clock(30);
        std::this_thread::yield();
    }
}

void Demo6(GfxSystem& engine)
{
    constexpr int UNIVERSE_RADIUS = 4000;

    constexpr int POINT_SIZE = 200;
    constexpr int NUM_POINTS_X = (2 * UNIVERSE_RADIUS / POINT_SIZE);
    constexpr int NUM_POINTS_Z = (2 * UNIVERSE_RADIUS / POINT_SIZE);
    // constexpr int NUM_POINTS = (NUM_POINTS_X * NUM_POINTS_Z);

    // defines for objects
    constexpr int NUM_TOWERS = 96;
    constexpr int NUM_TANKS = 24;
    constexpr int TANK_SPEED = 15;
    // initialize camera position and direction
    POINT4D  cam_pos = {0,40,0,1};
    POINT4D  cam_target = {0,0,0,1};
    VECTOR4D cam_dir = {0,0,0,1};
    // all your initialization code goes here...
    VECTOR4D vscale={1.0,1.0,1.0,1},
         vpos = {0,0,0,1}, 
         vrot = {0,0,0,1};

    CAM4DV1         cam;       // the single camera
    OBJECT4DV1      obj_tower,    // used to hold the master tower
                    obj_tank,     // used to hold the master tank
                    obj_marker,   // the ground marker
                    obj_player;   // the player object             

    POINT4D         towers[NUM_TOWERS],
                    tanks[NUM_TANKS];
    RENDERLIST4DV1 rend_list; // the render list

    // initialize math engine
    Build_Sin_Cos_Tables();

    // initialize the camera with 90 FOV, normalized coordinates
    Init_CAM4DV1(&cam,      // the camera object
                CAM_MODEL_EULER,
                &cam_pos,  // initial camera position
                &cam_dir,  // initial camera angles
                &cam_target,
                200.0,      // near and far clipping planes
                12000.0,
                120.0,      // field of view in degrees
                engine.GetW(),   // size of final screen viewport
                engine.GetH());

    // load the master tank object
    VECTOR4D_INITXYZ(&vscale,0.75,0.75,0.75);
    Load_OBJECT4DV1_PLG(&obj_tank, "media/tank2.plg", &vscale, &vpos, &vrot);

    // load player object for 3rd person view
    VECTOR4D_INITXYZ(&vscale,0.75,0.75,0.75);
    Load_OBJECT4DV1_PLG(&obj_player, "media/tank3.plg", &vscale, &vpos, &vrot);

    // load the master tower object
    VECTOR4D_INITXYZ(&vscale,1.0, 2.0, 1.0);
    Load_OBJECT4DV1_PLG(&obj_tower, "media/tower1.plg", &vscale, &vpos, &vrot);

    // load the master ground marker
    VECTOR4D_INITXYZ(&vscale,3.0,3.0,3.0);
    Load_OBJECT4DV1_PLG(&obj_marker, "media/marker1.plg", &vscale, &vpos, &vrot);

    // position the tanks
    for (int index = 0; index < NUM_TANKS; index++) {
        // randomly position the tanks
        tanks[index].x = RAND_RANGE(-UNIVERSE_RADIUS, UNIVERSE_RADIUS);
        tanks[index].y = 0; // obj_tank.max_radius;
        tanks[index].z = RAND_RANGE(-UNIVERSE_RADIUS, UNIVERSE_RADIUS);
        tanks[index].w = RAND_RANGE(0,360);
    } // end for

    // position the towers
    for (int index = 0; index < NUM_TOWERS; index++) {
        // randomly position the tower
        towers[index].x = RAND_RANGE(-UNIVERSE_RADIUS, UNIVERSE_RADIUS);
        towers[index].y = 0; // obj_tower.max_radius;
        towers[index].z = RAND_RANGE(-UNIVERSE_RADIUS, UNIVERSE_RADIUS);
    } // end for

    // main loop
    while (!engine.ExitRequest()) {        
        static MATRIX4X4 mrot; // general rotation matrix

        // these are used to create a circling camera
        // static float view_angle = 0; 
        // static float camera_distance = 1750;
        // static VECTOR4D pos = {0,0,0,0};
        static float tank_speed;
        static float turning = 0;
        engine.ProcessEvents();

        Start_Clock();
        
        //SDL_FillRect(engine.Screen(), NULL, 
        //    SDL_MapRGB(engine.Screen()->format, 0x0, 0x0, 0x0));
        // Engine pipeline start

        // draw the sky
        engine.DrawFilledRect(ScreenId, 
            0,0, engine.GetW() - 1, engine.GetH() / 2, engine.RGB(0,140,192));

        // draw the ground
        engine.DrawFilledRect(ScreenId, 
            0, engine.GetH() / 2, engine.GetW() - 1, 
            engine.GetH() - 1, engine.RGB(103,62,3));

        // initialize the renderlist
        Reset_RENDERLIST4DV1(&rend_list);

        // turbo
        tank_speed = TANK_SPEED;
        for (const auto& ev : engine.Events()) {
            if (ev.type == SDL_KEYDOWN) {
                switch (ev.key.keysym.sym)
                {
                case SDLK_SPACE:
                    tank_speed = 5*TANK_SPEED;
                    break;
                case SDLK_UP:
                    // move forward
                    cam.pos.x += tank_speed*Fast_Sin(cam.dir.y);
                    cam.pos.z += tank_speed*Fast_Cos(cam.dir.y);
                    break;
                case SDLK_DOWN:
                    // move backward
                    cam.pos.x -= tank_speed*Fast_Sin(cam.dir.y);
                    cam.pos.z -= tank_speed*Fast_Cos(cam.dir.y);
                    break;
                case SDLK_RIGHT:
                    cam.dir.y+=3;
                    // add a little turn to object
                    if ((turning+=2) > 15)
                        turning=15;
                    break;
                case SDLK_LEFT:
                    cam.dir.y-=3;
                    // add a little turn to object
                    if ((turning-=2) < -15)
                        turning=-15;
                    break;
                default:
                    if (turning > 0)
                        turning-=1;
                    else if (turning < 0)
                        turning+=1;
                    break;
                }
            }
        }
        engine.ClearEvents();

        // generate camera matrix
        Build_CAM4DV1_Matrix_Euler(&cam, CAM_ROT_SEQ_ZYX);

        // insert the tanks in the world
        for (int index = 0; index < NUM_TANKS; index++) {
            // reset the object (this only matters for backface and object removal)
            Reset_OBJECT4DV1(&obj_tank);

            // generate rotation matrix around y axis
            Build_XYZ_Rotation_MATRIX4X4(0, tanks[index].w, 0, &mrot);

            // rotate the local coords of the object
            Transform_OBJECT4DV1(&obj_tank, &mrot, TRANSFORM_LOCAL_TO_TRANS,1);

            // set position of tank
            obj_tank.world_pos.x = tanks[index].x;
            obj_tank.world_pos.y = tanks[index].y;
            obj_tank.world_pos.z = tanks[index].z;

            // attempt to cull object   
            if (!Cull_OBJECT4DV1(&obj_tank, &cam, CULL_OBJECT_XYZ_PLANES)) {
                // if we get here then the object is visible at this world position
                // so we can insert it into the rendering list
                // perform local/model to world transform
                Model_To_World_OBJECT4DV1(&obj_tank, TRANSFORM_TRANS_ONLY);

                // insert the object into render list
                Insert_OBJECT4DV1_RENDERLIST4DV1(&rend_list, &obj_tank);
            } // end if
 
        } // end for

        // insert the player into the world
        // reset the object (this only matters for backface and object removal)
        Reset_OBJECT4DV1(&obj_player);

        // set position of tank
        obj_player.world_pos.x = cam.pos.x+300*Fast_Sin(cam.dir.y);
        obj_player.world_pos.y = cam.pos.y-70;
        obj_player.world_pos.z = cam.pos.z+300*Fast_Cos(cam.dir.y);

        // generate rotation matrix around y axis
        Build_XYZ_Rotation_MATRIX4X4(0, cam.dir.y+turning, 0, &mrot);

        // rotate the local coords of the object
        Transform_OBJECT4DV1(&obj_player, &mrot, TRANSFORM_LOCAL_TO_TRANS,1);

        // perform world transform
        Model_To_World_OBJECT4DV1(&obj_player, TRANSFORM_TRANS_ONLY);

        // insert the object into render list
        Insert_OBJECT4DV1_RENDERLIST4DV1(&rend_list, &obj_player);


        // insert the towers in the world
        for (int index = 0; index < NUM_TOWERS; index++) {
            // reset the object (this only matters for backface and object removal)
            Reset_OBJECT4DV1(&obj_tower);

            // set position of tower
            obj_tower.world_pos.x = towers[index].x;
            obj_tower.world_pos.y = towers[index].y;
            obj_tower.world_pos.z = towers[index].z;

            // attempt to cull object   
            if (!Cull_OBJECT4DV1(&obj_tower, &cam, CULL_OBJECT_XYZ_PLANES)) {
                // if we get here then the object is visible at this world position
                // so we can insert it into the rendering list
                // perform local/model to world transform
                Model_To_World_OBJECT4DV1(&obj_tower);

                // insert the object into render list
                Insert_OBJECT4DV1_RENDERLIST4DV1(&rend_list, &obj_tower);
            } // end if
 
        } // end for

        // seed number generator so that modulation of markers is always the same
        srand(13);

        // insert the ground markers into the world
        for (int index_x = 0; index_x < NUM_POINTS_X; index_x++)
            for (int index_z = 0; index_z < NUM_POINTS_Z; index_z++) {
                // reset the object (this only matters for backface and object removal)
                Reset_OBJECT4DV1(&obj_marker);

                // set position of tower
                obj_marker.world_pos.x = RAND_RANGE(-100,100)-UNIVERSE_RADIUS+index_x*POINT_SIZE;
                obj_marker.world_pos.y = obj_marker.max_radius;
                obj_marker.world_pos.z = RAND_RANGE(-100,100)-UNIVERSE_RADIUS+index_z*POINT_SIZE;

                // attempt to cull object   
                if (!Cull_OBJECT4DV1(&obj_marker, &cam, CULL_OBJECT_XYZ_PLANES)) {
                    // if we get here then the object is visible at this world position
                    // so we can insert it into the rendering list
                    // perform local/model to world transform
                    Model_To_World_OBJECT4DV1(&obj_marker);

                    // insert the object into render list
                    Insert_OBJECT4DV1_RENDERLIST4DV1(&rend_list, &obj_marker);
                } // end if

            } // end for

        // remove backfaces
        Remove_Backfaces_RENDERLIST4DV1(&rend_list, &cam);

        // apply world to camera transform
        World_To_Camera_RENDERLIST4DV1(&rend_list, &cam);

        // apply camera to perspective transformation
        Camera_To_Perspective_RENDERLIST4DV1(&rend_list, &cam);

        // apply screen transform
        Perspective_To_Screen_RENDERLIST4DV1(&rend_list, &cam);

        // render the polygon list
        engine.BeginRenderToBuffer();
        Draw_RENDERLIST4DV1_Wire32(&rend_list, back_buffer, back_lpitch);
        engine.EndRenderToBuffer();

        engine.Update();

        Wait_Clock(30);
        // Engine pipeline end         
        std::this_thread::yield();
    }
}

void Demo7(GfxSystem& engine)
{
    constexpr int UNIVERSE_RADIUS = 4000;

    constexpr int POINT_SIZE = 100;
    constexpr int NUM_POINTS_X = (2 * UNIVERSE_RADIUS / POINT_SIZE);
    constexpr int NUM_POINTS_Z = (2 * UNIVERSE_RADIUS / POINT_SIZE);
    // constexpr int NUM_POINTS = (NUM_POINTS_X * NUM_POINTS_Z);

    // defines for objects
    constexpr int NUM_TOWERS = 96;
    constexpr int NUM_TANKS = 32;
    constexpr int TANK_SPEED = 15;

    // create some constants for ease of access
    constexpr int AMBIENT_LIGHT_INDEX = 0; // ambient light index
    constexpr int INFINITE_LIGHT_INDEX = 1; // infinite light index
    constexpr int POINT_LIGHT_INDEX = 2; // point light index
    constexpr int SPOT_LIGHT_INDEX = 3; // spot light index

    // initialize camera position and direction
    POINT4D  cam_pos = {0,40, 0,1};
    POINT4D  cam_target = {0,0,0,1};
    VECTOR4D cam_dir = {0,0,0,1};
    // all your initialization code goes here...
    VECTOR4D vscale={1.0,1.0,1.0,1},
         vpos = {0,0,0,1}, 
         vrot = {0,0,0,1};

    CAM4DV1         cam;       // the single camera
    OBJECT4DV1      obj_tower,    // used to hold the master tower
                    obj_tank,     // used to hold the master tank
                    obj_marker,   // the ground marker
                    obj_player;   // the player object             

    POINT4D         towers[NUM_TOWERS],
                    tanks[NUM_TANKS];
    RENDERLIST4DV1 rend_list; // the render list

    // initialize math engine
    Build_Sin_Cos_Tables();

    // initialize the camera with 90 FOV, normalized coordinates
    Init_CAM4DV1(&cam,      // the camera object
                CAM_MODEL_EULER,
                &cam_pos,  // initial camera position
                &cam_dir,  // initial camera angles
                &cam_target,
                200.0,      // near and far clipping planes
                12000.0,
                120.0,      // field of view in degrees
                engine.GetW(),   // size of final screen viewport
                engine.GetH());

    // load the master tank object
    VECTOR4D_INITXYZ(&vscale,0.75,0.75,0.75);
    Load_OBJECT4DV1_PLG(&obj_tank, "media/tank3.plg", &vscale, &vpos, &vrot);

    // load player object for 3rd person view
    VECTOR4D_INITXYZ(&vscale,0.75,0.75,0.75);
    Load_OBJECT4DV1_PLG(&obj_player, "media/tank2.plg", &vscale, &vpos, &vrot);

    // load the master tower object
    VECTOR4D_INITXYZ(&vscale,1.0, 2.0, 1.0);
    Load_OBJECT4DV1_PLG(&obj_tower, "media/tower1.plg", &vscale, &vpos, &vrot);

    // load the master ground marker
    VECTOR4D_INITXYZ(&vscale,3.0,3.0,3.0);
    Load_OBJECT4DV1_PLG(&obj_marker, "media/marker1.plg", &vscale, &vpos, &vrot);

    // position the tanks
    for (int index = 0; index < NUM_TANKS; index++) {
        // randomly position the tanks
        tanks[index].x = RAND_RANGE(-UNIVERSE_RADIUS, UNIVERSE_RADIUS);
        tanks[index].y = 0; // obj_tank.max_radius;
        tanks[index].z = RAND_RANGE(-UNIVERSE_RADIUS, UNIVERSE_RADIUS);
        tanks[index].w = RAND_RANGE(0,360);
    } // end for

    // position the towers
    for (int index = 0; index < NUM_TOWERS; index++) {
        // randomly position the tower
        towers[index].x = RAND_RANGE(-UNIVERSE_RADIUS, UNIVERSE_RADIUS);
        towers[index].y = 0; // obj_tower.max_radius;
        towers[index].z = RAND_RANGE(-UNIVERSE_RADIUS, UNIVERSE_RADIUS);
    } // end for

    // set up lights
    Reset_Lights_LIGHTV1();

    // create some working colors
    RGBAV1 /*white,*/ gray, black, red, green/*, blue*/;

    // white.rgba = _RGBA32BIT(255,255,255,0);
    gray.rgba  = _RGBA32BIT(100,100,100,0);
    black.rgba = _RGBA32BIT(0,0,0,0);
    red.rgba   = _RGBA32BIT(255,0,0,0);
    green.rgba = _RGBA32BIT(0,255,0,0);
    // blue.rgba  = _RGBA32BIT(0,0,255,0);


    // ambient light
    Init_Light_LIGHTV1(AMBIENT_LIGHT_INDEX,   
                    LIGHTV1_STATE_ON,      // turn the light on
                    LIGHTV1_ATTR_AMBIENT,  // ambient light type
                    gray, black, black,    // color for ambient term only
                    NULL, NULL,            // no need for pos or dir
                    0,0,0,                 // no need for attenuation
                    0,0,0);                // spotlight info NA


    VECTOR4D dlight_dir = {-1,0,-1,0};

    // directional light
    Init_Light_LIGHTV1(INFINITE_LIGHT_INDEX,  
                    LIGHTV1_STATE_ON,      // turn the light on
                    LIGHTV1_ATTR_INFINITE, // infinite light type
                    black, gray, black,    // color for diffuse term only
                    NULL, &dlight_dir,     // need direction only
                    0,0,0,                 // no need for attenuation
                    0,0,0);                // spotlight info NA


    VECTOR4D plight_pos = {0,200,0,0};

    // point light
    Init_Light_LIGHTV1(POINT_LIGHT_INDEX,
                    LIGHTV1_STATE_ON,      // turn the light on
                    LIGHTV1_ATTR_POINT,    // pointlight type
                    black, green, black,   // color for diffuse term only
                    &plight_pos, NULL,     // need pos only
                    0,.001,0,              // linear attenuation only
                    0,0,1);                // spotlight info NA

    VECTOR4D slight_pos = {0,200,0,0};
    VECTOR4D slight_dir = {-1,0,-1,0};

    // spot light
    Init_Light_LIGHTV1(SPOT_LIGHT_INDEX,
                    LIGHTV1_STATE_ON,         // turn the light on
                    LIGHTV1_ATTR_SPOTLIGHT2,  // spot light type 2
                    black, red, black,      // color for diffuse term only
                    &slight_pos, &slight_dir, // need pos only
                    0,.001,0,                 // linear attenuation only
                    0,0,1);    

    // ??
    // create lookup for lighting engine
    RGB_16_8_IndexedRGB_Table_Builder(DD_PIXEL_FORMAT565,  // format we want to build table for
                                  palette,             // source palette
                                  rgblookup);          // lookup table


    // main loop
    while (!engine.ExitRequest()) {        
        static MATRIX4X4 mrot; // general rotation matrix

        // these are used to create a circling camera
        // static float view_angle = 0; 
        // static float camera_distance = 1750;
        // static VECTOR4D pos = {0,0,0,0};
        static float tank_speed;
        static float turning = 0;

        // state variables for different rendering modes and help
        // static int wireframe_mode = -1;
        // static int backface_mode  = 1;
        // static int lighting_mode  = 1;
        // static int help_mode      = 1;
        // static int zsort_mode     = 1;

        engine.ProcessEvents();

        Start_Clock();

        // initialize the renderlist
        Reset_RENDERLIST4DV1(&rend_list);

        // turbo
        tank_speed = TANK_SPEED;
        for (const auto& ev : engine.Events()) {
            if (ev.type == SDL_KEYDOWN) {
                switch (ev.key.keysym.sym)
                {
                case SDLK_SPACE:
                    tank_speed = 5*TANK_SPEED;
                    break;
                case SDLK_UP:
                    // move forward
                    cam.pos.x += tank_speed*Fast_Sin(cam.dir.y);
                    cam.pos.z += tank_speed*Fast_Cos(cam.dir.y);
                    break;
                case SDLK_DOWN:
                    // move backward
                    cam.pos.x -= tank_speed*Fast_Sin(cam.dir.y);
                    cam.pos.z -= tank_speed*Fast_Cos(cam.dir.y);
                    break;
                case SDLK_RIGHT:
                    cam.dir.y+=3;
                    // add a little turn to object
                    if ((turning+=2) > 15)
                        turning=15;
                    break;
                case SDLK_LEFT:
                    cam.dir.y-=3;
                    // add a little turn to object
                    if ((turning-=2) < -15)
                        turning=-15;
                    break;
                default:
                    if (turning > 0)
                        turning-=1;
                    else if (turning < 0)
                        turning+=1;
                    break;
                }
            }
        }
        engine.ClearEvents();

        static float plight_ang = 0, slight_ang = 0; // angles for light motion

        // move point light source in ellipse around game world
        glights[POINT_LIGHT_INDEX].pos.x = 4000*Fast_Cos(plight_ang);
        glights[POINT_LIGHT_INDEX].pos.y = 200;
        glights[POINT_LIGHT_INDEX].pos.z = 4000*Fast_Sin(plight_ang);

        if ((plight_ang+=3) > 360)
            plight_ang = 0;

        // move spot light source in ellipse around game world
        glights[SPOT_LIGHT_INDEX].pos.x = 2000*Fast_Cos(slight_ang);
        glights[SPOT_LIGHT_INDEX].pos.y = 200;
        glights[SPOT_LIGHT_INDEX].pos.z = 2000*Fast_Sin(slight_ang);

        if ((slight_ang-=5) < 0)
            slight_ang = 360;

        // generate camera matrix
        Build_CAM4DV1_Matrix_Euler(&cam, CAM_ROT_SEQ_ZYX);

        // insert the tanks in the world
        for (int index = 0; index < NUM_TANKS; index++) {
            // reset the object (this only matters for backface and object removal)
            Reset_OBJECT4DV1(&obj_tank);

            // generate rotation matrix around y axis
            Build_XYZ_Rotation_MATRIX4X4(0, tanks[index].w, 0, &mrot);

            // rotate the local coords of the object
            Transform_OBJECT4DV1(&obj_tank, &mrot, TRANSFORM_LOCAL_TO_TRANS,1);

            // set position of tank
            obj_tank.world_pos.x = tanks[index].x;
            obj_tank.world_pos.y = tanks[index].y;
            obj_tank.world_pos.z = tanks[index].z;

            // attempt to cull object   
            if (!Cull_OBJECT4DV1(&obj_tank, &cam, CULL_OBJECT_XYZ_PLANES)) {
                // if we get here then the object is visible at this world position
                // so we can insert it into the rendering list
                // perform local/model to world transform
                Model_To_World_OBJECT4DV1(&obj_tank, TRANSFORM_TRANS_ONLY);

                // insert the object into render list
                Insert_OBJECT4DV1_RENDERLIST4DV12(&rend_list, &obj_tank, 0, 0);
            } // end if
 
        } // end for

        // insert the player into the world
        // reset the object (this only matters for backface and object removal)
        Reset_OBJECT4DV1(&obj_player);

        // set position of tank
        obj_player.world_pos.x = cam.pos.x+300*Fast_Sin(cam.dir.y);
        obj_player.world_pos.y = cam.pos.y-70;
        obj_player.world_pos.z = cam.pos.z+300*Fast_Cos(cam.dir.y);

        // generate rotation matrix around y axis
        Build_XYZ_Rotation_MATRIX4X4(0, cam.dir.y+turning, 0, &mrot);

        // rotate the local coords of the object
        Transform_OBJECT4DV1(&obj_player, &mrot, TRANSFORM_LOCAL_TO_TRANS,1);

        // perform world transform
        Model_To_World_OBJECT4DV1(&obj_player, TRANSFORM_TRANS_ONLY);

        // insert the object into render list
        Insert_OBJECT4DV1_RENDERLIST4DV12(&rend_list, &obj_player, 0, 0);


        // insert the towers in the world
        for (int index = 0; index < NUM_TOWERS; index++) {
            // reset the object (this only matters for backface and object removal)
            Reset_OBJECT4DV1(&obj_tower);

            // set position of tower
            obj_tower.world_pos.x = towers[index].x;
            obj_tower.world_pos.y = towers[index].y;
            obj_tower.world_pos.z = towers[index].z;

            // attempt to cull object   
            if (!Cull_OBJECT4DV1(&obj_tower, &cam, CULL_OBJECT_XYZ_PLANES)) {
                // if we get here then the object is visible at this world position
                // so we can insert it into the rendering list
                // perform local/model to world transform
                Model_To_World_OBJECT4DV1(&obj_tower);

                // insert the object into render list
                Insert_OBJECT4DV1_RENDERLIST4DV12(&rend_list, &obj_tower, 0, 0);
            } // end if
 
        } // end for

        // seed number generator so that modulation of markers is always the same
        srand(13);

        // insert the ground markers into the world
        for (int index_x = 0; index_x < NUM_POINTS_X; index_x++)
            for (int index_z = 0; index_z < NUM_POINTS_Z; index_z++) {
                // reset the object (this only matters for backface and object removal)
                Reset_OBJECT4DV1(&obj_marker);

                // set position of tower
                obj_marker.world_pos.x = RAND_RANGE(-100,100)-UNIVERSE_RADIUS+index_x*POINT_SIZE;
                obj_marker.world_pos.y = obj_marker.max_radius;
                obj_marker.world_pos.z = RAND_RANGE(-100,100)-UNIVERSE_RADIUS+index_z*POINT_SIZE;

                // attempt to cull object   
                if (!Cull_OBJECT4DV1(&obj_marker, &cam, CULL_OBJECT_XYZ_PLANES)) {
                    // if we get here then the object is visible at this world position
                    // so we can insert it into the rendering list
                    // perform local/model to world transform
                    Model_To_World_OBJECT4DV1(&obj_marker);

                    // insert the object into render list
                    Insert_OBJECT4DV1_RENDERLIST4DV12(&rend_list, &obj_marker, 0, 0);
                } // end if

            } // end for

        // remove backfaces
        Remove_Backfaces_RENDERLIST4DV1(&rend_list, &cam);

        // light scene all at once 
        Light_RENDERLIST4DV1_World16(&rend_list, &cam, glights, 4);

        // apply world to camera transform
        World_To_Camera_RENDERLIST4DV1(&rend_list, &cam);

        // sort the polygon list (hurry up!)
        Sort_RENDERLIST4DV1(&rend_list,  SORT_POLYLIST_AVGZ);

        // apply camera to perspective transformation
        Camera_To_Perspective_RENDERLIST4DV1(&rend_list, &cam);

        // apply screen transform
        Perspective_To_Screen_RENDERLIST4DV1(&rend_list, &cam);

        // render the polygon list

        engine.BeginRenderToBuffer(false);
        // draw the sky
        engine.DrawFilledRectRend(0,0, engine.GetW() - 1, engine.GetH() / 2, 
            0, 140, 192);

        // draw the ground
        engine.DrawFilledRectRend(0, engine.GetH() / 2, engine.GetW() - 1, 
            engine.GetH() - 1, 103, 62, 3);
        // Draw_RENDERLIST4DV1_Wire32(&rend_list, back_buffer, back_lpitch);
        Draw_RENDERLIST4DV1_Solid32(&rend_list, back_buffer, back_lpitch);
        engine.EndRenderToBuffer();

        engine.Update();

        Wait_Clock(30);
        // Engine pipeline end         
        std::this_thread::yield();
    }
}

void Demo8(GfxSystem& engine)
{
    Build_Sin_Cos_Tables();

    engine.Clear();
    engine.ScreenTexture().ClearTexture(0);

    while (!engine.ExitRequest()) {
        engine.ProcessEvents();
        engine.ClearEvents();        

        Start_Clock();

        POLYF4DV2 face;

        face.tvlist[0].x  = 0;
        face.tvlist[0].y  = 0;
        face.lit_color[0] = RGB32(0, 0, 255);

        face.tvlist[1].x  = 0;
        face.tvlist[1].y  = screen_height - 1;
        face.lit_color[1] = RGB32(255, 0, 0);

        face.tvlist[2].x  = screen_width - 1;
        face.tvlist[2].y  = 0;
        face.lit_color[2] = RGB32(0, 255, 0);

        if (engine.ScreenTexture().LockTexture()) {
            back_buffer = (UCHAR*)engine.ScreenTexture().Buffer();
            back_lpitch = engine.ScreenTexture().Pitch();
            // draw the gouraud shaded triangle

            engine.DrawFilledRectRend(screen_width/2 + 50, screen_height/2 + 50, 50, 50, 
                RGB32(255, 0, 0));

            engine.DrawFilledRectRend(screen_width/2 + 100, screen_height/2 + 50, 50, 50, 
                RGB32(0, 255, 0));

            engine.DrawFilledRectRend(screen_width/2 + 150, screen_height/2 + 50, 50, 50, 
                RGB32(0, 0, 255));

            Draw_Gouraud_Triangle32(&face, back_buffer, back_lpitch);
            engine.ScreenTexture().UnlockTexture();
            engine.RenderScreenTexture();
        }
                
        engine.Update();
        Wait_Clock(30);
        std::this_thread::yield();
    }
}

void Demo9(GfxSystem& engine)
{
    Build_Sin_Cos_Tables();

    engine.Clear();
    engine.ScreenTexture().ClearTexture(0);

    while (!engine.ExitRequest()) {
        engine.ProcessEvents();
        engine.ClearEvents();        

        Start_Clock();

        POLYF4DV2 face;
        face.tvlist[0].x  = (int)RAND_RANGE(0, screen_width - 1);
        face.tvlist[0].y  = (int)RAND_RANGE(0, screen_height - 1);
        face.lit_color[0] = RGB32(RAND_RANGE(0,255), RAND_RANGE(0,255), RAND_RANGE(0,255));

        face.tvlist[1].x  = (int)RAND_RANGE(0, screen_width - 1);
        face.tvlist[1].y  = (int)RAND_RANGE(0, screen_height - 1);
        face.lit_color[1] = RGB32(RAND_RANGE(0,255), RAND_RANGE(0,255), RAND_RANGE(0,255));

        face.tvlist[2].x  = (int)(int)RAND_RANGE(0, screen_width - 1);
        face.tvlist[2].y  = (int)(int)RAND_RANGE(0, screen_height - 1);
        face.lit_color[2] = RGB32(RAND_RANGE(0,255), RAND_RANGE(0,255), RAND_RANGE(0,255));

        if (engine.ScreenTexture().LockTexture()) {
            back_buffer = (UCHAR*)engine.ScreenTexture().Buffer();
            back_lpitch = engine.ScreenTexture().Pitch();
            // draw the gouraud shaded triangle
            Draw_Gouraud_Triangle32(&face, back_buffer, back_lpitch);
            engine.ScreenTexture().UnlockTexture();
            engine.RenderScreenTexture();
        }
                
        engine.Update();
        Wait_Clock(30);
        std::this_thread::yield();
    }
}

void Demo10(GfxSystem& engine)
{
    constexpr int AMBIENT_LIGHT_INDEX =  0; // ambient light index
    constexpr int INFINITE_LIGHT_INDEX = 1; // infinite light index
    constexpr int POINT_LIGHT_INDEX =    2; // point light index
    // constexpr int SPOT_LIGHT1_INDEX =    4; // point light index
    constexpr int SPOT_LIGHT2_INDEX =    3; // spot light index

    POINT4D  cam_pos    = {0,0,0,1};
    POINT4D  cam_target = {0,0,0,1};
    VECTOR4D cam_dir    = {0,0,0,1};

    VECTOR4D vscale={1.0,1.0,1.0,1}, 
            vpos = {0,0,0,1}, 
            vrot = {0,0,0,1};

    CAM4DV1        cam;       // the single camera

    OBJECT4DV2     obj_constant_water,
                obj_flat_water,
                obj_gouraud_water;

    RENDERLIST4DV2 rend_list; // the render list

    RGBAV1 /*white,*/ gray, black, red, green/*, blue*/;

    Build_Sin_Cos_Tables();

    // initialize the camera with 90 FOV, normalized coordinates
    Init_CAM4DV1(&cam,      // the camera object
                CAM_MODEL_EULER, // the euler model
                &cam_pos,  // initial camera position
                &cam_dir,  // initial camera angles
                &cam_target,      // no target
                200.0,      // near and far clipping planes
                12000.0,
                120.0,      // field of view in degrees
                engine.GetW(),   // size of final screen viewport
                engine.GetH());

    // load constant shaded water
    VECTOR4D_INITXYZ(&vscale,10.00,10.00,10.00);
    Load_OBJECT4DV2_COB(&obj_constant_water,"media/water_constant_01.cob",  
                            &vscale, &vpos, &vrot, VERTEX_FLAGS_SWAP_YZ | 
                                                VERTEX_FLAGS_TRANSFORM_LOCAL  | 
                                                VERTEX_FLAGS_TRANSFORM_LOCAL_WORLD );

    // load flat shaded water
    VECTOR4D_INITXYZ(&vscale,10.00,10.00,10.00);
    Load_OBJECT4DV2_COB(&obj_flat_water,"media/water_flat_01.cob",  
                            &vscale, &vpos, &vrot, VERTEX_FLAGS_SWAP_YZ | 
                                                VERTEX_FLAGS_TRANSFORM_LOCAL | 
                                                VERTEX_FLAGS_TRANSFORM_LOCAL_WORLD );

    // load gouraud shaded water
    VECTOR4D_INITXYZ(&vscale,10.00,10.00,10.00);
    Load_OBJECT4DV2_COB(&obj_gouraud_water,"media/water_gouraud_01.cob",  
                            &vscale, &vpos, &vrot, VERTEX_FLAGS_SWAP_YZ | 
                                                VERTEX_FLAGS_TRANSFORM_LOCAL | 
                                                VERTEX_FLAGS_TRANSFORM_LOCAL_WORLD );

    // set up lights
    Reset_Lights_LIGHTV1();

    // create some working colors
    // white.rgba = _RGBA32BIT(255,255,255,0);
    gray.rgba  = _RGBA32BIT(100,100,100,0);
    black.rgba = _RGBA32BIT(0,0,0,0);
    red.rgba   = _RGBA32BIT(255,0,0,0);
    green.rgba = _RGBA32BIT(0,255,0,0);
    // blue.rgba  = _RGBA32BIT(0,0,255,0);

    // ambient light
    Init_Light_LIGHTV1(AMBIENT_LIGHT_INDEX,   
                    LIGHTV1_STATE_ON,      // turn the light on
                    LIGHTV1_ATTR_AMBIENT,  // ambient light type
                    gray, black, black,    // color for ambient term only
                    NULL, NULL,            // no need for pos or dir
                    0,0,0,                 // no need for attenuation
                    0,0,0);                // spotlight info NA

    VECTOR4D dlight_dir = {-1,0,-1,0};

    // directional light
    Init_Light_LIGHTV1(INFINITE_LIGHT_INDEX,  
                    LIGHTV1_STATE_ON,      // turn the light on
                    LIGHTV1_ATTR_INFINITE, // infinite light type
                    black, gray, black,    // color for diffuse term only
                    NULL, &dlight_dir,     // need direction only
                    0,0,0,                 // no need for attenuation
                    0,0,0);                // spotlight info NA


    VECTOR4D plight_pos = {0,200,0,0};

    // point light
    Init_Light_LIGHTV1(POINT_LIGHT_INDEX,
                    LIGHTV1_STATE_ON,      // turn the light on
                    LIGHTV1_ATTR_POINT,    // pointlight type
                    black, green, black,   // color for diffuse term only
                    &plight_pos, NULL,     // need pos only
                    0,.001,0,              // linear attenuation only
                    0,0,1);                // spotlight info NA


    VECTOR4D slight2_pos = {0,200,0,0};
    VECTOR4D slight2_dir = {-1,0,-1,0};

    // spot light2
    Init_Light_LIGHTV1(SPOT_LIGHT2_INDEX,
                    LIGHTV1_STATE_ON,         // turn the light on
                    LIGHTV1_ATTR_SPOTLIGHT2,  // spot light type 2
                    black, red, black,      // color for diffuse term only
                    &slight2_pos, &slight2_dir, // need pos only
                    0,.001,0,                 // linear attenuation only
                    0,0,1);    

    while (!engine.ExitRequest()) {
        engine.ProcessEvents();
        engine.ClearEvents();        

        static MATRIX4X4 mrot;   // general rotation matrix

        // start the timing clock
        Start_Clock();

        // reset the render list
        Reset_RENDERLIST4DV2(&rend_list);

        static float plight_ang = 0, slight_ang = 0; // angles for light motion

        // move point light source in ellipse around game world
        glights[POINT_LIGHT_INDEX].pos.x = 1000*Fast_Cos(plight_ang);
        glights[POINT_LIGHT_INDEX].pos.y = 100;
        glights[POINT_LIGHT_INDEX].pos.z = 1000*Fast_Sin(plight_ang);

        if ((plight_ang+=3) > 360)
            plight_ang = 0;

        // move spot light source in ellipse around game world
        glights[SPOT_LIGHT2_INDEX].pos.x = 1000*Fast_Cos(slight_ang);
        glights[SPOT_LIGHT2_INDEX].pos.y = 200;
        glights[SPOT_LIGHT2_INDEX].pos.z = 1000*Fast_Sin(slight_ang);

        if ((slight_ang-=5) < 0)
            slight_ang = 360;

        // generate camera matrix
        Build_CAM4DV1_Matrix_Euler(&cam, CAM_ROT_SEQ_ZYX);

        // use these to rotate objects
        static float x_ang = 0, y_ang = 0, z_ang = 0;

        //////////////////////////////////////////////////////////////////////////
        // constant shaded water

        // reset the object (this only matters for backface and object removal)
        Reset_OBJECT4DV2(&obj_constant_water);

        // set position of constant shaded water
        obj_constant_water.world_pos.x = -50;
        obj_constant_water.world_pos.y = 0;
        obj_constant_water.world_pos.z = 120;

        // generate rotation matrix around y axis
        Build_XYZ_Rotation_MATRIX4X4(x_ang, y_ang, z_ang, &mrot);

        // rotate the local coords of the object
        Transform_OBJECT4DV2(&obj_constant_water, &mrot, TRANSFORM_LOCAL_TO_TRANS,1);

        // perform world transform
        Model_To_World_OBJECT4DV2(&obj_constant_water, TRANSFORM_TRANS_ONLY);

        // insert the object into render list
        Insert_OBJECT4DV2_RENDERLIST4DV2(&rend_list, &obj_constant_water,0);

        //////////////////////////////////////////////////////////////////////////
        // flat shaded water

        // reset the object (this only matters for backface and object removal)
        Reset_OBJECT4DV2(&obj_flat_water);

        // set position of constant shaded water
        obj_flat_water.world_pos.x = 0;
        obj_flat_water.world_pos.y = 0;
        obj_flat_water.world_pos.z = 120;

        // generate rotation matrix around y axis
        Build_XYZ_Rotation_MATRIX4X4(x_ang, y_ang, z_ang, &mrot);

        // rotate the local coords of the object
        Transform_OBJECT4DV2(&obj_flat_water, &mrot, TRANSFORM_LOCAL_TO_TRANS,1);

        // perform world transform
        Model_To_World_OBJECT4DV2(&obj_flat_water, TRANSFORM_TRANS_ONLY);

        // insert the object into render list
        Insert_OBJECT4DV2_RENDERLIST4DV2(&rend_list, &obj_flat_water,0);

        //////////////////////////////////////////////////////////////////////////
        // gouraud shaded water

        // reset the object (this only matters for backface and object removal)
        Reset_OBJECT4DV2(&obj_gouraud_water);

        // set position of constant shaded water
        obj_gouraud_water.world_pos.x = 50;
        obj_gouraud_water.world_pos.y = 0;
        obj_gouraud_water.world_pos.z = 120;

        // generate rotation matrix around y axis
        Build_XYZ_Rotation_MATRIX4X4(x_ang, y_ang, z_ang, &mrot);

        // rotate the local coords of the object
        Transform_OBJECT4DV2(&obj_gouraud_water, &mrot, TRANSFORM_LOCAL_TO_TRANS,1);

        // perform world transform
        Model_To_World_OBJECT4DV2(&obj_gouraud_water, TRANSFORM_TRANS_ONLY);

        // insert the object into render list
        Insert_OBJECT4DV2_RENDERLIST4DV2(&rend_list, &obj_gouraud_water,0);

        // update rotation angles
        if ((x_ang+=1) > 360) x_ang = 0;
        if ((y_ang+=2) > 360) y_ang = 0;
        if ((z_ang+=3) > 360) z_ang = 0;

        // remove backfaces
        Remove_Backfaces_RENDERLIST4DV2(&rend_list, &cam);

        // light scene all at once 
        Light_RENDERLIST4DV2_World32(&rend_list, &cam, glights, 4);

        // apply world to camera transform
        World_To_Camera_RENDERLIST4DV2(&rend_list, &cam);

        // sort the polygon list (hurry up!)
        Sort_RENDERLIST4DV2(&rend_list,  SORT_POLYLIST_AVGZ);

        // apply camera to perspective transformation
        Camera_To_Perspective_RENDERLIST4DV2(&rend_list, &cam);

        // apply screen transform
        Perspective_To_Screen_RENDERLIST4DV2(&rend_list, &cam);

        engine.BeginRenderToBuffer();

        Draw_RENDERLIST4DV2_Solid32(&rend_list, back_buffer, back_lpitch);

        engine.EndRenderToBuffer();
                
        engine.Update();
        Wait_Clock(30);
        std::this_thread::yield();
    }
}

void Demo11(GfxSystem& engine)
{
    constexpr int AMBIENT_LIGHT_INDEX =  0; // ambient light index
    constexpr int INFINITE_LIGHT_INDEX = 1; // infinite light index
    constexpr int POINT_LIGHT_INDEX =    2; // point light index
    // constexpr int SPOT_LIGHT1_INDEX =    4; // point light index
    constexpr int SPOT_LIGHT2_INDEX =    3; // spot light index

    POINT4D  cam_pos    = {0,0,0,1};
    POINT4D  cam_target = {0,0,0,1};
    VECTOR4D cam_dir    = {0,0,0,1};

    VECTOR4D vscale={1.0,1.0,1.0,1}, 
            vpos = {0,0,0,1}, 
            vrot = {0,0,0,1};

    CAM4DV1        cam;       // the single camera

    OBJECT4DV2     obj_flat_cube;

    RENDERLIST4DV2 rend_list; // the render list

    RGBAV1 /*white,*/ gray, black, red, green/*, blue*/;

    Build_Sin_Cos_Tables();

    // initialize the camera with 90 FOV, normalized coordinates
    Init_CAM4DV1(&cam,      // the camera object
                CAM_MODEL_EULER, // the euler model
                &cam_pos,  // initial camera position
                &cam_dir,  // initial camera angles
                &cam_target,      // no target
                200.0,      // near and far clipping planes
                12000.0,
                120.0,      // field of view in degrees
                engine.GetW(),   // size of final screen viewport
                engine.GetH());

    // load flat shaded cube
    VECTOR4D_INITXYZ(&vscale,20.00,20.00,20.00);
    Load_OBJECT4DV2_COB(&obj_flat_cube,"media/cube_flat_textured_01.cob",  
                            &vscale, &vpos, &vrot, VERTEX_FLAGS_SWAP_YZ | 
                                                VERTEX_FLAGS_TRANSFORM_LOCAL | 
                                                VERTEX_FLAGS_TRANSFORM_LOCAL_WORLD);


    // set up lights
    Reset_Lights_LIGHTV1();

    // create some working colors
    // white.rgba = _RGBA32BIT(255,255,255,0);
    gray.rgba  = _RGBA32BIT(100,100,100,0);
    black.rgba = _RGBA32BIT(0,0,0,0);
    red.rgba   = _RGBA32BIT(255,0,0,0);
    green.rgba = _RGBA32BIT(0,255,0,0);
    // blue.rgba  = _RGBA32BIT(0,0,255,0);

    // ambient light
    Init_Light_LIGHTV1(AMBIENT_LIGHT_INDEX,   
                    LIGHTV1_STATE_ON,      // turn the light on
                    LIGHTV1_ATTR_AMBIENT,  // ambient light type
                    gray, black, black,    // color for ambient term only
                    NULL, NULL,            // no need for pos or dir
                    0,0,0,                 // no need for attenuation
                    0,0,0);                // spotlight info NA

    VECTOR4D dlight_dir = {-1,0,-1,0};

    // directional light
    Init_Light_LIGHTV1(INFINITE_LIGHT_INDEX,  
                    LIGHTV1_STATE_ON,      // turn the light on
                    LIGHTV1_ATTR_INFINITE, // infinite light type
                    black, gray, black,    // color for diffuse term only
                    NULL, &dlight_dir,     // need direction only
                    0,0,0,                 // no need for attenuation
                    0,0,0);                // spotlight info NA


    VECTOR4D plight_pos = {0,200,0,0};

    // point light
    Init_Light_LIGHTV1(POINT_LIGHT_INDEX,
                    LIGHTV1_STATE_ON,      // turn the light on
                    LIGHTV1_ATTR_POINT,    // pointlight type
                    black, green, black,   // color for diffuse term only
                    &plight_pos, NULL,     // need pos only
                    0,.001,0,              // linear attenuation only
                    0,0,1);                // spotlight info NA


    VECTOR4D slight2_pos = {0,200,0,0};
    VECTOR4D slight2_dir = {-1,0,-1,0};

    // spot light2
    Init_Light_LIGHTV1(SPOT_LIGHT2_INDEX,
                    LIGHTV1_STATE_ON,         // turn the light on
                    LIGHTV1_ATTR_SPOTLIGHT2,  // spot light type 2
                    black, red, black,      // color for diffuse term only
                    &slight2_pos, &slight2_dir, // need pos only
                    0,.001,0,                 // linear attenuation only
                    0,0,1);    

    while (!engine.ExitRequest()) {
        engine.ProcessEvents();
        engine.ClearEvents();        

        static MATRIX4X4 mrot;   // general rotation matrix

        // start the timing clock
        Start_Clock();

        // reset the render list
        Reset_RENDERLIST4DV2(&rend_list);

        static float plight_ang = 0, slight_ang = 0; // angles for light motion

        // move point light source in ellipse around game world
        glights[POINT_LIGHT_INDEX].pos.x = 1000*Fast_Cos(plight_ang);
        glights[POINT_LIGHT_INDEX].pos.y = 100;
        glights[POINT_LIGHT_INDEX].pos.z = 1000*Fast_Sin(plight_ang);

        if ((plight_ang+=1) > 360)
            plight_ang = 0;

        // move spot light source in ellipse around game world
        glights[SPOT_LIGHT2_INDEX].pos.x = 1000*Fast_Cos(slight_ang);
        glights[SPOT_LIGHT2_INDEX].pos.y = 200;
        glights[SPOT_LIGHT2_INDEX].pos.z = 1000*Fast_Sin(slight_ang);

        if ((slight_ang-=2) < 0)
            slight_ang = 360;

        // generate camera matrix
        Build_CAM4DV1_Matrix_Euler(&cam, CAM_ROT_SEQ_ZYX);

        // use these to rotate objects
        static float x_ang = 0, y_ang = 0, z_ang = 0;

        //////////////////////////////////////////////////////////////////////////
        // constant shaded water

        // reset the object (this only matters for backface and object removal)
        Reset_OBJECT4DV2(&obj_flat_cube);

        // set position of constant shaded cube
        obj_flat_cube.world_pos.x = 0;
        obj_flat_cube.world_pos.y = 0;
        obj_flat_cube.world_pos.z = 150;

        // generate rotation matrix around y axis
        Build_XYZ_Rotation_MATRIX4X4(x_ang, y_ang, z_ang, &mrot);

        // rotate the local coords of the object
        Transform_OBJECT4DV2(&obj_flat_cube, &mrot, TRANSFORM_LOCAL_TO_TRANS,1);

        // perform world transform
        Model_To_World_OBJECT4DV2(&obj_flat_cube, TRANSFORM_TRANS_ONLY);

        // insert the object into render list
        Insert_OBJECT4DV2_RENDERLIST4DV2(&rend_list, &obj_flat_cube,0);

        //////////////////////////////////////////////////////////////////////////
        // flat shaded water

        // reset the object (this only matters for backface and object removal)
        Reset_OBJECT4DV2(&obj_flat_cube);

        // update rotation angles
        if ((x_ang+=0.1) > 360) x_ang = 0;
        if ((y_ang+=0.2) > 360) y_ang = 0;
        if ((z_ang+=0.3) > 360) z_ang = 0;
        
        // remove backfaces
        Remove_Backfaces_RENDERLIST4DV2(&rend_list, &cam);

        // light scene all at once 
        Light_RENDERLIST4DV2_World32(&rend_list, &cam, glights, 4);

        // apply world to camera transform
        World_To_Camera_RENDERLIST4DV2(&rend_list, &cam);

        // sort the polygon list (hurry up!)
        Sort_RENDERLIST4DV2(&rend_list,  SORT_POLYLIST_AVGZ);

        // apply camera to perspective transformation
        Camera_To_Perspective_RENDERLIST4DV2(&rend_list, &cam);

        // apply screen transform
        Perspective_To_Screen_RENDERLIST4DV2(&rend_list, &cam);

        engine.BeginRenderToBuffer();

        Draw_RENDERLIST4DV2_Solid32(&rend_list, back_buffer, back_lpitch);

        engine.EndRenderToBuffer();
                
        engine.Update();
        Wait_Clock(30);
        std::this_thread::yield();
    }
}

void Demo12(GfxSystem& engine)
{
    constexpr int UNIVERSE_RADIUS = 4000;

    constexpr int POINT_SIZE = 100;
    constexpr int NUM_POINTS_X = (2 * UNIVERSE_RADIUS / POINT_SIZE);
    constexpr int NUM_POINTS_Z = (2 * UNIVERSE_RADIUS / POINT_SIZE);
    // constexpr int NUM_POINTS = (NUM_POINTS_X * NUM_POINTS_Z);

    // defines for objects
    constexpr int NUM_TOWERS = 96;
    constexpr int NUM_TANKS = 32;
    constexpr int TANK_SPEED = 15;

    // create some constants for ease of access
    constexpr int AMBIENT_LIGHT_INDEX = 0; // ambient light index
    constexpr int INFINITE_LIGHT_INDEX = 1; // infinite light index
    constexpr int POINT_LIGHT_INDEX = 2; // point light index
    constexpr int SPOT_LIGHT2_INDEX = 3; // spot light index
    // constexpr int SPOT_LIGHT1_INDEX = 4; // point light index

    // initialize camera position and direction
    POINT4D  cam_pos = {0,40, 0,1};
    POINT4D  cam_target = {0,0,0,1};
    VECTOR4D cam_dir = {0,0,0,1};
    // all your initialization code goes here...
    VECTOR4D vscale={1.0,1.0,1.0,1},
         vpos = {0,0,0,1}, 
         vrot = {0,0,0,1};

    CAM4DV1         cam;       // the single camera
    OBJECT4DV2      obj_tower,    // used to hold the master tower
                    obj_tank,     // used to hold the master tank
                    obj_marker,   // the ground marker
                    obj_player;   // the player object             

    POINT4D         towers[NUM_TOWERS],
                    tanks[NUM_TANKS];
    RENDERLIST4DV2 rend_list; // the render list

    // BITMAP_IMAGE textures[12]; // holds our texture library 

    // initialize math engine
    Build_Sin_Cos_Tables();

    // initialize the camera with 90 FOV, normalized coordinates
    Init_CAM4DV1(&cam,      // the camera object
                CAM_MODEL_EULER,
                &cam_pos,  // initial camera position
                &cam_dir,  // initial camera angles
                &cam_target,
                200.0,      // near and far clipping planes
                12000.0,
                120.0,      // field of view in degrees
                engine.GetW(),   // size of final screen viewport
                engine.GetH());

    // load the master tank object
    VECTOR4D_INITXYZ(&vscale,0.75,0.75,0.75);
    Load_OBJECT4DV2_PLG(&obj_tank, "media/tank3.plg",&vscale, &vpos, &vrot);

    // load player object for 3rd person view
    VECTOR4D_INITXYZ(&vscale,15.75,15.75,15.75);
    Load_OBJECT4DV2_COB(&obj_player,"media/tie04.cob",  
                        &vscale, &vpos, &vrot, VERTEX_FLAGS_INVERT_TEXTURE_V |
                            VERTEX_FLAGS_SWAP_YZ | VERTEX_FLAGS_TRANSFORM_LOCAL_WORLD );

    // load the master tower object
    VECTOR4D_INITXYZ(&vscale,1.0, 2.0, 1.0);
    Load_OBJECT4DV2_PLG(&obj_tower, "media/towerg1.plg",&vscale, &vpos, &vrot);

    // load the master ground marker
    VECTOR4D_INITXYZ(&vscale,3.0,3.0,3.0);
    Load_OBJECT4DV2_PLG(&obj_marker, "media/marker2.plg",&vscale, &vpos, &vrot);
 

    // position the tanks
    for (int index = 0; index < NUM_TANKS; index++) {
        // randomly position the tanks
        tanks[index].x = RAND_RANGE(-UNIVERSE_RADIUS, UNIVERSE_RADIUS);
        tanks[index].y = 0; // obj_tank.max_radius;
        tanks[index].z = RAND_RANGE(-UNIVERSE_RADIUS, UNIVERSE_RADIUS);
        tanks[index].w = RAND_RANGE(0,360);
    } // end for

    // position the towers
    for (int index = 0; index < NUM_TOWERS; index++) {
        // randomly position the tower
        towers[index].x = RAND_RANGE(-UNIVERSE_RADIUS, UNIVERSE_RADIUS);
        towers[index].y = 0; // obj_tower.max_radius;
        towers[index].z = RAND_RANGE(-UNIVERSE_RADIUS, UNIVERSE_RADIUS);
    } // end for

    // set up lights
    Reset_Lights_LIGHTV1();

    // create some working colors
    RGBAV1 /*white,*/ gray, black, red, green/*, blue*/;

    // white.rgba = _RGBA32BIT(255,255,255,0);
    gray.rgba  = _RGBA32BIT(100,100,100,0);
    black.rgba = _RGBA32BIT(0,0,0,0);
    red.rgba   = _RGBA32BIT(255,0,0,0);
    green.rgba = _RGBA32BIT(0,255,0,0);
    // blue.rgba  = _RGBA32BIT(0,0,255,0);


    // ambient light
    Init_Light_LIGHTV1(AMBIENT_LIGHT_INDEX,   
                    LIGHTV1_STATE_ON,      // turn the light on
                    LIGHTV1_ATTR_AMBIENT,  // ambient light type
                    gray, black, black,    // color for ambient term only
                    NULL, NULL,            // no need for pos or dir
                    0,0,0,                 // no need for attenuation
                    0,0,0);                // spotlight info NA


    VECTOR4D dlight_dir = {-1,0,-1,0};

    // directional light
    Init_Light_LIGHTV1(INFINITE_LIGHT_INDEX,  
                    LIGHTV1_STATE_ON,      // turn the light on
                    LIGHTV1_ATTR_INFINITE, // infinite light type
                    black, gray, black,    // color for diffuse term only
                    NULL, &dlight_dir,     // need direction only
                    0,0,0,                 // no need for attenuation
                    0,0,0);                // spotlight info NA


    VECTOR4D plight_pos = {0,200,0,0};

    // point light
    Init_Light_LIGHTV1(POINT_LIGHT_INDEX,
                    LIGHTV1_STATE_ON,      // turn the light on
                    LIGHTV1_ATTR_POINT,    // pointlight type
                    black, green, black,   // color for diffuse term only
                    &plight_pos, NULL,     // need pos only
                    0,.001,0,              // linear attenuation only
                    0,0,1);                // spotlight info NA

    VECTOR4D slight2_pos = {0,200,0,0};
    VECTOR4D slight2_dir = {-1,0,-1,0};

    // spot light
    Init_Light_LIGHTV1(SPOT_LIGHT2_INDEX,
                    LIGHTV1_STATE_ON,         // turn the light on
                    LIGHTV1_ATTR_SPOTLIGHT2,  // spot light type 2
                    black, red, black,      // color for diffuse term only
                    &slight2_pos, &slight2_dir, // need pos only
                    0,.001,0,                 // linear attenuation only
                    0,0,1);


    // main loop
    while (!engine.ExitRequest()) {        
        static MATRIX4X4 mrot; // general rotation matrix

        // these are used to create a circling camera
        // static float view_angle = 0; 
        // static float camera_distance = 1750;
        // static VECTOR4D pos = {0,0,0,0};
        static float tank_speed;
        static float turning = 0;

        // state variables for different rendering modes and help
        // static int wireframe_mode = -1;
        // static int backface_mode  = 1;
        // static int lighting_mode  = 1;
        // static int help_mode      = 1;
        // static int zsort_mode     = 1;

        engine.ProcessEvents();

        Start_Clock();

        // initialize the renderlist
        Reset_RENDERLIST4DV2(&rend_list);

        // turbo
        tank_speed = TANK_SPEED;
        for (const auto& ev : engine.Events()) {
            if (ev.type == SDL_KEYDOWN) {
                switch (ev.key.keysym.sym)
                {
                case SDLK_SPACE:
                    tank_speed = 5*TANK_SPEED;
                    break;
                case SDLK_UP:
                    // move forward
                    cam.pos.x += tank_speed*Fast_Sin(cam.dir.y);
                    cam.pos.z += tank_speed*Fast_Cos(cam.dir.y);
                    break;
                case SDLK_DOWN:
                    // move backward
                    cam.pos.x -= tank_speed*Fast_Sin(cam.dir.y);
                    cam.pos.z -= tank_speed*Fast_Cos(cam.dir.y);
                    break;
                case SDLK_RIGHT:
                    cam.dir.y+=3;
                    // add a little turn to object
                    if ((turning+=2) > 15)
                        turning=15;
                    break;
                case SDLK_LEFT:
                    cam.dir.y-=3;
                    // add a little turn to object
                    if ((turning-=2) < -15)
                        turning=-15;
                    break;
                default:
                    if (turning > 0)
                        turning-=1;
                    else if (turning < 0)
                        turning+=1;
                    break;
                }
            }
        }
        engine.ClearEvents();

        static float plight_ang = 0, slight_ang = 0; // angles for light motion

        // move point light source in ellipse around game world
        glights[POINT_LIGHT_INDEX].pos.x = 4000*Fast_Cos(plight_ang);
        glights[POINT_LIGHT_INDEX].pos.y = 200;
        glights[POINT_LIGHT_INDEX].pos.z = 4000*Fast_Sin(plight_ang);

        if ((plight_ang+=3) > 360)
            plight_ang = 0;

        // move spot light source in ellipse around game world
        glights[SPOT_LIGHT2_INDEX].pos.x = 2000*Fast_Cos(slight_ang);
        glights[SPOT_LIGHT2_INDEX].pos.y = 200;
        glights[SPOT_LIGHT2_INDEX].pos.z = 2000*Fast_Sin(slight_ang);

        if ((slight_ang-=5) < 0)
            slight_ang = 360;

        // generate camera matrix
        Build_CAM4DV1_Matrix_Euler(&cam, CAM_ROT_SEQ_ZYX);

        // insert the tanks in the world
        for (int index = 0; index < NUM_TANKS; index++) {
            // reset the object (this only matters for backface and object removal)
            Reset_OBJECT4DV2(&obj_tank);

            // generate rotation matrix around y axis
            Build_XYZ_Rotation_MATRIX4X4(0, tanks[index].w, 0, &mrot);

            // rotate the local coords of the object
            Transform_OBJECT4DV2(&obj_tank, &mrot, TRANSFORM_LOCAL_TO_TRANS,1);

            // set position of tank
            obj_tank.world_pos.x = tanks[index].x;
            obj_tank.world_pos.y = tanks[index].y;
            obj_tank.world_pos.z = tanks[index].z;

            // attempt to cull object   
            if (!Cull_OBJECT4DV2(&obj_tank, &cam, CULL_OBJECT_XYZ_PLANES)) {
                // if we get here then the object is visible at this world position
                // so we can insert it into the rendering list
                // perform local/model to world transform
                Model_To_World_OBJECT4DV2(&obj_tank, TRANSFORM_TRANS_ONLY);

                // insert the object into render list
                Insert_OBJECT4DV2_RENDERLIST4DV2(&rend_list, &obj_tank, 0);
            } // end if
 
        } // end for

        // insert the player into the world
        // reset the object (this only matters for backface and object removal)
        Reset_OBJECT4DV2(&obj_player);

        // set position of tank
        obj_player.world_pos.x = cam.pos.x+300*Fast_Sin(cam.dir.y);
        obj_player.world_pos.y = cam.pos.y-70;
        obj_player.world_pos.z = cam.pos.z+300*Fast_Cos(cam.dir.y);

        // generate rotation matrix around y axis
        Build_XYZ_Rotation_MATRIX4X4(1, cam.dir.y+turning, 2, &mrot);

        // rotate the local coords of the object
        Transform_OBJECT4DV2(&obj_player, &mrot, TRANSFORM_LOCAL_TO_TRANS,1);

        // perform world transform
        Model_To_World_OBJECT4DV2(&obj_player, TRANSFORM_TRANS_ONLY);

        // insert the object into render list
        Insert_OBJECT4DV2_RENDERLIST4DV2(&rend_list, &obj_player, 0);


        // insert the towers in the world
        for (int index = 0; index < NUM_TOWERS; index++) {
            // reset the object (this only matters for backface and object removal)
            Reset_OBJECT4DV2(&obj_tower);

            // set position of tower
            obj_tower.world_pos.x = towers[index].x;
            obj_tower.world_pos.y = towers[index].y;
            obj_tower.world_pos.z = towers[index].z;

            // attempt to cull object   
            if (!Cull_OBJECT4DV2(&obj_tower, &cam, CULL_OBJECT_XYZ_PLANES)) {
                // if we get here then the object is visible at this world position
                // so we can insert it into the rendering list
                // perform local/model to world transform
                Model_To_World_OBJECT4DV2(&obj_tower);

                // insert the object into render list
                Insert_OBJECT4DV2_RENDERLIST4DV2(&rend_list, &obj_tower, 0);
            } // end if
 
        } // end for

        // seed number generator so that modulation of markers is always the same
        srand(13);

        // insert the ground markers into the world
        for (int index_x = 0; index_x < NUM_POINTS_X; index_x++)
            for (int index_z = 0; index_z < NUM_POINTS_Z; index_z++) {
                // reset the object (this only matters for backface and object removal)
                Reset_OBJECT4DV2(&obj_marker);

                // set position of tower
                obj_marker.world_pos.x = RAND_RANGE(-100,100)-UNIVERSE_RADIUS+index_x*POINT_SIZE;
                obj_marker.world_pos.y = obj_marker.max_radius[0] + 50*Fast_Sin(index_x*10+Fast_Sin(index_z));
                obj_marker.world_pos.z = RAND_RANGE(-100,100)-UNIVERSE_RADIUS+index_z*POINT_SIZE;

                // attempt to cull object   
                if (!Cull_OBJECT4DV2(&obj_marker, &cam, CULL_OBJECT_XYZ_PLANES)) {
                    // if we get here then the object is visible at this world position
                    // so we can insert it into the rendering list
                    // perform local/model to world transform
                    Model_To_World_OBJECT4DV2(&obj_marker);

                    // insert the object into render list
                    Insert_OBJECT4DV2_RENDERLIST4DV2(&rend_list, &obj_marker, 0);
                } // end if

            } // end for

        // remove backfaces
        Remove_Backfaces_RENDERLIST4DV2(&rend_list, &cam);

        // light scene all at once 
        Light_RENDERLIST4DV2_World32(&rend_list, &cam, glights, 4);

        // apply world to camera transform
        World_To_Camera_RENDERLIST4DV2(&rend_list, &cam);

        // sort the polygon list (hurry up!)
        Sort_RENDERLIST4DV2(&rend_list,  SORT_POLYLIST_AVGZ);

        // apply camera to perspective transformation
        Camera_To_Perspective_RENDERLIST4DV2(&rend_list, &cam);

        // apply screen transform
        Perspective_To_Screen_RENDERLIST4DV2(&rend_list, &cam);

        // render the polygon list

        engine.BeginRenderToBuffer(false);
        // draw the sky
        engine.DrawFilledRectRend(0,0, engine.GetW() - 1, engine.GetH() / 2, 
            0, 140, 192);

        // draw the ground
        engine.DrawFilledRectRend(0, engine.GetH() / 2, engine.GetW() - 1, 
            engine.GetH() - 1, 103, 62, 3);
        // Draw_RENDERLIST4DV1_Wire32(&rend_list, back_buffer, back_lpitch);
        Draw_RENDERLIST4DV2_Solid32(&rend_list, back_buffer, back_lpitch);
        engine.EndRenderToBuffer();

        engine.Update();

        Wait_Clock(30);
        // Engine pipeline end         
        std::this_thread::yield();
    }
}

void Demo13(GfxSystem& engine)
{
    // terrain defines
    constexpr int TERRAIN_WIDTH =        4000;
    constexpr int TERRAIN_HEIGHT =       4000;
    constexpr int TERRAIN_SCALE =        700;
    constexpr int MAX_SPEED =            20;

    constexpr float PITCH_RETURN_RATE = 0.33; // how fast the pitch straightens out
    constexpr float PITCH_CHANGE_RATE = 0.02; // the rate that pitch changes relative to height
    constexpr float CAM_HEIGHT_SCALER = 0.3;  // percentage cam height changes relative to height
    constexpr float VELOCITY_SCALER = 0.025;  // rate velocity changes with height
    constexpr float CAM_DECEL       = 0.25;  

    // create some constants for ease of access
    constexpr int AMBIENT_LIGHT_INDEX = 0; // ambient light index
    constexpr int INFINITE_LIGHT_INDEX = 1; // infinite light index
    constexpr int POINT_LIGHT_INDEX = 2; // point light index
    constexpr int SPOT_LIGHT2_INDEX = 3; // spot light index
    // constexpr int SPOT_LIGHT1_INDEX = 4; // point light index

    // initialize camera position and direction
    POINT4D  cam_pos = {0,500, -200,1};
    POINT4D  cam_target = {0,0,0,1};
    VECTOR4D cam_dir = {0,0,0,1};
    // all your initialization code goes here...
    // VECTOR4D vscale={1.0,1.0,1.0,1},
        //  vpos = {0,0,150,1}, 
        //  vrot = {0,0,0,1};

    CAM4DV1         cam;       // the single camera
    OBJECT4DV2      obj_terrain;

    RENDERLIST4DV2 rend_list; // the render list

    // physical model defines play with these to change the feel of the vehicle
    float gravity    = -.40;    // general gravity
    float vel_y      = 0;       // the y velocity of camera/jeep
    float cam_speed  = 0;       // speed of the camera/jeep
    float sea_level  = 50;      // sea level of the simulation
    float gclearance = 75;      // clearance from the camera to the ground
    float neutral_pitch = 10;   // the neutral pitch of the camera

    // BOB cockpit;   

    // BITMAP_IMAGE textures[12]; // holds our texture library 

    // initialize math engine
    Build_Sin_Cos_Tables();

    // initialize the camera with 90 FOV, normalized coordinates
    Init_CAM4DV1(&cam,      // the camera object
                CAM_MODEL_EULER,
                &cam_pos,  // initial camera position
                &cam_dir,  // initial camera angles
                &cam_target,
                10.0,      // near and far clipping planes
                12000.0,
                90.0,      // field of view in degrees
                engine.GetW(),   // size of final screen viewport
                engine.GetH());

    
    VECTOR4D terrain_pos = {0,0,0,0};

    Generate_Terrain_OBJECT4DV2(&obj_terrain,            // pointer to object
                            TERRAIN_WIDTH,           // width in world coords on x-axis
                            TERRAIN_HEIGHT,          // height (length) in world coords on z-axis
                            TERRAIN_SCALE,           // vertical scale of terrain
                            "media/earthheightmap02.bmp",  // filename of height bitmap encoded in 256 colors
                            "media/earthcolormap01.bmp",   // filename of texture map
                             RGB16Bit(255,255,255),  // color of terrain if no texture        
                             &terrain_pos,           // initial position
                             NULL,                   // initial rotations
                             POLY4DV2_ATTR_RGB32 | 
                             POLY4DV2_ATTR_SHADE_MODE_FLAT | /* POLY4DV2_ATTR_SHADE_MODE_GOURAUD */
                             POLY4DV2_ATTR_SHADE_MODE_TEXTURE);

    // set up lights
    Reset_Lights_LIGHTV2(lights2, MAX_LIGHTS);

    // create some working colors
    RGBAV1 /*white,*/ gray, black, red, green/*, blue*/;

    // white.rgba = _RGBA32BIT(255,255,255,0);
    gray.rgba  = _RGBA32BIT(100,100,100,0);
    black.rgba = _RGBA32BIT(0,0,0,0);
    red.rgba   = _RGBA32BIT(255,0,0,0);
    green.rgba = _RGBA32BIT(0,255,0,0);
    // blue.rgba  = _RGBA32BIT(0,0,255,0);

    // ambient light
    Init_Light_LIGHTV2(lights2,
                    AMBIENT_LIGHT_INDEX,   
                    LIGHTV2_STATE_ON,      // turn the light on
                    LIGHTV2_ATTR_AMBIENT,  // ambient light type
                    gray, black, black,    // color for ambient term only
                    NULL, NULL,            // no need for pos or dir
                    0,0,0,                 // no need for attenuation
                    0,0,0);                // spotlight info NA

    VECTOR4D dlight_dir = {-1,0,-1,1};

    // directional light
    Init_Light_LIGHTV2(lights2,
                    INFINITE_LIGHT_INDEX,  
                    LIGHTV2_STATE_ON,      // turn the light on
                    LIGHTV2_ATTR_INFINITE, // infinite light type
                    black, gray, black,    // color for diffuse term only
                    NULL, &dlight_dir,     // need direction only
                    0,0,0,                 // no need for attenuation
                    0,0,0);                // spotlight info NA


    VECTOR4D plight_pos = {0,200,0,1};

    // point light
    Init_Light_LIGHTV2(lights2,
                    POINT_LIGHT_INDEX,
                    LIGHTV2_STATE_ON,      // turn the light on
                    LIGHTV2_ATTR_POINT,    // pointlight type
                    black, green, black,   // color for diffuse term only
                    &plight_pos, NULL,     // need pos only
                    0,.001,0,              // linear attenuation only
                    0,0,1);                // spotlight info NA


    VECTOR4D slight2_pos = {0,200,0,1};
    VECTOR4D slight2_dir = {-1,0,-1,1};

    // spot light2
    Init_Light_LIGHTV2(lights2,
                    SPOT_LIGHT2_INDEX,
                    LIGHTV2_STATE_ON,         // turn the light on
                    LIGHTV2_ATTR_SPOTLIGHT2,  // spot light type 2
                    black, red, black,      // color for diffuse term only
                    &slight2_pos, &slight2_dir, // need pos only
                    0,.001,0,                 // linear attenuation only
                    0,0,1);    

    // load in the cockpit image
    // Load_Bitmap_File(&bitmap16bit, "media/lego01.BMP");
    // Create_BOB(&cockpit, 0,0,800,600,1, BOB_ATTR_VISIBLE | BOB_ATTR_SINGLE_FRAME, 0, 0, 16); 
    // Load_Frame_BOB16(&cockpit, &bitmap16bit,0,0,0,BITMAP_EXTRACT_MODE_ABS);
    // Unload_Bitmap_File(&bitmap16bit);

    // main loop
    while (!engine.ExitRequest()) {        
        static MATRIX4X4 mrot; // general rotation matrix

        static float plight_ang = 0, 
                    slight_ang = 0; // angles for light motion

        engine.ProcessEvents();

        Start_Clock();

        // initialize the renderlist
        Reset_RENDERLIST4DV2(&rend_list);

        if (engine.key()[SDL_SCANCODE_UP]) {
            // move forward
            if ( (cam_speed+=1) > MAX_SPEED) cam_speed = MAX_SPEED;
        }
        if (engine.key()[SDL_SCANCODE_DOWN]) {
            // move backward
            if ((cam_speed-=1) < -MAX_SPEED) cam_speed = -MAX_SPEED;
        }

        if (engine.key()[SDL_SCANCODE_RIGHT]) {
            cam.dir.y+=5;
        }

        if (engine.key()[SDL_SCANCODE_LEFT]) {
            cam.dir.y-=5;
        }

        engine.ClearEvents();
        
        int cell_x = (cam.pos.x  + TERRAIN_WIDTH/2) / obj_terrain.fvar1;
        int cell_y = (cam.pos.z  + TERRAIN_HEIGHT/2) / obj_terrain.fvar1;

        static float terrain_height, delta;

        // test if we are on terrain
        if ( (cell_x >=0) 
            && (cell_x < obj_terrain.ivar1) 
            && (cell_y >=0) 
            && (cell_y < obj_terrain.ivar2) ) {
            // compute vertex indices into vertex list of the current quad
            int v0 = cell_x + cell_y*obj_terrain.ivar2;
            int v1 = v0 + 1;
            int v2 = v1 + obj_terrain.ivar2;
            int v3 = v0 + obj_terrain.ivar2;   

            // now simply index into table 
            terrain_height = 0.25 * (obj_terrain.vlist_trans[v0].y + obj_terrain.vlist_trans[v1].y +
                                    obj_terrain.vlist_trans[v2].y + obj_terrain.vlist_trans[v3].y);

            // compute height difference
            delta = terrain_height - (cam.pos.y - gclearance);

            // test for penetration
            if (delta > 0) {
            // apply force immediately to camera (this will give it a springy feel)
            vel_y+=(delta * (VELOCITY_SCALER));
            // test for pentration, if so move up immediately so we don't penetrate geometry
            cam.pos.y+=(delta*CAM_HEIGHT_SCALER);
            // now this is more of a hack than the physics model :) let move the front
            // up and down a bit based on the forward velocity and the gradient of the 
            // hill
            cam.dir.x -= (delta*PITCH_CHANGE_RATE);
            } // end if
        } // end if

        // decelerate camera
        if (cam_speed > (CAM_DECEL) ) cam_speed-=CAM_DECEL;
        else if (cam_speed < (-CAM_DECEL) ) cam_speed+=CAM_DECEL;
        else cam_speed = 0;

        // force camera to seek a stable orientation
        if (cam.dir.x > (neutral_pitch+PITCH_RETURN_RATE)) cam.dir.x -= (PITCH_RETURN_RATE);
        else if (cam.dir.x < (neutral_pitch-PITCH_RETURN_RATE)) cam.dir.x += (PITCH_RETURN_RATE);
        else cam.dir.x = neutral_pitch;

        // apply gravity
        vel_y+=gravity;

        // test for absolute sea level and push upward..
        if (cam.pos.y < sea_level) { 
            vel_y = 0; 
            cam.pos.y = sea_level;
        } // end if

        // move camera
        cam.pos.x += cam_speed*Fast_Sin(cam.dir.y);
        cam.pos.z += cam_speed*Fast_Cos(cam.dir.y);
        cam.pos.y += vel_y;

        // move point light source in ellipse around game world
        lights2[POINT_LIGHT_INDEX].pos.x = 1000*Fast_Cos(plight_ang);
        lights2[POINT_LIGHT_INDEX].pos.y = 100;
        lights2[POINT_LIGHT_INDEX].pos.z = 1000*Fast_Sin(plight_ang);

        if ((plight_ang+=3) > 360)
            plight_ang = 0;

        // move spot light source in ellipse around game world
        lights2[SPOT_LIGHT2_INDEX].pos.x = 1000*Fast_Cos(slight_ang);
        lights2[SPOT_LIGHT2_INDEX].pos.y = 200;
        lights2[SPOT_LIGHT2_INDEX].pos.z = 1000*Fast_Sin(slight_ang);

        if ((slight_ang-=5) < 0)
            slight_ang = 360;

        Build_CAM4DV1_Matrix_Euler(&cam, CAM_ROT_SEQ_ZYX);

        Reset_OBJECT4DV2(&obj_terrain);

        MAT_IDENTITY_4X4(&mrot);

        // rotate the local coords of the object
        Transform_OBJECT4DV2(&obj_terrain, &mrot, TRANSFORM_LOCAL_TO_TRANS,1);

        // perform world transform
        Model_To_World_OBJECT4DV2(&obj_terrain, TRANSFORM_TRANS_ONLY);

        // insert the object into render list
        Insert_OBJECT4DV2_RENDERLIST4DV2(&rend_list, &obj_terrain,0);

        Remove_Backfaces_RENDERLIST4DV2(&rend_list, &cam);

        World_To_Camera_RENDERLIST4DV2(&rend_list, &cam);
        // render the polygon list

        // clip the polygons themselves now
        Clip_Polys_RENDERLIST4DV2(&rend_list, &cam, CLIP_POLY_X_PLANE 
            | CLIP_POLY_Y_PLANE | CLIP_POLY_Z_PLANE);
        // light scene all at once 
        Transform_LIGHTSV2(lights2, 4, &cam.mcam, TRANSFORM_LOCAL_TO_TRANS);
        Light_RENDERLIST4DV2_World2_16(&rend_list, &cam, lights2, 4);

        // sort the polygon list (hurry up!)
        Sort_RENDERLIST4DV2(&rend_list,  SORT_POLYLIST_AVGZ);

        // apply camera to perspective transformation
        Camera_To_Perspective_RENDERLIST4DV2(&rend_list, &cam);

        // apply screen transform
        Perspective_To_Screen_RENDERLIST4DV2(&rend_list, &cam);

        engine.BeginRenderToBuffer(false);
        // draw the sky
        engine.DrawFilledRectRend(0,0, engine.GetW() - 1, engine.GetH() / 2, 
            0, 140, 192);

        // draw the ground
        engine.DrawFilledRectRend(0, engine.GetH() / 2, engine.GetW() - 1, 
            engine.GetH() - 1, 103, 62, 3);
        // Draw_RENDERLIST4DV1_Wire32(&rend_list, back_buffer, back_lpitch);
        Draw_RENDERLIST4DV2_Solid32(&rend_list, back_buffer, back_lpitch);
        engine.EndRenderToBuffer();

        // draw cockpit
        // Draw_BOB16(&cockpit, 0);

        engine.Update();

        Wait_Clock(30);
        // Engine pipeline end         
        std::this_thread::yield();
    }
}

void Demo14(GfxSystem& engine)
{
    // terrain defines
    constexpr int TERRAIN_WIDTH =        5000;
    constexpr int TERRAIN_HEIGHT =       5000;
    constexpr int TERRAIN_SCALE =        400;
    constexpr int MAX_SPEED =            20;

    constexpr float PITCH_RETURN_RATE = 0.5; // how fast the pitch straightens out
    constexpr float PITCH_CHANGE_RATE = 0.045; // the rate that pitch changes relative to height
    constexpr float CAM_HEIGHT_SCALER = 0.3;  // percentage cam height changes relative to height
    constexpr float VELOCITY_SCALER = 0.03;  // rate velocity changes with height
    constexpr float CAM_DECEL       = 0.25;  

    constexpr float WATER_LEVEL        = 40;    // level where wave will have effect
    constexpr float WAVE_HEIGHT        = 25;    // amplitude of modulation
    constexpr float WAVE_RATE       = (0.1f);   // how fast the wave propagates

    constexpr float MAX_JETSKI_TURN          = 25;    // maximum turn angle
    constexpr float JETSKI_TURN_RETURN_RATE  = .5;    // rate of return for turn
    constexpr float JETSKI_TURN_RATE         =  2;    // turn rate
    constexpr float JETSKI_YAW_RATE          = .1;    // yaw rates, return, and max
    constexpr float MAX_JETSKI_YAW           = 10; 
    constexpr float JETSKI_YAW_RETURN_RATE   = .02; 

    // game states
    constexpr int  GAME_STATE_INIT           = 0;
    constexpr int  GAME_STATE_RESTART        = 1;
    constexpr int  GAME_STATE_RUN            = 2;
    constexpr int  GAME_STATE_INTRO          = 3;
    constexpr int  GAME_STATE_PLAY           = 4;
    constexpr int  GAME_STATE_EXIT           = 5;

    // delays for introductions
    constexpr int  INTRO_STATE_COUNT1 = (30*9);
    constexpr int  INTRO_STATE_COUNT2 = (30*11);
    constexpr int  INTRO_STATE_COUNT3 = (30*17);

    // player/jetski states
    constexpr int  JETSKI_OFF                = 0;
    constexpr int  JETSKI_START              = 1; 
    // constexpr int  JETSKI_IDLE               = 2;
    // constexpr int  JETSKI_ACCEL              = 3; 
    // constexpr int  JETSKI_FAST               = 4;

    // create some constants for ease of access
    constexpr int AMBIENT_LIGHT_INDEX = 0; // ambient light index
    constexpr int INFINITE_LIGHT_INDEX = 1; // infinite light index
    constexpr int POINT_LIGHT_INDEX = 2; // point light index

    // initialize camera position and direction
    POINT4D  cam_pos = {0,500, -200,1};
    POINT4D  cam_target = {0,0,0,1};
    VECTOR4D cam_dir = {0,0,0,1};
    // all your initialization code goes here...
    VECTOR4D vscale={1.0,1.0,1.0,1},
         vpos = {0,0,150,1}, 
         vrot = {0,0,0,1};

    CAM4DV1         cam;       // the single camera
    OBJECT4DV2      obj_terrain,
                    obj_player;    // the player object

    RENDERLIST4DV2 rend_list; // the render list

    // physical model defines play with these to change the feel of the vehicle
    float gravity       = -.30;    // general gravity
    float vel_y         = 0;       // the y velocity of camera/jetski
    float cam_speed     = 0;       // speed of the camera/jetski
    float sea_level     = 30;      // sea level of the simulation
    float gclearance    = 75;      // clearance from the camera to the ground/water
    float neutral_pitch = 1;       // the neutral pitch of the camera
    float turn_ang      = 0; 
    float jetski_yaw    = 0;

    float wave_count    = 0;       // used to track wave propagation
    int scroll_count    = 0;
    
    // game imagery assets
    BOB intro_image,
        ready_image, 
        nice_one_image;
 

    BITMAP_IMAGE background_bmp;   // holds the background
 
    ZBUFFERV1 zbuffer;             // out little z buffer

    int game_state         = GAME_STATE_INIT;
    int game_state_count1  = 0;

    int player_state       = JETSKI_OFF;
    // int player_state_count1 = 0;
    int enable_model_view   = 0;

    // initialize math engine
    Build_Sin_Cos_Tables();

    // initialize the camera with 90 FOV, normalized coordinates
    Init_CAM4DV1(&cam,      // the camera object
                CAM_MODEL_EULER,
                &cam_pos,  // initial camera position
                &cam_dir,  // initial camera angles
                &cam_target,
                10.0,      // near and far clipping planes
                12000.0,
                90.0,      // field of view in degrees
                engine.GetW(),   // size of final screen viewport
                engine.GetH());

    
    VECTOR4D terrain_pos = {0,0,0,0};

    // generate terrain
    Generate_Terrain_OBJECT4DV2(&obj_terrain,            // pointer to object
                            TERRAIN_WIDTH,           // width in world coords on x-axis
                            TERRAIN_HEIGHT,          // height (length) in world coords on z-axis
                            TERRAIN_SCALE,           // vertical scale of terrain
                            "media/water_track_height_04.bmp",  // filename of height bitmap encoded in 256 colors
                            "media/water_track_color_03.bmp",   // filename of texture map
                             _RGB32Bit(255,255,255),  // color of terrain if no texture        
                             &terrain_pos,           // initial position
                             NULL,                   // initial rotations
                             POLY4DV2_ATTR_RGB32 | 
                             POLY4DV2_ATTR_SHADE_MODE_FLAT | /*POLY4DV2_ATTR_SHADE_MODE_GOURAUD */
                             POLY4DV2_ATTR_SHADE_MODE_TEXTURE );

    // we are getting divide by zero errors due to degenerate triangles
    // after projection, this is a problem with the rasterizer that we will
    // fix later, but if we add a random bias to all y components of each vertice
    // is fixes the problem, its a hack, but hey no one is perfect :)
    for (int v = 0; v < obj_terrain.num_vertices; v++)
        obj_terrain.vlist_local[v].y+= ((float)RAND_RANGE(-5,5))/10;

    // load the jetski model for player
    VECTOR4D_INITXYZ(&vscale,17.5,17.50,17.50);  

    Load_OBJECT4DV2_COB(&obj_player, "media/jetski05.cob",  
                    &vscale, &vpos, &vrot, VERTEX_FLAGS_SWAP_YZ  |
                    VERTEX_FLAGS_TRANSFORM_LOCAL  | VERTEX_FLAGS_INVERT_TEXTURE_V
                    /* VERTEX_FLAGS_TRANSFORM_LOCAL_WORLD*/ );

    // set up lights
    Reset_Lights_LIGHTV2(lights2, MAX_LIGHTS);

    // create some working colors
    RGBAV1 /*white,*/ lightgray, gray, black/*, red, green, blue*/;

    // white.rgba = _RGBA32BIT(255,255,255,0);
    gray.rgba  = _RGBA32BIT(100,100,100,0);
    black.rgba = _RGBA32BIT(0,0,0,0);
    // red.rgba   = _RGBA32BIT(255,0,0,0);
    // green.rgba = _RGBA32BIT(0,255,0,0);
    lightgray.rgba = _RGBA32BIT(200,200,200,0);
    // blue.rgba  = _RGBA32BIT(0,0,255,0);

    // ambient light
    Init_Light_LIGHTV2(lights2,
                    AMBIENT_LIGHT_INDEX,   
                    LIGHTV2_STATE_ON,      // turn the light on
                    LIGHTV2_ATTR_AMBIENT,  // ambient light type
                    gray, black, black,    // color for ambient term only
                    NULL, NULL,            // no need for pos or dir
                    0,0,0,                 // no need for attenuation
                    0,0,0);                // spotlight info NA

    VECTOR4D dlight_dir = {-1,1,0,1};

    // directional light
    Init_Light_LIGHTV2(lights2,
                    INFINITE_LIGHT_INDEX,  
                    LIGHTV2_STATE_ON,      // turn the light on
                    LIGHTV2_ATTR_INFINITE, // infinite light type
                    black, lightgray, black,    // color for diffuse term only
                    NULL, &dlight_dir,     // need direction only
                    0,0,0,                 // no need for attenuation
                    0,0,0);                // spotlight info NA


    VECTOR4D plight_pos = {0,200,0,1};

    // point light
    Init_Light_LIGHTV2(lights2,
                    POINT_LIGHT_INDEX,
                    LIGHTV2_STATE_ON,      // turn the light on
                    LIGHTV2_ATTR_POINT,    // pointlight type
                    black, gray, black,   // color for diffuse term only
                    &plight_pos, NULL,     // need pos only
                    0,.002,0,              // linear attenuation only
                    0,0,1);                // spotlight info NA

    // load background image
    Load_Bitmap_File(&bitmap32bit, "media/cloud03.bmp");
    Create_Bitmap(&background_bmp, 0, 0, 800, 600, 32);
    Load_Image_Bitmap32(&background_bmp, &bitmap32bit, 0, 0, BITMAP_EXTRACT_MODE_ABS);
    Unload_Bitmap_File(&bitmap32bit);

    // load in all the text images

    // intro
    Load_Bitmap_File(&bitmap32bit, "media/zbufferwr_intro01.bmp");
    Create_BOB(&intro_image, engine.GetW()/2 - 560/2,40,560,160,1, 
        BOB_ATTR_VISIBLE | BOB_ATTR_SINGLE_FRAME, 0, 0, 32); 
    Load_Frame_BOB32(&intro_image, &bitmap32bit,0,0,0,BITMAP_EXTRACT_MODE_ABS);
    Unload_Bitmap_File(&bitmap32bit);

    // get ready
    Load_Bitmap_File(&bitmap32bit, "media/zbufferwr_ready01.bmp");
    Create_BOB(&ready_image, engine.GetW()/2 - 596/2,40,596,244,1, 
        BOB_ATTR_VISIBLE | BOB_ATTR_SINGLE_FRAME, 0, 0, 32); 
    Load_Frame_BOB32(&ready_image, &bitmap32bit,0,0,0,BITMAP_EXTRACT_MODE_ABS);
    Unload_Bitmap_File(&bitmap32bit);

    // nice one
    Load_Bitmap_File(&bitmap32bit, "media/zbufferwr_nice01.bmp");
    Create_BOB(&nice_one_image, engine.GetW()/2 - 588/2,40,588,92,1, 
        BOB_ATTR_VISIBLE | BOB_ATTR_SINGLE_FRAME, 0, 0, 32); 
    Load_Frame_BOB32(&nice_one_image, &bitmap32bit,0,0,0,BITMAP_EXTRACT_MODE_ABS);
    Unload_Bitmap_File(&bitmap32bit);

    // set single precission
    //_control87( _PC_24, MCW_PC );

    // allocate memory for zbuffer
    Create_Zbuffer(&zbuffer,
                engine.GetW(),
                engine.GetH(),
                ZBUFFER_ATTR_32BIT);

    // main loop
    while (!engine.ExitRequest()) {        
        static MATRIX4X4 mrot; // general rotation matrix

        // static float plight_ang = 0, 
                    // slight_ang = 0; // angles for light motion

        engine.ProcessEvents();
        engine.ClearEvents();
        
        // use these to rotate objects
        // static float x_ang = 0, y_ang = 0, z_ang = 0;

        // state variables for different rendering modes and help
        static int wireframe_mode = 1;
        static int backface_mode  = 1;
        static int lighting_mode  = 1;
        static int help_mode      = -1;
        static int zsort_mode     = -1;
        static int x_clip_mode    = 1;
        static int y_clip_mode    = 1;
        static int z_clip_mode    = 1;
        static int z_buffer_mode  = 1;
        static int display_mode   = 1;
        char work_string[256]; // temp string

        static int nice_one_on = 0; // used to display the nice one text
        static int nice_count1 = 0;

        // int index; // looping var

        // what state is the game in?
        switch (game_state) {
        case GAME_STATE_INIT:
        {
            // perform any important initializations
            // transition to restart
            game_state = GAME_STATE_RESTART;
        } break;
        case GAME_STATE_RESTART:
        {
            // reset all variables
            game_state_count1   = 0;
            player_state        = JETSKI_OFF;
            // player_state_count1 = 0;
            gravity             = -.30;    
            vel_y               = 0;       
            cam_speed           = 0;       
            sea_level           = 30;      
            gclearance          = 75;      
            neutral_pitch       = 10;   
            turn_ang            = 0; 
            jetski_yaw          = 0;
            wave_count          = 0;
            scroll_count        = 0;
            enable_model_view   = 0;

            // set camera high atop mount aleph one
            cam.pos.x = 1550;
            cam.pos.y = 800;
            cam.pos.z = 1950;
            cam.dir.y = -140;      

            // transition to introduction sub-state of run
            game_state = GAME_STATE_INTRO;
        } break;
        // in any of these cases enter into the main simulation loop
        case GAME_STATE_RUN:   
        case GAME_STATE_INTRO: 
        case GAME_STATE_PLAY:
        {
            // perform sub-state transition logic here
            if (game_state == GAME_STATE_INTRO) {
                // update timer
                ++game_state_count1;
                // test for first part of intro
                if (game_state_count1 == INTRO_STATE_COUNT1) {
                    // reposition camera in water
                    cam.pos.x = 444; // 560;
                    cam.pos.y = 200;
                    cam.pos.z = -534; // -580;
                    cam.dir.y = 45;// (-100);  

                    // enable model
                    enable_model_view = 1;
                } else if (game_state_count1 == INTRO_STATE_COUNT2) {
                } else if (game_state_count1 == INTRO_STATE_COUNT3) {
                    // reset counter for other use
                    game_state_count1 = 0;
                    // transition to play state
                    game_state = GAME_STATE_PLAY;  
                } // end if
            } // end if
            // start the timing clock
            Start_Clock();

            // reset the render list
            Reset_RENDERLIST4DV2(&rend_list);

            // modes and lights

            // wireframe mode
            if (engine.key()['w']) {
                // toggle wireframe mode
                if (++wireframe_mode > 1) wireframe_mode=0;
                Wait_Clock(100); // wait, so keyboard doesn't bounce
            } // end if

            // backface removal
            if (engine.key()['b']) {
                // toggle backface removal
                backface_mode = -backface_mode;
                Wait_Clock(100); // wait, so keyboard doesn't bounce
            } // end if

            // lighting
            if (engine.key()['l']) {
                // toggle lighting engine completely
                lighting_mode = -lighting_mode;
                Wait_Clock(100); // wait, so keyboard doesn't bounce
            } // end if

            // toggle ambient light
            if (engine.key()['a']) {
                // toggle ambient light
                if (lights2[AMBIENT_LIGHT_INDEX].state == LIGHTV2_STATE_ON)
                    lights2[AMBIENT_LIGHT_INDEX].state = LIGHTV2_STATE_OFF;
                else
                    lights2[AMBIENT_LIGHT_INDEX].state = LIGHTV2_STATE_ON;

                Wait_Clock(100); // wait, so keyboard doesn't bounce
            } // end if

            // toggle infinite light
            if (engine.key()['i']) {
                // toggle ambient light
                if (lights2[INFINITE_LIGHT_INDEX].state == LIGHTV2_STATE_ON)
                    lights2[INFINITE_LIGHT_INDEX].state = LIGHTV2_STATE_OFF;
                else
                    lights2[INFINITE_LIGHT_INDEX].state = LIGHTV2_STATE_ON;

                Wait_Clock(100); // wait, so keyboard doesn't bounce
            } // end if

            // toggle point light
            if (engine.key()['p']) {
                // toggle point light
                if (lights2[POINT_LIGHT_INDEX].state == LIGHTV2_STATE_ON)
                    lights2[POINT_LIGHT_INDEX].state = LIGHTV2_STATE_OFF;
                else
                    lights2[POINT_LIGHT_INDEX].state = LIGHTV2_STATE_ON;

                Wait_Clock(100); // wait, so keyboard doesn't bounce
            } // end if

            // help menu
            if (engine.key()['h']) {
                // toggle help menu 
                help_mode = -help_mode;
                Wait_Clock(100); // wait, so keyboard doesn't bounce
            } // end if

            // z-sorting
            if (engine.key()['s']) {
                // toggle z sorting
                zsort_mode = -zsort_mode;
                Wait_Clock(100); // wait, so keyboard doesn't bounce
            } // end if

            // z buffer
            if (engine.key()['z']) {
                // toggle z buffer
                z_buffer_mode = -z_buffer_mode;
                Wait_Clock(100); // wait, so keyboard doesn't bounce
            } // end if

            // display mode
            if (engine.key()['d']) {
                // toggle display mode
                display_mode = -display_mode;
                Wait_Clock(100); // wait, so keyboard doesn't bounce
            } // end if

            // PLAYER CONTROL AREA ////////////////////////////////////////////////////////////

            // filter player control if not in PLAY state
            if (game_state == GAME_STATE_PLAY) {
                // forward/backward
                if (engine.key()[SDL_SCANCODE_UP] && player_state >= JETSKI_START) {
                    // test for acceleration 
                    if (cam_speed == 0) {
                    } // end if
        
                    // move forward
                    cam_speed += 1.0;
                    if (cam_speed > MAX_SPEED) cam_speed = MAX_SPEED;
                } // end if

                // rotate around y axis or yaw
                if (engine.key()[SDL_SCANCODE_RIGHT]) {
                    cam.dir.y+=5;

                    if ((turn_ang+=JETSKI_TURN_RATE) > MAX_JETSKI_TURN)
                        turn_ang = MAX_JETSKI_TURN;

                    if ((jetski_yaw-=(JETSKI_YAW_RATE*cam_speed)) < -MAX_JETSKI_YAW)
                        jetski_yaw = -MAX_JETSKI_YAW;

                    // scroll the background
                    Scroll_Bitmap(&background_bmp, -10);
                } // end if

                if (engine.key()[SDL_SCANCODE_LEFT]) {
                    cam.dir.y-=5;

                    if ((turn_ang-=JETSKI_TURN_RATE) < -MAX_JETSKI_TURN)
                        turn_ang = -MAX_JETSKI_TURN;

                    if ((jetski_yaw+=(JETSKI_YAW_RATE*cam_speed)) > MAX_JETSKI_YAW)
                        jetski_yaw = MAX_JETSKI_YAW;

                    // scroll the background
                    Scroll_Bitmap(&background_bmp, 10);
                } // end if

                // is player trying to start jetski?
                if ( (player_state == JETSKI_OFF) && 
                    (engine.key()[SDL_SCANCODE_RETURN] || engine.key()[']'])) {
                    // start jetski
                    player_state = JETSKI_START;
                } // end if 

            } // end if controls enabled

            // turn JETSKI straight
            if (turn_ang > JETSKI_TURN_RETURN_RATE) 
                turn_ang-=JETSKI_TURN_RETURN_RATE;
            else if (turn_ang < -JETSKI_TURN_RETURN_RATE) 
                turn_ang+=JETSKI_TURN_RETURN_RATE;
            else 
                turn_ang = 0;

            // turn JETSKI straight
            if (jetski_yaw > JETSKI_YAW_RETURN_RATE) 
                jetski_yaw-=JETSKI_YAW_RETURN_RATE;
            else if (jetski_yaw < -JETSKI_YAW_RETURN_RATE)
                jetski_yaw+=JETSKI_YAW_RETURN_RATE;
            else
                jetski_yaw = 0;

            // reset the object (this only matters for backface and object removal)
            Reset_OBJECT4DV2(&obj_terrain);

            // perform world transform to terrain now, so we can destroy the transformed
            // coordinates with the wave function
            Model_To_World_OBJECT4DV2(&obj_terrain, TRANSFORM_LOCAL_TO_TRANS);

            // apply wind effect ////////////////////////////////////////////////////

            // scroll the background
            if (++scroll_count > 5)  {
                Scroll_Bitmap(&background_bmp, -1);
                scroll_count = 0;
            } // end if

            // wave generator ////////////////////////////////////////////////////////

            // for each vertex in the mesh apply wave modulation if height < water level
            for (int v = 0; v < obj_terrain.num_vertices; v++) {
                // wave modulate below water level only
                if (obj_terrain.vlist_trans[v].y < WATER_LEVEL)
                    obj_terrain.vlist_trans[v].y
                        += WAVE_HEIGHT * sin(wave_count 
                                + (float)v / (2*(float)obj_terrain.ivar2+0));
            } // end for v

            // increase the wave position in time
            wave_count+=WAVE_RATE;

            // motion section /////////////////////////////////////////////////////////

            // terrain following, simply find the current cell we are over and then
            // index into the vertex list and find the 4 vertices that make up the 
            // quad cell we are hovering over and then average the values, and based
            // on the current height and the height of the terrain push the player upward

            // the terrain generates and stores some results to help with terrain following
            //ivar1 = columns;
            //ivar2 = rows;
            //fvar1 = col_vstep;
            //fvar2 = row_vstep;

            int cell_x = (cam.pos.x  + TERRAIN_WIDTH/2) / obj_terrain.fvar1;
            int cell_y = (cam.pos.z  + TERRAIN_HEIGHT/2) / obj_terrain.fvar1;

            static float terrain_height, delta;

            // test if we are on terrain
            if ((cell_x >=0) 
                && (cell_x < obj_terrain.ivar1) 
                && (cell_y >=0) 
                && (cell_y < obj_terrain.ivar2) ) {

                // compute vertex indices into vertex list of the current quad
                int v0 = cell_x + cell_y*obj_terrain.ivar2;
                int v1 = v0 + 1;
                int v2 = v1 + obj_terrain.ivar2;
                int v3 = v0 + obj_terrain.ivar2;   

                // now simply index into table 
                terrain_height = 0.25 * (obj_terrain.vlist_trans[v0].y 
                                + obj_terrain.vlist_trans[v1].y
                                + obj_terrain.vlist_trans[v2].y 
                                + obj_terrain.vlist_trans[v3].y);

                // compute height difference
                delta = terrain_height - (cam.pos.y - gclearance);

                // test for penetration
                if (delta > 0) {
                    // apply force immediately to camera (this will give it a springy feel)
                    vel_y+=(delta * (VELOCITY_SCALER));

                    // test for pentration, if so move up immediately so we don't penetrate geometry
                    cam.pos.y+=(delta*CAM_HEIGHT_SCALER);

                    // now this is more of a hack than the physics model :) let move the front
                    // up and down a bit based on the forward velocity and the gradient of the 
                    // hill
                    cam.dir.x -= (delta*PITCH_CHANGE_RATE); 
                } // end if

            } // end if

            // decelerate camera
            if (cam_speed > (CAM_DECEL) ) 
                cam_speed-=CAM_DECEL;
            else if (cam_speed < (-CAM_DECEL) ) 
                cam_speed+=CAM_DECEL;
            else
                cam_speed = 0;

            // force camera to seek a stable orientation
            if (cam.dir.x > (neutral_pitch+PITCH_RETURN_RATE)) 
                cam.dir.x -= (PITCH_RETURN_RATE);
            else if (cam.dir.x < (neutral_pitch-PITCH_RETURN_RATE)) 
                cam.dir.x += (PITCH_RETURN_RATE);
            else 
                cam.dir.x = neutral_pitch;

            // apply gravity
            vel_y+=gravity;

            // test for absolute sea level and push upward..
            if (cam.pos.y < sea_level) { 
                vel_y = 0; 
                cam.pos.y = sea_level;
            } // end if

            // move camera
            cam.pos.x += cam_speed*Fast_Sin(cam.dir.y);
            cam.pos.z += cam_speed*Fast_Cos(cam.dir.y);
            cam.pos.y += vel_y;

            // position point light in front of player
            lights2[POINT_LIGHT_INDEX].pos.x = cam.pos.x + 130*Fast_Sin(cam.dir.y);
            lights2[POINT_LIGHT_INDEX].pos.y = cam.pos.y + 50;
            lights2[POINT_LIGHT_INDEX].pos.z = cam.pos.z + 130*Fast_Cos(cam.dir.y);

            // position the player object slighty in front of camera and in water
            obj_player.world_pos.x = cam.pos.x + (130+cam_speed*1.75)*Fast_Sin(cam.dir.y);
            obj_player.world_pos.y = cam.pos.y - 25 + 7.5*sin(wave_count);
            obj_player.world_pos.z = cam.pos.z + (130+cam_speed*1.75)*Fast_Cos(cam.dir.y);

            // make splash sound if altitude changed enough
            if ((fabs(delta) > 30) 
                && (cam_speed >= (.75*MAX_SPEED)) 
                && ((rand()%20)==1) ) {

                // display nice one!
                nice_one_on = 1; 
                nice_count1 = 60;
            } // end if 


            // begin 3D rendering section ///////////////////////////////////////////////////////

            // generate camera matrix
            Build_CAM4DV1_Matrix_Euler(&cam, CAM_ROT_SEQ_ZYX);

            // generate rotation matrix around y axis
            //Build_XYZ_Rotation_MATRIX4X4(x_ang, y_ang, z_ang, &mrot);

            // rotate the local coords of the object
            //Transform_OBJECT4DV2(&obj_terrain, &mrot, TRANSFORM_LOCAL_TO_TRANS,1);


            // insert terrain object into render list
            Insert_OBJECT4DV2_RENDERLIST4DV2(&rend_list, &obj_terrain,0);

            //MAT_IDENTITY_4X4(&mrot);

            // generate rotation matrix around y axis
            Build_XYZ_Rotation_MATRIX4X4(-cam.dir.x*1.5, cam.dir.y+turn_ang, jetski_yaw, &mrot);

            // rotate the local coords of the object
            Transform_OBJECT4DV2(&obj_player, &mrot, TRANSFORM_LOCAL_TO_TRANS,1);

            // perform world transform
            Model_To_World_OBJECT4DV2(&obj_player, TRANSFORM_TRANS_ONLY);

            // don't show model unless in play state
            if (enable_model_view) {
                // insert the object into render list
                Insert_OBJECT4DV2_RENDERLIST4DV2(&rend_list, &obj_player,0);
            } // end if


            // reset number of polys rendered
            debug_polys_rendered_per_frame = 0;
            debug_polys_lit_per_frame = 0;

            // remove backfaces
            if (backface_mode==1)
                Remove_Backfaces_RENDERLIST4DV2(&rend_list, &cam);

            // apply world to camera transform
            World_To_Camera_RENDERLIST4DV2(&rend_list, &cam);

            // clip the polygons themselves now
            Clip_Polys_RENDERLIST4DV2(&rend_list, &cam, ((x_clip_mode == 1) ? CLIP_POLY_X_PLANE : 0) | 
                                                ((y_clip_mode == 1) ? CLIP_POLY_Y_PLANE : 0) | 
                                                ((z_clip_mode == 1) ? CLIP_POLY_Z_PLANE : 0) );

            // light scene all at once 
            if (lighting_mode==1) {
                Transform_LIGHTSV2(lights2, 3, &cam.mcam, TRANSFORM_LOCAL_TO_TRANS);
                Light_RENDERLIST4DV2_World2_16(&rend_list, &cam, lights2, 3);
            } // end if

            // sort the polygon list (hurry up!)
            if (zsort_mode == 1)
                Sort_RENDERLIST4DV2(&rend_list,  SORT_POLYLIST_AVGZ);

            // apply camera to perspective transformation
            Camera_To_Perspective_RENDERLIST4DV2(&rend_list, &cam);

            // apply screen transform
            Perspective_To_Screen_RENDERLIST4DV2(&rend_list, &cam);

            engine.BeginRenderToBuffer(false);
            // draw background
            Draw_Bitmap32(&background_bmp, back_buffer, back_lpitch,0);

            // render the object
            if (wireframe_mode  == 0) {
                Clear_Zbuffer(&zbuffer, (32000 << FIXP16_SHIFT));
                Draw_RENDERLIST4DV2_SolidZB32(&rend_list, back_buffer, 
                    back_lpitch, (UCHAR *)zbuffer.zbuffer, engine.GetW()*4);

                Draw_RENDERLIST4DV2_Wire32(&rend_list, back_buffer, back_lpitch);
            } else if (wireframe_mode  == 1) {
                if (z_buffer_mode == 1) {
                    // initialize zbuffer to 32000 fixed point
                    //Mem_Set_QUAD((void *)zbuffer, (32000 << FIXP16_SHIFT), (WINDOW_WIDTH*WINDOW_HEIGHT) ); 
                    Clear_Zbuffer(&zbuffer, (32000 << FIXP16_SHIFT));
                    Draw_RENDERLIST4DV2_SolidZB32(&rend_list, back_buffer, 
                        back_lpitch, (UCHAR *)zbuffer.zbuffer, engine.GetW()*4);
                } else {
                    // initialize zbuffer to 32000 fixed point
                    Draw_RENDERLIST4DV2_Solid32(&rend_list, back_buffer, back_lpitch);
                } 
            } // end if

            // dislay z buffer graphically (sorta)
            if (display_mode==-1) {
                // use z buffer visualization mode
                // copy each line of the z buffer into the back buffer and translate each z pixel
                // into a color
                USHORT *screen_ptr = (USHORT *)back_buffer;
                UINT   *zb_ptr    =  (UINT *)zbuffer.zbuffer;


                for (int y = 0; y < engine.GetH(); y++) {
                    for (int x = 0; x < engine.GetW(); x++) {
                        // z buffer is 32 bit, so be carefull
                        UINT zpixel = zb_ptr[x + y*engine.GetW()];
                        zpixel = (zpixel/4096 & 0xffff);
                        screen_ptr[x + y* (back_lpitch >> 1)] = (USHORT)zpixel;
                    } // end for
                } // end for y
            } // end if

            engine.EndRenderToBuffer();
        
            // draw overlay text
            if (game_state == GAME_STATE_INTRO 
                &&  game_state_count1 < INTRO_STATE_COUNT1) {
                Draw_BOB32(&intro_image, engine);
            } else if (game_state == GAME_STATE_INTRO 
                &&  game_state_count1 > INTRO_STATE_COUNT2) {
                Draw_BOB32(&ready_image, engine);
            } // end if

            // check for nice one display
            if (nice_one_on == 1 &&  game_state == GAME_STATE_PLAY) {
                // are we done displaying?
                if (--nice_count1 <=0)
                    nice_one_on = 0;

                Draw_BOB32(&nice_one_image, engine);
            } // end if draw nice one

            // draw statistics 
            sprintf(work_string,"Lighting [%s]: Ambient=%d, Infinite=%d, "
                "Point=%d, BckFceRM [%s], Zsort [%s], Zbuffer [%s]", 
                ((lighting_mode == 1) ? "ON" : "OFF"),
                lights2[AMBIENT_LIGHT_INDEX].state,
                lights2[INFINITE_LIGHT_INDEX].state, 
                lights2[POINT_LIGHT_INDEX].state,
                ((backface_mode == 1) ? "ON" : "OFF"),
                ((zsort_mode == 1) ? "ON" : "OFF"),
                ((z_buffer_mode == 1) ? "ON" : "OFF"));

            engine.Text(work_string, 0, engine.GetH()-34-16);

            // draw instructions
            engine.Text("Press ESC to exit. Press <H> for Help.", 0, 0);

            // should we display help
            int text_y = 16;
            if (help_mode==1) {
                // draw help menu
                engine.Text("<A>..............Toggle ambient light source.", 0, text_y+=12);
                engine.Text("<I>..............Toggle infinite light source.", 0, text_y+=12);
                engine.Text("<P>..............Toggle point light source.", 0, text_y+=12);
                engine.Text("<N>..............Next object.", 0, text_y+=12);
                engine.Text("<W>..............Toggle wire frame/solid mode.", 0, text_y+=12);
                engine.Text("<B>..............Toggle backface removal.", 0, text_y+=12);
                engine.Text("<S>..............Toggle Z sorting.", 0, text_y+=12);
                engine.Text("<Z>..............Toggle Z buffering.", 0, text_y+=12);
                engine.Text("<D>..............Toggle Normal 3D display / Z buffer visualization mode.", 0, text_y+=12);
                engine.Text("<H>..............Toggle Help.", 0, text_y+=12);
                engine.Text("<ESC>............Exit demo.", 0, text_y+=12);
            } // end help

            sprintf(work_string,"Polys Rendered: %d, Polys lit: %d", 
                debug_polys_rendered_per_frame, debug_polys_lit_per_frame);
            engine.Text(work_string, 0, engine.GetH()-34-16-16);

            sprintf(work_string,"CAM [%5.2f, %5.2f, %5.2f @ %5.2f]",
                cam.pos.x, cam.pos.y, cam.pos.z, cam.dir.y);
            engine.Text(work_string, 0, engine.GetH()-34-16-16-16);

            // flip the surfaces
            engine.Update();

            // sync to 30ish fps
            Wait_Clock(30);
            // end main simulation loop
        } break;
        case GAME_STATE_EXIT:
        {
            // do any cleanup here, and exit
        } break;

        default: break;
        } // end switch game_state

        // Engine pipeline end         
        std::this_thread::yield();
    }
}

int main(int argc, char** argv)
{
    try {
        GfxSystem engine(1024, 748);

        std::cout << "start...\n";
        
        if (argc > 1) {
            if (strcmp(argv[1], "1") == 0) {
                Demo1(engine);
            } else if (strcmp(argv[1], "2") == 0) {
                Demo2(engine);
            } else if (strcmp(argv[1], "3") == 0) {
                Demo3(engine);
            } else if (strcmp(argv[1], "4") == 0) {
                Demo4(engine);
            } else if (strcmp(argv[1], "5") == 0) {
                Demo5(engine);
            } else if (strcmp(argv[1], "6") == 0) {
                Demo6(engine);
            } else if (strcmp(argv[1], "7") == 0) {
                Demo7(engine);
            } else if (strcmp(argv[1], "8") == 0) {
                Demo8(engine);
            } else if (strcmp(argv[1], "9") == 0) {
                Demo9(engine);
            } else if (strcmp(argv[1], "10") == 0) {
                Demo10(engine);
            } else if (strcmp(argv[1], "11") == 0) {
                Demo11(engine);
            } else if (strcmp(argv[1], "12") == 0) {
                Demo12(engine);
            } else if (strcmp(argv[1], "13") == 0) {
                Demo13(engine);
            } else if (strcmp(argv[1], "14") == 0) {
                Demo14(engine);
            }
        } else {
            Demo14(engine);
        }
    } catch (const std::exception& ex) {
        std::cout << "Exception thrown: " << ex.what() << std::endl;
    }

    return 0;
}