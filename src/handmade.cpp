#include "handmade.h"

internal void
render_weird_gradient(game_offscreen_buffer* buffer, i32 blue_offset, i32 green_offset)
{
    u8* row = (u8*)buffer->memory;

    for (i32 y = 0; y < buffer->height; y++)
    {
        u32* pixel = (u32*)row;

        for (i32 x = 0; x < buffer->width; x++)
        {
            // Pixel in memory: BB GG RR xx where xx is padding, BB is blue, GG
            // is green, and RR is red.
            u8 blue  = x + blue_offset;
            u8 green = y + green_offset;

            *pixel++ = (green << 8) | blue;
        }

        row += buffer->pitch;
    }
}

internal void
game_update_and_render(game_offscreen_buffer* buffer, int blue_offset, int green_offset)
{
    render_weird_gradient(buffer, blue_offset, green_offset);
}