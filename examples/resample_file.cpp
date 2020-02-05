#include "file_resamplers.h"
#include <sndfile.hh>
#include <getopt.h>
#include <memory>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

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
        fprintf(stderr, "No output file given (-o).\n");
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

    size_t in_frames = snd_in.frames();
    size_t out_frames = (size_t)std::ceil(in_frames * ratio);

    float *in = new float[in_frames * channels]{};
    snd_in.readf(in, in_frames);

    for (const ResamplingChoice &rc : sResamplingChoices) {
        std::string this_out_path;

        this_out_path = out_path;
        if (strcmp(rc.name, "mine") != 0) {
            this_out_path += '.';
            this_out_path += rc.name;
        }

        fprintf(stderr, "* Resample (%s): %s\n", rc.name, this_out_path.c_str());

        SndfileHandle snd_out{this_out_path.c_str(), SFM_WRITE, SF_FORMAT_WAV|SF_FORMAT_PCM_16, (int)channels, (int)samplerate};
        if (!snd_out) {
            fprintf(stderr, "Cannot open the output file: %s.\n", out_path);
            return 1;
        }

        float *out = new float[out_frames * channels]{};

        rc.resample(
            samplerate, ratio * samplerate,
            in, in_frames, out, out_frames, channels);

        snd_out.writef(out, out_frames);
    }

    return 0;
}
