/*
 * Copyright (C) 2020 Claudio Castiglia
 *
 * This file is part of caio.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see http://www.gnu.org/licenses/
 */
#include "signal.hpp"

#include <fstream>
#include <sstream>

#include <gsl/span>


namespace caio {
namespace signal {

std::random_device rd{};
std::uniform_real_distribution<float> uni_random{-1.0f, 1.0f};


float blackman(size_t pos, size_t N)
{
    if (N <= 1) {
        return 1.0f;
    }

    const float k = static_cast<float>(pos) / static_cast<float>(N - 1);
    return (0.42f - 0.5f * std::cos(2.0f * M_PI * k) + 0.08f * std::cos(4.0f * M_PI * k));
}

void spectral_inversion(samples_fp& krn)
{
    std::for_each(krn.begin(), krn.end(), [](float& value) {
        value = -value;
    });

    const size_t c = krn.size() >> 1;
    krn[c] += 1.0f;
}

samples_fp conv(samples_fp& dst, const samples_fp& sig, const samples_fp& krn, enum ConvShape shape)
{
    const size_t Ns = sig.size();
    const size_t Nk = krn.size();
    const size_t Nc = Ns + Nk - 1;
    const size_t Nf = (shape == ConvShape::Central ? Ns : Nc);

    if (dst.size() < Nf) {
        std::ostringstream os{};
        os << "Required buffer size " << Nf << ", provided buffer size " << dst.size();
        throw InvalidArgument{__FUNCTION__, os.str()};
    }

    float cdata[Nc];
    samples_fp c{cdata, Nc};
    std::fill_n(c.begin(), Nc, 0.0f);

    //TODO optimise
    for (size_t ix = 0; ix < Ns; ++ix) {
        for (size_t iy = 0; iy < Nk; ++iy) {
            c[ix + iy] += sig[ix] * krn[iy];
        }
    }

    if (shape == ConvShape::Central) {
        auto from = c.begin() + ((Nc - Ns) >> 1);
        auto to = from + Ns;
        std::copy(from, to, dst.begin());
    } else {
        std::copy(c.begin(), c.end(), dst.begin());
    }

    return samples_fp{dst.data(), Nf};
}

samples_fp lopass(samples_fp& krn, float fc, float fs, bool osiz)
{
    size_t N;
    if (osiz) {
        N = kernel_size(fc, fs);
        if (N > krn.size()) {
            std::ostringstream os{};
            os << "fc " << fc << ", fs " << fs << ", krn size " << krn.size() << ", required size " << N;
            throw InvalidArgument{__FUNCTION__, os.str()};
        }
    } else {
        N = krn.size();
    }

    const float w = 2.0f * M_PI * fc;
    const float ts = 1.0f / fs;

    float t = -ts * static_cast<float>(N >> 1);
    float sum = 0.0f;

    //TODO optimise
    for (size_t k = 0; k < N; ++k, t += ts) {
        float value = sinc(w * t) * blackman(k, N);
        krn[k] = value;
        sum += value;
    }

    std::for_each(krn.begin(), krn.end(), [&sum](float& value) {
        value /= sum;
    });

    return krn.subspan(0, N);
}

samples_fp hipass(samples_fp& krn, float fc, float fs, bool osiz)
{
    auto hi_krn = lopass(krn, fc, fs, osiz);
    spectral_inversion(hi_krn);
    return hi_krn;
}

samples_fp bapass(samples_fp& krn, float fcl, float fch, float fs, bool osiz)
{
//FIXME test
#if 0
    float fhi = std::min(fcl, fch);
    float flo = std::max(fcl, fch);

    size_t N = kernel_size(flo, fs);
    float lodata[N];
    samples_fp lo{lodata, N};
    auto lo_krn = lopass(lo, flo, fs, false);

    N = kernel_size(fhi, fs);
    float hidata[N];
    samples_fp hi{hidata, N};
    auto hi_krn = hipass(hi, fhi, fs, false);

    auto ba_krn = conv(krn, lo_krn, hi_krn, ConvShape::Central);
    return ba_krn;
#else
    float fhi = std::min(fcl, fch);
    float flo = std::max(fcl, fch);
    float w0  = M_PI * (fhi + flo);

    size_t N = kernel_size(flo, fs);
    float data[N];
    samples_fp lo{data, N};
    auto ba_krn = lopass(lo, flo, fs, false);

    for (size_t k = 0; k < ba_krn.size(); ++k) {
        ba_krn[k] *= 2.0f * std::cos(w0 * k);
    }

    return ba_krn;
#endif
}

samples_fp bastop(samples_fp& krn, float fcl, float fch, float fs, bool osiz)
{
    auto sb = bapass(krn, fcl, fch, fs, osiz);
    spectral_inversion(sb);
    return sb;
}

samples_fp lopass_40(samples_fp& krn, float f0, float Q, float fs, bool osiz)
{
    size_t N;
    if (osiz) {
        N = kernel_size(f0, fs);
        if (N > krn.size()) {
            std::ostringstream os{};
            os << "f0 " << f0 << ", fs " << fs << ", krn size " << krn.size() << ", required size " << N;
            throw InvalidArgument{__FUNCTION__, os.str()};
        }
    } else {
        N = krn.size();
    }

    auto hN = N >> 1;

    /*
     * FT = sqrt(1 - 1 / (4 * Q^2));
     * w0 = 2 * pi * f0;
     * w  = FT * w0;
     * a  = w0 / (2 * Q);
     * v  = (w0 / FT) * e.^(-a * t) .* sin(w .* t);
     * v  ./= sum(v);
     */
    const float FT = std::sqrt(1.0f - 1.0f / (4.0f * Q * Q));
    const float w0 = 2 * M_PI * f0;
    const float w  = w0 * FT;
    const float a  = w0 / (2.0f * Q);
    const float A  = w0 / FT;

    const float ts = 1.0f / fs;
    float t        = 0.0f;
    float sum      = 0.0f;

    std::fill(krn.begin(), krn.end(), 0.0f);

    //TODO optimise
    for (size_t k = hN; k < N; ++k, t += ts) {
        float value = A * std::exp(-a * t) * std::sin(w * t) * blackman(k, N);
        krn[k] = value;
        sum += value;
    }

    std::for_each(krn.begin(), krn.end(), [&sum](float& value) {
        value /= sum;
    });

    auto qp = krn.subspan(0, N);
    return qp;
}

samples_fp hipass_40(samples_fp& krn, float f0, float Q, float fs, bool osiz)
{
    auto iqp = lopass_40(krn, f0, Q, fs, osiz);
    spectral_inversion(iqp);
    return iqp;
}

samples_fp lopass_20(samples_fp& krn, float f0, float fs, bool osiz)
{
    size_t N;
    if (osiz) {
        N = kernel_size(f0, fs);
        if (N > krn.size()) {
            std::ostringstream os{};
            os << "f0 " << f0 << ", fs " << fs << ", krn size " << krn.size() << ", required size " << N;
            throw InvalidArgument{__FUNCTION__, os.str()};
        }
    } else {
        N = krn.size();
    }

    /*
     * ts = 1 / fs;
     * w0 = 2 * pi * f0;
     * t  = [0 : ts : 1 - ts];
     * v  = w0 * e.^(-w0 * t)
     * v  ./= sum(v);
     */
    const float ts = 1.0f / fs;
    const float w0 = 2 * M_PI * f0;
    float t        = 0.0f;
    float sum      = 0.0f;

    auto hN = N >> 1;

    std::fill(krn.begin(), krn.end(), 0.0f);

    //TODO optimise
    for (size_t k = hN; k < N; ++k, t += ts) {
        float value = std::exp(-w0 * t) * blackman(k, N);
        krn[k] = value;
        sum += value;
    }

    std::for_each(krn.begin(), krn.end(), [&sum](float& value) {
        value /= sum;
    });

    auto lo20 = krn.subspan(0, N);
    return lo20;
}

samples_fp hipass_20(samples_fp& krn, float f0, float fs, bool osiz)
{
    auto z = lopass_20(krn, f0, fs, osiz);
    spectral_inversion(z);
    return z;
}

samples_fp bapass_20(samples_fp& krn, float f0, float fs, bool osiz)
{
    size_t N = kernel_size(f0, fs);
    float lodata[N];
    samples_fp lo{lodata, N};
    auto lo_krn = lopass_20(lo, f0, fs, false);

    float hidata[N];
    samples_fp hi{hidata, N};
    auto hi_krn = hipass_20(hi, f0, fs, false);

    auto tri_krn = conv(lo_krn, lo_krn, hi_krn, ConvShape::Central);
    return tri_krn;
}

std::ostream& dump(std::ostream& os, const samples_fp& samples, const std::string& name, float fs,
    float fc1, float fc2, float Q)
{
    os << name << " = struct('fs', "   << fs  << ", "
               <<           "'fc1', "  << fc1 << ", "
               <<           "'fc2', "  << fc2 << ", "
               <<           "'Q', "    << Q   << ", "
               <<           "'v', [ ";

    std::for_each(samples.begin(), samples.end(), [&os](const float& value) {
        os << value << " ";
    });

    os << " ]);\n";

    return os;
}

}
}
