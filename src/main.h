#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <SDL2/SDL.h>

#include "camera.h"

class State {
    EditorCameraRig m_rig;

public:
    State() = default;

    EditorCameraRig &rig() { return m_rig; }
    const EditorCameraRig &rig() const { return m_rig; }

    void handle_event(const SDL_Event &event);
};

#endif
