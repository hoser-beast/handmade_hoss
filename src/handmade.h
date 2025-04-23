#if !defined HANDMADE_H
#define HANDMADE_H

// [NOTE]
// HANDMADE_INTERNAL: 0 = public release; 1 = developer release
// HANDMADE_DEBUG:    0 = fast code;      1 = slow code

#if HANDMADE_DEBUG
#define assert(x)           \
    do                      \
    {                       \
        if (!(x))           \
        {                   \
            __debugbreak(); \
        }                   \
    } while (0)
#else
#define assert(x)
#endif

#define KILOBYTES(x) ((x) * 1024UL)
#define MEGABYTES(x) (KILOBYTES(x) * 1024UL)
#define GIGABYTES(x) (MEGABYTES(x) * 1024ULL)
#define TERABYTES(x) (GIGABYTES(x) * 1024ULL)

#define array_count(array) (sizeof(array) / sizeof(array[0]))
// [NOTE] Services that the game provides to the platform layer.
// (This may expand in the future, i.e., sound on separate thread.)

// Timing, controller/keyboard input, bitmap buffer to use, sound buffer to use.

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
    // [TODO] Insert clock values here.
    game_controller_input controllers[4];
};

struct game_state
{
    i32 tone_hz;
    i32 green_offset;
    i32 blue_offset;
};

struct game_memory
{
    b32 is_initialized;

    u64   permanent_storage_size;
    void* permanent_storage;    // [NOTE] *REQUIRED* to be cleared to zero at start-up.

    u64   transient_storage_size;
    void* transient_storage;
};

internal void game_update_and_render(game_memory* memory, game_input* input, game_offscreen_buffer* buffer,
                                     game_sound_output_buffer* sound_buffer);

#endif