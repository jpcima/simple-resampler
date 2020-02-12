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

void resample_with_sox_mq(
    double input_rate, double output_rate,
    const float *in, size_t in_frames,
    float *out, size_t out_frames,
    unsigned channels);

void resample_with_sox_lq(
    double input_rate, double output_rate,
    const float *in, size_t in_frames,
    float *out, size_t out_frames,
    unsigned channels);

void resample_with_src_best(
    double input_rate, double output_rate,
    const float *in, size_t in_frames,
    float *out, size_t out_frames,
    unsigned channels);

void resample_with_src_medium(
    double input_rate, double output_rate,
    const float *in, size_t in_frames,
    float *out, size_t out_frames,
    unsigned channels);

void resample_with_src_fastest(
    double input_rate, double output_rate,
    const float *in, size_t in_frames,
    float *out, size_t out_frames,
    unsigned channels);

void resample_with_speex_mq(
    double input_rate, double output_rate,
    const float *in, size_t in_frames,
    float *out, size_t out_frames,
    unsigned channels);

void resample_with_speex_vhq(
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

static std::array<ResamplingChoice, 10> sResamplingChoices {{
    {"mine", &resample_with_mine},
    {"soxvhq", &resample_with_sox_vhq},
    {"soxmq", &resample_with_sox_mq},
    {"soxlq", &resample_with_sox_lq},
    {"srcvhq", &resample_with_src_best},
    {"srcmq", &resample_with_src_medium},
    {"srclq", &resample_with_src_fastest},
    {"speexmq", &resample_with_speex_mq},
    {"speexvhq", &resample_with_speex_vhq},
    {"linear", &resample_with_linear},
}};
