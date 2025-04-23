
struct win32_offscreen_buffer
{
    BITMAPINFO info;
    void*      memory;
    i32        width;
    i32        height;
    i32        pitch;
};

struct win32_window_dimensions
{
    i32 width;
    i32 height;
};

struct win32_sound_output
{
    i32 samples_per_second;
    u32 running_sample_index;
    i32 bytes_per_sample;
    i32 secondary_buffer_size;
    f32 t_sine;
    i32 latency_sample_count;
};
