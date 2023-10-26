#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <SDL2/SDL.h>

#include "camera.h"

class State {
    std::unique_ptr<CameraRig> m_rig;
    bool m_highlight = false;

public:
    State(std::unique_ptr<CameraRig> &&rig) : m_rig{std::move(rig)} {}

    CameraRig &rig() { return *m_rig; }
    const CameraRig &rig() const { return *m_rig; }

    bool &highlight() { return m_highlight; }
    const bool &highlight() const { return m_highlight; }

    void handle_event(const SDL_Event &event);
    void ticker(const uint8_t *keystate);
};

#endif
