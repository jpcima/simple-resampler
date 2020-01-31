#include "resampler.h"
#include <sndfile.hh>
#include <getopt.h>
#include <memory>
#include <cmath>
#include <cstdio>
#include <cstdlib>

int main(int argc, char *argv[])
{
    const char *in_path = nullptr;
    const char *out_path = nullptr;
    double ratio = 0;

    for (int c; (c = getopt(argc, argv, "i:o:r:")) != -1;) {
        switch (c) {
        case 'i':
            in_path = optarg;
            break;
        case 'o':
            out_path = optarg;
            break;
        case 'r':
            ratio = atof(optarg);
            break;
        default:
            return 1;
        }
    }

    if (!in_path) {
        fprintf(stderr, "No input file given (-i).\n");
        return 1;
    }
    if (!out_path) {
        fprintf(stderr, "No input file given (-o).\n");
        return 1;
    }
    if (ratio <= 0) {
        fprintf(stderr, "No ratio given (-r).\n");
        return 1;
    }

    SndfileHandle snd_in{in_path};
    if (!snd_in) {
        fprintf(stderr, "Cannot open the input file: %s.\n", in_path);
        return 1;
    }

    unsigned samplerate = snd_in.samplerate();
    unsigned channels = snd_in.channels();

    SndfileHandle snd_out{out_path, SFM_WRITE, SF_FORMAT_WAV|SF_FORMAT_PCM_16, (int)channels, (int)samplerate};
    if (!snd_out) {
        fprintf(stderr, "Cannot open the output file: %s.\n", out_path);
        return 1;
    }

    size_t in_frames = snd_in.frames();
    size_t out_frames = (size_t)std::ceil(in_frames * ratio);

    float *in = new float[in_frames * channels]{};
    snd_in.readf(in, in_frames);
    float *out = new float[out_frames * channels]{};

    ///
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

    snd_out.writef(out, out_frames);

    return 0;
}
