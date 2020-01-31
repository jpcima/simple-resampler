#include "resampler.h"

template <uint32_t Ksize, uint32_t Kover>
const typename Resampler<Ksize, Kover>::Kmat Resampler<Ksize, Kover>::sKernel = makeLanczosKernel();

template <uint32_t Ksize, uint32_t Kover>
void Resampler<Ksize, Kover>::setup(double ratio)
{
    fIncrPos = 1.0 / ratio;
}

template <uint32_t Ksize, uint32_t Kover>
template <class G, class P>
void Resampler<Ksize, Kover>::resample(const G &getNext, const P &putNext, uint32_t putCount)
{
    double incrPos = fIncrPos;
    double fracPos = fFracPos;
    unsigned historyIndex = fHistoryIndex;

    for (uint32_t i = 0; i < putCount; ++i) {
        fracPos += incrPos;
        while (fracPos >= 1.0) {
            float next = getNext();
            fHistory[historyIndex] = next;
            fHistory[historyIndex + Ksize] = next;
            historyIndex = (historyIndex + 1) % Ksize;
            fracPos -= 1.0;
        }

        const Krow &row = sKernel[(uint32_t)(fracPos * Kover)];

        float s = 0;
        float ks = 0;
        for (uint32_t i = 0; i < Ksize; ++i) {
            float k = row[i];
            s += k * fHistory[historyIndex + i];
            ks += k;
        }
        s /= ks;

        putNext(s);
    }

    fracPos = fFracPos;
    fHistoryIndex = historyIndex;
}

template <uint32_t Ksize, uint32_t Kover>
auto Resampler<Ksize, Kover>::makeLanczosKernel() -> Kmat
{
    auto sinc = [](double x) -> double
    {
        if (x == 0)
            return 1;
        return std::sin(x) / (M_PI * x);
    };

    Kmat mat;
    for (uint32_t o = 0; o < Kover; ++o) {
        Krow &row = mat[o];
        double offset = o / (double)Kover;
        for (uint32_t i = 0; i < Ksize; ++i) {
            double x = i - 0.5 * (Ksize - 1) - offset;
            double k = sinc(x) / sinc(x / Ksize);
            row[i] = k;
        }
    }
    return mat;
}
