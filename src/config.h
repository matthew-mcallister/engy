#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#include <numbers>

const char *const WINDOW_TITLE = "engy";
const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;

const float FOVY = 45 * (std::numbers::pi / 180);
const float Z_NEAR = 0.1;
const float Z_FAR = 10;

#endif
