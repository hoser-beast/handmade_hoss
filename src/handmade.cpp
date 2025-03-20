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
game_output_sound(game_sound_output_buffer* sound_buffer, i32 tone_hz)
{
    local_persist f32 t_sine;
    i16               tone_volume = 1024;
    i32               wave_period = sound_buffer->samples_per_second / tone_hz;

    i16* sample_out = sound_buffer->samples;
    for (DWORD sample_index = 0; sample_index < sound_buffer->sample_count; sample_index++)
    {
        f32 sine_value   = sinf(t_sine);
        i16 sample_value = (i16)(sine_value * tone_volume);

        *sample_out++ = sample_value;
        *sample_out++ = sample_value;

        t_sine += 2.0f * PI32 * 1.0f / (f32)wave_period;
    }
}

internal void
game_update_and_render(game_offscreen_buffer* buffer, int blue_offset, int green_offset,
                       game_sound_output_buffer* sound_buffer, i32 tone_hz)
{
    // [TODO] Allow sample offset here for more robust platform options.
    game_output_sound(sound_buffer, tone_hz);
    render_weird_gradient(buffer, blue_offset, green_offset);
}