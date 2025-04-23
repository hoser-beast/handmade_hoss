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
game_update_and_render(game_memory* memory, game_input* input, game_offscreen_buffer* buffer,
                       game_sound_output_buffer* sound_buffer)
{
    assert(sizeof(game_state) <= memory->permanent_storage_size);

    game_state* state = (game_state*)memory->permanent_storage;
    if (!memory->is_initialized)
    {
        state->tone_hz         = 256;
        memory->is_initialized = true;    // [TODO] This may be more appropriate to do in the platform layer.
    }

    game_controller_input* input0 = &input->controllers[0];
    if (input0->is_analog)
    {
        state->tone_hz      = 256 + (int)(128.0f * input0->end_x);
        state->blue_offset += (int)4.0f * (input0->end_y);
    }
    else
    {
        // [TODO] Use digital movement tuning.
    }

    if (input0->down.ended_down)
    {
        state->green_offset += 1;
    }

    // [TODO] Allow sample offset here for more robust platform options.
    game_output_sound(sound_buffer, state->tone_hz);
    render_weird_gradient(buffer, state->blue_offset, state->green_offset);
}
