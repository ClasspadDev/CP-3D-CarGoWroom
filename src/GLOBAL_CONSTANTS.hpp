// ----------------------------------------------------
// Project global defines.
// This file is included during compilation using
// "-include ./src/GLOBAL_CONSTANTS.hpp"
//
// Vscode autocomplete include fix c_cpp_properties.json:
//  "forcedInclude": ["${workspaceFolder}/src/GLOBAL_CONSTANTS.hpp"]
// ----------------------------------------------------

// Libfixmath (fixpoint library defines)
#define FIXMATH_NO_CACHE
#define FIXMATH_NO_CTYPE
#define FIXMATH_NO_64BIT
#define FIXMATH_NO_HARD_DIVISION
#define FIXMATH_SATURATING_ARITHMETIC

#ifdef PC
#   define WINDOW_SIZE_MULTIPLIER 3.5f
#endif

#define ROTATION_VISUALIZER_LINE_WIDTH  20.0f
#define ROTATION_VISALIZER_EDGE_OFFSET  15

// Landscape or Portrait mode (portrait mode may have missing features at this point)
#define LANDSCAPE_MODE

// 3 screen clearing options:
//  PER_MODEL_CLEAR   -> Bounding box per model (Best with caveat of maybe ending up with lots of overlaping clears which may end up slowing down actually)
//  (none)            -> Bounding box created using all models
//  CLEAR_FULL_SCREEN -> Always clears full screen
//
// Either making a global clearing bound box or per model.
// Per model works well if there are lots of tiny objects
// with lots of space inbetween. However if they start to over-lap alot then
// we end up clearing same location multiple times which wastes time.
// Hence option to clear per model.
#   define PER_MODEL_CLEAR
// When defined then full screen will be cleared instead of bound box based clearing
// -> Slowest option
// #   define CLEAR_FULL_SCREEN

#define SCREEN_X 320
#define SCREEN_Y 528

#ifdef PC
#   define UNIVERSIAL_FILE_READ O_RDONLY
#else
#   define UNIVERSIAL_FILE_READ OPEN_READ
#endif