#include "resampler.h"

template <uint32_t Nch, uint32_t Ksize, uint32_t Kover>
const typename Resampler<Nch, Ksize, Kover>::Kmat Resampler<Nch, Ksize, Kover>::sKernel = makeLanczosKernel();

template <uint32_t Nch, uint32_t Ksize, uint32_t Kover>
void Resampler<Nch, Ksize, Kover>::setup(double ratio)
{
    fIncrPos = 1.0 / ratio;
}

template <uint32_t Nch, uint32_t Ksize, uint32_t Kover>
template <class G, class P>
void Resampler<Nch, Ksize, Kover>::resample(const G &getNext, const P &putNext, uint32_t putCount)
{
    double incrPos = fIncrPos;
    double fracPos = fFracPos;
    uint32_t historyIndex = fHistoryIndex;

    for (uint32_t i = 0; i < putCount; ++i) {
        fracPos += incrPos;

        while (fracPos >= 1.0) {
            std::array<float, Nch> next;
            getNext(next.data());

            for (uint32_t c = 0; c < Nch; ++c) {
                std::array<float, 2 * Ksize> &hist = fHistory[c];
                hist[historyIndex] = next[c];
                hist[historyIndex + Ksize] = next[c];
            }

            historyIndex = (historyIndex + 1) % Ksize;
            fracPos -= 1.0;
        }

        const Krow &row = sKernel[(uint32_t)(fracPos * Kover)];

        std::array<float, Nch> out;

        for (uint32_t c = 0; c < Nch; ++c) {
            std::array<float, 2 * Ksize> &hist = fHistory[c];
            float s = 0;
            for (uint32_t i = 0; i < Ksize; ++i)
                s += row[i] * hist[historyIndex + i];
            out[c] = s;
        }

        putNext(out.data());
    }

    fracPos = fFracPos;
    fHistoryIndex = historyIndex;
}

template <uint32_t Nch, uint32_t Ksize, uint32_t Kover>
auto Resampler<Nch, Ksize, Kover>::makeLanczosKernel() -> Kmat
{
    auto sinc = [](double x) -> double
    {
        if (x == 0)
            return 1;
        return std::sin(M_PI * x) / (M_PI * x);
    };

    Kmat mat;
    for (uint32_t o = 0; o < Kover; ++o) {
        Krow &row = mat[o];
        double offset = o / (double)Kover;
        double sum = 0;
        for (uint32_t i = 0; i < Ksize; ++i) {
            double a = 0.5 * (Ksize - 1);
            double x = i - a - offset;
            double k = 0;
            if (x > -a && x < a)
                k = sinc(x / a) * sinc(x);
            row[i] = k;
            sum += k;
        }
        if (0) {
            for (uint32_t i = 0; i < Ksize; ++i)
                row[i] /= sum; // normalize for unity gain
        }
    }
    return mat;
}
