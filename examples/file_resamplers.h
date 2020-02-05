#pragma once
#include <array>
#include <cstddef>

void resample_with_mine(
    double input_rate, double output_rate,
    const float *in, size_t in_frames,
    float *out, size_t out_frames,
    unsigned channels);

void resample_with_sox_vhq(
    double input_rate, double output_rate,
    const float *in, size_t in_frames,
    float *out, size_t out_frames,
    unsigned channels);

void resample_with_linear(
    double input_rate, double output_rate,
    const float *in, size_t in_frames,
    float *out, size_t out_frames,
    unsigned channels);

typedef void (resample_file_t)(
    double input_rate, double output_rate,
    const float *in, size_t in_frames,
    float *out, size_t out_frames,
    unsigned channels);

struct ResamplingChoice {
    const char *name;
    resample_file_t *resample;
};

static std::array<ResamplingChoice, 3> sResamplingChoices {{
    {"mine", &resample_with_mine},
    {"soxvhq", &resample_with_sox_vhq},
    {"linear", &resample_with_linear},
}};
