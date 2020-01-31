#pragma once
#include <array>
#include <cmath>
#include <cstdint>

/**
   Convolution-based realtime resampler

   This resampler convolves the input samples with a lowpass kernel evaluated
   at fractional intermediate points, depending on the output position.

   `Nch` number of channels
   `Ksize` convolution size (higher = more quality, latency, computation)
   `Kover` oversampling of the lookup table, to lookup fractional values
 */
template <uint32_t Nch, uint32_t Ksize = 8, uint32_t Kover = 64>
class Resampler {
public:
    /**
       Set the ratio of rate conversion: ratio = Fs_out/Fs_in.
     */
    void setup(double ratio);

    /**
       Compute the next resampled block.

       `getNext` function which reads the next input frame
       `putNext` function which writes the next output frame
       `putCount` number of frames to write to the output
     */
    template <class G, class P>
    void resample(const G &getNext, const P &putNext, uint32_t putCount);

    /**
       Get the latency introduced by this resampler, in frames.
     */
    constexpr uint32_t latency() const { return Ksize / 2; }

private:
    typedef std::array<float, Ksize> Krow;
    typedef std::array<Krow, Kover> Kmat;

    /**
       Matrix of convolution kernels, of Kover rows and Ksize columns
       It is a kernel of size (Kover x Ksize) stored in column-major order.
       Each row is for a different fractional offset (0 <= frac < 1).
     */
    static const Kmat sKernel;
    static Kmat makeLanczosKernel();

    /**
       Increment of the fractional input position every output frame
     */
    double fIncrPos = 1;

    /**
       Current fractional position over input signal
     */
    double fFracPos = 0;

    /**
       The history index points into the storage to the last Ksize samples of
       signal.
     */
    uint32_t fHistoryIndex = 0;

    /**
       Storage for a history of Ksize samples
       The second part [Ksize:2*Ksize-1] is a duplicate of [0:Ksize-1].
       (vectorization purposes)
     */
    std::array<float, 2 * Ksize> fHistory[Nch] = {};
};

#include "resampler.tcc"
