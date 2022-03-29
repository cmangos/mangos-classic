//
// Copyright (c) 2009-2010 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "SDL.h"
#include "SDL_opengl.h"
#include "SDL_main.h"
#include "imgui.h"
#include "imguiRenderGL.h"
#include "Recast.h"
#include "RecastDebugDraw.h"
#include "Filelist.h"
#include <filesystem>

//#include "Sample_SoloMesh.h"
//#include "Sample_TileMesh.h"
#include "CMaNGOS_Map.h"
//#include "Sample_TempObstacles.h"
//#include "Sample_Debug.h"

#ifdef WIN32
#	define snprintf _snprintf
#	define putenv _putenv
#endif

void printUsage()
{
    printf("Generator command line args\n\n");
    printf("-? or /? or -h or --help: Show this help\n");
    printf("--dataDir [directory] : Path to base directory of maps/vmaps/mmap subfolder.\n");
    printf("                        Please not that folder with space or special characters are not supported.\n");
}

bool handleArgs(int argc, char** argv, char*& dataDir)
{
    char* param = NULL;
    dataDir = "./";

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--dataDir") == 0 || strcmp(argv[i], "-d") == 0)
        {
            param = argv[++i];
            if (!param)
            {
                printUsage();
                return false;
            }

            dataDir = param;
        }
    }

    return true;
}

int main(int argc, char** argv)
{
    // Init SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("Could not initialize SDL\n");
        return -1;
    }

    char* dataDir = nullptr;

    if (!handleArgs(argc, argv, dataDir))
        return -1;

    // Center window
    char env[] = "SDL_VIDEO_CENTERED=1";
    putenv(env);

    // Init OpenGL
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    //#ifndef WIN32
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    //#endif

    const SDL_VideoInfo* vi = SDL_GetVideoInfo();

    int width, height;
    SDL_Surface* screen = 0;

    /*width = vi->current_w;
    height = vi->current_h;
    screen = SDL_SetVideoMode(width, height, 0, SDL_OPENGL | SDL_FULLSCREEN);*/

    width = rcMin(vi->current_w, (int)(vi->current_h * 16.0 / 9.0));
    width = width - 80;
    height = vi->current_h - 80;
    screen = SDL_SetVideoMode(width, height, 0, SDL_OPENGL);

    if (!screen)
    {
        printf("Could not initialize SDL OpenGL\n");
        return -1;
    }

    glEnable(GL_MULTISAMPLE);

    SDL_WM_SetCaption("RecastDemo Mod (for CMaNGOS)", 0);

    cout << "Current working directory: " << std::filesystem::current_path() << endl;

    if (!imguiRenderGLInit("DroidSans.ttf"))
    {
        printf("Could not init GUI renderer.\n");
        SDL_Quit();
        return -1;
    }

    float t = 0.0f;
    float timeAcc = 0.0f;
    Uint32 lastTime = SDL_GetTicks();
    int mx = 0, my = 0;
    float rx = 45;
    float ry = -45;
    float moveW = 0, moveS = 0, moveA = 0, moveD = 0;
    float camx = 0, camy = 0, camz = 0, camr = 1000;
    float origrx = 0, origry = 0;
    int origx = 0, origy = 0;
    float scrollZoom = 0;
    bool rotate = false;
    bool movedDuringRotate = false;
    float rays[3], raye[3];
    bool mouseOverMenu = false;
    bool showMenu = true;
    bool showLog = false;
    bool showTools = true;

    int propScroll = 0;
    int logScroll = 0;
    int toolsScroll = 0;

    FileList files;

    float mpos[3] = { 0, 0, 0 };
    bool mposSet = false;

    //InputGeom* geom = 0;
    CMaNGOS_Map* sample = new CMaNGOS_Map();
    BuildContext ctx(dataDir);
    sample->setContext(&ctx);
    sample->Init();
    if (sample->GetMapId() < 0)
    {
        printf("Could not init data. Maps folders exist?\n");
        SDL_Quit();
        return -1;
    }
    glEnable(GL_CULL_FACE);

    float fogCol[4] = { 0.32f, 0.31f, 0.30f, 1.0f };
    /*glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, camr*0.1f);
    glFogf(GL_FOG_END, camr*1.25f);
    glFogfv(GL_FOG_COLOR, fogCol);
    glFogf(GL_FOG_DENSITY, 0.01f);*/
    glDepthFunc(GL_LEQUAL);

    bool done = false;

    while (!done)
    {
        // Handle input events.
        int mscroll = 0;
        bool processHitTest = false;
        bool processHitTestShift = false;
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_KEYDOWN:
                    // Handle any key presses here.
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                    {
                        done = true;
                    }
                    else if (event.key.keysym.sym == SDLK_TAB)
                    {
                        showMenu = !showMenu;
                    }/*
                    else if (event.key.keysym.sym == SDLK_SPACE)
                    {
                        if (sample)
                            sample->handleToggle();
                    }*/
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_RIGHT)
                    {
                        if (!mouseOverMenu)
                        {
                            // Rotate view
                            rotate = true;
                            movedDuringRotate = false;
                            origx = mx;
                            origy = my;
                            origrx = rx;
                            origry = ry;
                        }
                    }
                    else if (event.button.button == SDL_BUTTON_WHEELUP)
                    {
                        if (mouseOverMenu)
                            mscroll--;
                        else
                            scrollZoom -= 5.0f;
                    }
                    else if (event.button.button == SDL_BUTTON_WHEELDOWN)
                    {
                        if (mouseOverMenu)
                            mscroll++;
                        else
                            scrollZoom += 5.0f;
                    }
                    break;

                case SDL_MOUSEBUTTONUP:
                    // Handle mouse clicks here.
                    if (event.button.button == SDL_BUTTON_RIGHT)
                    {
                        rotate = false;
                        if (!mouseOverMenu)
                        {
                            if (!movedDuringRotate)
                            {
                                processHitTest = true;
                                processHitTestShift = true;
                            }
                        }
                    }
                    else if (event.button.button == SDL_BUTTON_LEFT)
                    {
                        if (!mouseOverMenu)
                        {
                            processHitTest = true;
                            processHitTestShift = (SDL_GetModState() & KMOD_SHIFT) ? true : false;
                        }
                    }

                    break;

                case SDL_MOUSEMOTION:
                    mx = event.motion.x;
                    my = height - 1 - event.motion.y;
                    if (rotate)
                    {
                        int dx = mx - origx;
                        int dy = my - origy;
                        rx = origrx - dy*0.25f;
                        ry = origry + dx*0.25f;
                        if (dx*dx + dy*dy > 3 * 3)
                            movedDuringRotate = true;
                    }
                    break;

                case SDL_QUIT:
                    done = true;
                    break;

                default:
                    break;
            }
        }

        unsigned char mbut = 0;
        if (SDL_GetMouseState(0, 0) & SDL_BUTTON_LMASK)
            mbut |= IMGUI_MBUT_LEFT;
        if (SDL_GetMouseState(0, 0) & SDL_BUTTON_RMASK)
            mbut |= IMGUI_MBUT_RIGHT;

        Uint32	time = SDL_GetTicks();
        float	dt = (time - lastTime) / 1000.0f;
        lastTime = time;

        t += dt;

        // Hit test mesh.
        if (processHitTest)
        {
            float hitt;
            bool hit = sample->GetMapInfos()->RaycastMesh(rays, raye, hitt);

            if (hit)
            {
                if ((SDL_GetModState() & KMOD_CTRL) && (SDL_GetModState() & KMOD_SHIFT))
                {
                    // Marker
                    mposSet = true;
                    mpos[0] = rays[0] + (raye[0] - rays[0])*hitt;
                    mpos[1] = rays[1] + (raye[1] - rays[1])*hitt;
                    mpos[2] = rays[2] + (raye[2] - rays[2])*hitt;
                }
                else
                {
                    float pos[3];
                    pos[0] = rays[0] + (raye[0] - rays[0])*hitt;
                    pos[1] = rays[1] + (raye[1] - rays[1])*hitt;
                    pos[2] = rays[2] + (raye[2] - rays[2])*hitt;
                    sample->handleClick(rays, pos, processHitTestShift, (SDL_GetModState() & KMOD_CTRL));
                }
            }
            else
            {
                mposSet = false;
            }
        }

        // Update sample simulation.
        const float SIM_RATE = 20;
        const float DELTA_TIME = 1.0f / SIM_RATE;
        timeAcc = rcClamp(timeAcc + dt, -1.0f, 1.0f);
        int simIter = 0;
        while (timeAcc > DELTA_TIME)
        {
            timeAcc -= DELTA_TIME;
            if (simIter < 5)
            {
                if (sample)
                    sample->handleUpdate(DELTA_TIME);
            }
            simIter++;
        }

        // Clamp the framerate so that we do not hog all the CPU.
        const float MIN_FRAME_TIME = 1.0f / 40.0f;
        if (dt < MIN_FRAME_TIME)
        {
            int ms = (int)((MIN_FRAME_TIME - dt)*1000.0f);
            if (ms > 10) ms = 10;
            if (ms >= 0)
                SDL_Delay(ms);
        }


        // Update and render
        glViewport(0, 0, width, height);
        glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_TEXTURE_2D);

        // Render 3d
        glEnable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(50.0f, (float)width / (float)height, 1.0f, camr+10000);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glRotatef(rx, 1, 0, 0);
        glRotatef(ry, 0, 1, 0);
        glTranslatef(-camx, -camy, -camz);

        // Get hit ray position and direction.
        GLdouble proj[16];
        GLdouble model[16];
        GLint view[4];
        glGetDoublev(GL_PROJECTION_MATRIX, proj);
        glGetDoublev(GL_MODELVIEW_MATRIX, model);
        glGetIntegerv(GL_VIEWPORT, view);
        GLdouble x, y, z;
        gluUnProject(mx, my, 0.0f, model, proj, view, &x, &y, &z);
        rays[0] = (float)x; rays[1] = (float)y; rays[2] = (float)z;
        gluUnProject(mx, my, 1.0f, model, proj, view, &x, &y, &z);
        raye[0] = (float)x; raye[1] = (float)y; raye[2] = (float)z;

        // Handle keyboard movement.
        Uint8* keystate = SDL_GetKeyState(NULL);
        moveW = rcClamp(moveW + dt * 4 * ((keystate[SDLK_w] || keystate[SDLK_UP]) ? 1 : -1), 0.0f, 1.0f);
        moveS = rcClamp(moveS + dt * 4 * ((keystate[SDLK_s] || keystate[SDLK_DOWN]) ? 1 : -1), 0.0f, 1.0f);
        moveA = rcClamp(moveA + dt * 4 * ((keystate[SDLK_a] || keystate[SDLK_LEFT]) ? 1 : -1), 0.0f, 1.0f);
        moveD = rcClamp(moveD + dt * 4 * ((keystate[SDLK_d] || keystate[SDLK_RIGHT]) ? 1 : -1), 0.0f, 1.0f);

        float keybSpeed = 22.0f;
        if (SDL_GetModState() & KMOD_SHIFT)
            keybSpeed *= 15.0f;

        float movex = (moveD - moveA) * keybSpeed * dt;
        float movey = (moveS - moveW) * keybSpeed * dt;

        movey += scrollZoom * 2.0f;
        scrollZoom = 0;

        camx += movex * (float)model[0];
        camy += movex * (float)model[4];
        camz += movex * (float)model[8];

        camx += movey * (float)model[2];
        camy += movey * (float)model[6];
        camz += movey * (float)model[10];

        //glEnable(GL_FOG);

        if (sample)
            sample->handleRender();

        //glDisable(GL_FOG);

        // Render GUI
        glDisable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(0, width, 0, height);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        mouseOverMenu = false;

        imguiBeginFrame(mx, my, mbut, mscroll);

        if (sample)
        {
            sample->handleRenderOverlay((double*)proj, (double*)model, (int*)view);
        }

        // Help text.
        if (showMenu)
        {
            const char msg[] = "W/S/A/D: Move  RMB: Rotate";
            imguiDrawText(280, 35, IMGUI_ALIGN_LEFT, msg, imguiRGBA(255, 255, 255, 128));
            const char msg2[] = "CTRL + LMB to select a tile (to load/buil/clear navmesh)";
            imguiDrawText(280, 20, IMGUI_ALIGN_LEFT, msg2, imguiRGBA(255, 255, 255, 128));
        }

        if (showMenu)
        {
            if (imguiBeginScrollArea("Properties", width - 250 - 10, 10, 250, height - 20, &propScroll))
                mouseOverMenu = true;

            if (imguiCheck("Show Log", showLog))
                showLog = !showLog;
            if (imguiCheck("Show Tools", showTools))
                showTools = !showTools;

            imguiSeparator();

            if (sample)
            {
                sample->handleExtraSettings();
                sample->handleSettings();
            }

            /*if (geom && sample)
            {
                imguiSeparatorLine();


                if (imguiButton("Build"))
                {
                    ctx.resetLog();
                    if (!sample->handleBuild())
                    {
                        showLog = true;
                        logScroll = 0;
                    }
                    ctx.dumpLog("Build log %s:", meshName);
                }

                imguiSeparator();
            }*/

            if (sample)
            {
                imguiSeparatorLine();
                sample->handleDebugMode();
            }

            imguiEndScrollArea();
        }

        if (sample && sample->ShowLevel(height, width))
            mouseOverMenu = true;

        if (sample->GeomChanged())
        {
            const float* bmin = 0;
            const float* bmax = 0;
            sample->GetGeomBounds(bmin, bmax);

            // Reset camera and fog to match the mesh bounds.
            if (bmin && bmax)
            {
                camr = sqrtf(rcSqr(bmax[0] - bmin[0]) +
                    rcSqr(bmax[1] - bmin[1]) +
                    rcSqr(bmax[2] - bmin[2])) / 2;
                camx = (bmax[0] + bmin[0]) / 2 + camr;
                camy = (bmax[1] + bmin[1]) / 2 + camr;
                camz = (bmax[2] + bmin[2]) / 2 + camr;
                camr *= 3;
            }
            rx = 45;
            ry = -45;/*
            glFogf(GL_FOG_START, camr*0.1f);
            glFogf(GL_FOG_END, camr*1.25f);*/
        }

        // Log
        if (showLog && showMenu)
        {
            if (imguiBeginScrollArea("Log", 250 + 20, 10, width - 300 - 250, 200, &logScroll))
                mouseOverMenu = true;
            for (int i = 0; i < ctx.getLogCount(); ++i)
                imguiLabel(ctx.getLogText(i));
            imguiEndScrollArea();
        }

        // Tools
        if (showTools && showMenu) // && geom && sample)
        {
            if (imguiBeginScrollArea("Tools", 10, 10, 250, height - 20, &toolsScroll))
                mouseOverMenu = true;

            if (sample)
                sample->handleTools();

            imguiEndScrollArea();
        }

        // Marker
        if (mposSet && gluProject((GLdouble)mpos[0], (GLdouble)mpos[1], (GLdouble)mpos[2],
            model, proj, view, &x, &y, &z))
        {
            // Draw marker circle
            glLineWidth(5.0f);
            glColor4ub(240, 220, 0, 196);
            glBegin(GL_LINE_LOOP);
            const float r = 25.0f;
            for (int i = 0; i < 20; ++i)
            {
                const float a = (float)i / 20.0f * RC_PI * 2;
                const float fx = (float)x + cosf(a)*r;
                const float fy = (float)y + sinf(a)*r;
                glVertex2f(fx, fy);
            }
            glEnd();
            glLineWidth(1.0f);
        }

        imguiEndFrame();
        imguiRenderGLDraw();

        glEnable(GL_DEPTH_TEST);
        SDL_GL_SwapBuffers();
    }

    imguiRenderGLDestroy();

    SDL_Quit();

    delete sample;

    return 0;
}
