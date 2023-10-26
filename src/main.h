#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <SDL2/SDL.h>

#include "camera.h"

class State {
    EditorCameraRig m_rig;
    bool m_highlight = false;

public:
    State() = default;

    EditorCameraRig &rig() { return m_rig; }
    const EditorCameraRig &rig() const { return m_rig; }
    bool &highlight() { return m_highlight; }
    const bool &highlight() const { return m_highlight; }

    void handle_event(const SDL_Event &event);
};

#endif
