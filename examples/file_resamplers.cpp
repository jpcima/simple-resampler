#include "file_resamplers.h"
#include "resampler.h"
#include <soxr.h>
#include <samplerate.h>
#include <speex/speex_resampler.h>

void resample_with_mine(
    double input_rate, double output_rate,
    const float *in, size_t in_frames,
    float *out, size_t out_frames,
    unsigned channels)
{
    double ratio = output_rate / input_rate;
    uint32_t i_in = 0;
    uint32_t i_out = 0;
    auto getNextFrame = [&i_in, in, in_frames, channels](float *frame) {
        if (i_in < in_frames) {
            for (uint32_t c = 0; c < channels; ++c)
                frame[c] = in[c + i_in * channels];
            ++i_in;
        }
        else {
            for (uint32_t c = 0; c < channels; ++c)
                frame[c] = 0;
        }
    };
    auto putNextFrame = [&i_out, out, channels](const float *frame) {
        for (uint32_t c = 0; c < channels; ++c)
            out[c + i_out * channels] = frame[c];
        ++i_out;
    };

    ///
    switch (channels) {
    default:
        fprintf(stderr, "Unsupported number of audio channels: %u\n", channels);
        break;
    case 1: {
        Resampler<1> rsm;
        rsm.setup(ratio);
        rsm.resample(getNextFrame, putNextFrame, out_frames);
        break;
    }
    case 2: {
        Resampler<2> rsm;
        rsm.setup(ratio);
        rsm.resample(getNextFrame, putNextFrame, out_frames);
        break;
    }
    case 4: {
        Resampler<4> rsm;
        rsm.setup(ratio);
        rsm.resample(getNextFrame, putNextFrame, out_frames);
        break;
    }
    case 8: {
        Resampler<8> rsm;
        rsm.setup(ratio);
        rsm.resample(getNextFrame, putNextFrame, out_frames);
        break;
    }
    }
}

static void resample_with_sox(
    soxr_quality_spec_t quality_spec,
    double input_rate, double output_rate,
    const float *in, size_t in_frames,
    float *out, size_t out_frames,
    unsigned channels)
{
    soxr_io_spec_t io_spec = soxr_io_spec(SOXR_FLOAT32_I, SOXR_FLOAT32_I);
    soxr_runtime_spec_t runtime_spec = soxr_runtime_spec(2);

    size_t idone = 0;
    size_t odone = 0;
    soxr_error_t err = soxr_oneshot(input_rate, output_rate, channels, in,
                                    in_frames, &idone, out, out_frames, &odone,
                                    &io_spec, &quality_spec, &runtime_spec);
    if (err)
        fprintf(stderr, "Error from SoX resampler: %s\n", err);
}

void resample_with_sox_vhq(
    double input_rate, double output_rate,
    const float *in, size_t in_frames,
    float *out, size_t out_frames,
    unsigned channels)
{
    resample_with_sox(
        soxr_quality_spec(SOXR_VHQ, 0),
        input_rate, output_rate,
        in, in_frames, out, out_frames, channels);
}

void resample_with_sox_mq(
    double input_rate, double output_rate,
    const float *in, size_t in_frames,
    float *out, size_t out_frames,
    unsigned channels)
{
    resample_with_sox(
        soxr_quality_spec(SOXR_MQ, 0),
        input_rate, output_rate,
        in, in_frames, out, out_frames, channels);
}

void resample_with_sox_lq(
    double input_rate, double output_rate,
    const float *in, size_t in_frames,
    float *out, size_t out_frames,
    unsigned channels)
{
    resample_with_sox(
        soxr_quality_spec(SOXR_LQ, 0),
        input_rate, output_rate,
        in, in_frames, out, out_frames, channels);
}

static void resample_with_src(
    int converter_type,
    double input_rate, double output_rate,
    const float *in, size_t in_frames,
    float *out, size_t out_frames,
    unsigned channels)
{
    SRC_DATA src_data = {};
    src_data.data_in = in;
    src_data.data_out = out;
    src_data.input_frames = in_frames;
    src_data.output_frames = out_frames;
    src_data.src_ratio = output_rate / input_rate;

    int err = src_simple(&src_data, converter_type, channels);
    if (err != 0)
        fprintf(stderr, "Error from Secret Rabbit Code resampler: %s\n", src_strerror(err));
}

void resample_with_src_best(
    double input_rate, double output_rate,
    const float *in, size_t in_frames,
    float *out, size_t out_frames,
    unsigned channels)
{
    resample_with_src(
        SRC_SINC_BEST_QUALITY,
        input_rate, output_rate,
        in, in_frames, out, out_frames, channels);
}

void resample_with_src_medium(
    double input_rate, double output_rate,
    const float *in, size_t in_frames,
    float *out, size_t out_frames,
    unsigned channels)
{
    resample_with_src(
        SRC_SINC_MEDIUM_QUALITY,
        input_rate, output_rate,
        in, in_frames, out, out_frames, channels);
}

void resample_with_src_fastest(
    double input_rate, double output_rate,
    const float *in, size_t in_frames,
    float *out, size_t out_frames,
    unsigned channels)
{
    resample_with_src(
        SRC_SINC_FASTEST,
        input_rate, output_rate,
        in, in_frames, out, out_frames, channels);
}

static void resample_with_speex(
    int quality,
    double input_rate, double output_rate,
    const float *in, size_t in_frames,
    float *out, size_t out_frames,
    unsigned channels)
{
    int err = 0;
    SpeexResamplerState *src = speex_resampler_init(
        channels, input_rate, output_rate, quality, &err);
    if (!src) {
        fprintf(stderr, "Error from Speex resampler: %s\n", speex_resampler_strerror(err));
        return;
    }

    speex_resampler_skip_zeros(src);

    spx_uint32_t in_spx = in_frames;
    spx_uint32_t out_spx = out_frames;

    speex_resampler_process_interleaved_float(
        src, in, &in_spx, out, &out_spx);

    speex_resampler_destroy(src);
}

void resample_with_speex_mq(
    double input_rate, double output_rate,
    const float *in, size_t in_frames,
    float *out, size_t out_frames,
    unsigned channels)
{
    resample_with_speex(
        SPEEX_RESAMPLER_QUALITY_DEFAULT,
        input_rate, output_rate,
        in, in_frames, out, out_frames, channels);
}

void resample_with_speex_vhq(
    double input_rate, double output_rate,
    const float *in, size_t in_frames,
    float *out, size_t out_frames,
    unsigned channels)
{
    resample_with_speex(
        SPEEX_RESAMPLER_QUALITY_MAX,
        input_rate, output_rate,
        in, in_frames, out, out_frames, channels);
}

void resample_with_linear(
    double input_rate, double output_rate,
    const float *in, size_t in_frames,
    float *out, size_t out_frames,
    unsigned channels)
{
    for (size_t out_index = 0; out_index < out_frames; ++out_index) {
        double frac = out_index * ((double)in_frames / out_frames);

        size_t in_index1 = (size_t)frac;
        size_t in_index2 = in_index1 + 1;
        double mu = frac - in_index1;

        for (unsigned c = 0; c < channels; ++c) {
            double sample1 = 0;
            double sample2 = 0;

            if (in_index1 < in_frames)
                sample1 = in[c + channels * in_index1];
            if (in_index2 < in_frames)
                sample2 = in[c + channels * in_index2];

            out[c + channels * out_index] = mu * sample2 + (1 - mu) * sample1;
        }
    }
}
