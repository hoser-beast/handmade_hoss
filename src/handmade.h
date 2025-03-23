#if !defined HANDMADE_H
#define HANDMADE_H

#define array_count(array) (sizeof(array) / sizeof(array[0]))
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

struct game_sound_output_buffer
{
    i32  samples_per_second;
    i32  sample_count;
    i16* samples;
};

struct game_button_state
{
    i32 half_transition_count;
    b32 ended_down;
};

struct game_controller_input
{
    b32 is_analog;

    f32 start_x;
    f32 start_y;

    f32 min_x;
    f32 min_y;

    f32 max_x;
    f32 max_y;

    f32 end_x;
    f32 end_y;

    union
    {
        game_button_state buttons[6];
        struct
        {
            game_button_state up;
            game_button_state down;
            game_button_state left;
            game_button_state right;
            game_button_state left_shoulder;
            game_button_state right_shoulder;
        };
    };
};

struct game_input
{
    game_controller_input controllers[4];
};

internal void
game_update_and_render(game_input* input, game_offscreen_buffer* buffer, game_sound_output_buffer* sound_buffer);

#endif