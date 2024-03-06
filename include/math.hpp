#pragma once
// File to hold includes for GLM because for some reason double includes aren't protected... 
// Maybe `#pragma once` isn't in some important file

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
// #include "glm/ext.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtc/matrix_transform.hpp"
