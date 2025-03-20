#if !defined HANDMADE_H


// [NOTE] Services that the game provides to the platform layer.
// (This may expand in the future, i.e., sound on separate thread.)

// Timing, controller/keybaord input, bitmap buffer to use, sound buffer to use.

struct game_offscreen_buffer
{
    BITMAPINFO info;
    void*      memory;
    i32        width;
    i32        height;
    i32        pitch;
};

internal void game_update_and_render(game_offscreen_buffer, int blue_offset, int green_offset);

#define HANDMADE_H
#endif