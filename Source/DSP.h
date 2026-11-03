// HADES by GERMO - DSP building blocks and the six modules.
#pragma once
#include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <vector>
#include <array>
#include <complex>
#include <atomic>

namespace hd
{
constexpr float kPi = 3.14159265358979f;
constexpr float kTwoPi = 6.28318530717959f;

inline float dbToGain (float db) { return std::pow (10.f, db * 0.05f); }
inline float gainToDb (float g)  { return 20.f * std::log10 (std::max (g, 1.0e-9f)); }
inline float clamp01 (float x)   { return juce::jlimit (0.f, 1.f, x); }
inline float onePoleCoef (float hz, double sr) { return 1.f - std::exp (-kTwoPi * hz / (float) sr); }
inline float timeCoef (float ms, double sr) { return 1.f - std::exp (-1.f / (std::max (ms, 0.01f) * 0.001f * (float) sr)); }

//==============================================================================
struct Biquad
{
    float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;

    inline float process (float x) noexcept
    {
        const float y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return y;
    }
    void reset() { z1 = z2 = 0; }

    void set (double B0, double B1, double B2, double A0, double A1, double A2)
    {
        b0 = (float) (B0 / A0); b1 = (float) (B1 / A0); b2 = (float) (B2 / A0);
        a1 = (float) (A1 / A0); a2 = (float) (A2 / A0);
    }
    static double w (double sr, double f) { return 2.0 * 3.141592653589793 * juce::jlimit (5.0, sr * 0.49, f) / sr; }

    void lowpass (double sr, double f, double q)
    {
        const double w0 = w (sr, f), cs = std::cos (w0), al = std::sin (w0) / (2 * q);
        set ((1 - cs) / 2, 1 - cs, (1 - cs) / 2, 1 + al, -2 * cs, 1 - al);
    }
    void highpass (double sr, double f, double q)
    {
        const double w0 = w (sr, f), cs = std::cos (w0), al = std::sin (w0) / (2 * q);
        set ((1 + cs) / 2, -(1 + cs), (1 + cs) / 2, 1 + al, -2 * cs, 1 - al);
    }
    void peak (double sr, double f, double q, double db)
    {
        const double A = std::pow (10.0, db / 40.0), w0 = w (sr, f), cs = std::cos (w0), al = std::sin (w0) / (2 * q);
        set (1 + al * A, -2 * cs, 1 - al * A, 1 + al / A, -2 * cs, 1 - al / A);
    }
    void lowShelf (double sr, double f, double q, double db)
    {
        const double A = std::pow (10.0, db / 40.0), w0 = w (sr, f), cs = std::cos (w0), al = std::sin (w0) / (2 * q), sq = 2 * std::sqrt (A) * al;
        set (A * ((A + 1) - (A - 1) * cs + sq), 2 * A * ((A - 1) - (A + 1) * cs), A * ((A + 1) - (A - 1) * cs - sq),
             (A + 1) + (A - 1) * cs + sq, -2 * ((A - 1) + (A + 1) * cs), (A + 1) + (A - 1) * cs - sq);
    }
    void highShelf (double sr, double f, double q, double db)
    {
        const double A = std::pow (10.0, db / 40.0), w0 = w (sr, f), cs = std::cos (w0), al = std::sin (w0) / (2 * q), sq = 2 * std::sqrt (A) * al;
        set (A * ((A + 1) + (A - 1) * cs + sq), -2 * A * ((A - 1) + (A + 1) * cs), A * ((A + 1) + (A - 1) * cs - sq),
             (A + 1) - (A - 1) * cs + sq, 2 * ((A - 1) - (A + 1) * cs), (A + 1) - (A - 1) * cs - sq);
    }
    void identity() { b0 = 1; b1 = b2 = a1 = a2 = 0; }

    // magnitude response (for the UI curve)
    double magnitude (double f, double sr) const
    {
        const double om = 2.0 * 3.141592653589793 * f / sr;
        const std::complex<double> z1c = std::polar (1.0, -om), z2c = z1c * z1c;
        const auto num = (double) b0 + (double) b1 * z1c + (double) b2 * z2c;
        const auto den = 1.0 + (double) a1 * z1c + (double) a2 * z2c;
        return std::abs (num / den);
    }
};

//==============================================================================
// Smooth random walk: the heart of FLUX. Picks a new random target at `rate`
// and glides there through two cascaded one-poles, so it never sounds like an LFO.
struct SmoothRandom
{
    juce::Random rng;
    float target = 0, y1 = 0, y2 = 0, coef = 0.001f;
    int count = 0, period = 1000;

    void seed (juce::int64 s) { rng.setSeed (s); }
    void setRate (float hz, double sr)
    {
        hz = std::max (hz, 0.01f);
        period = std::max (1, (int) (sr / hz));
        coef = onePoleCoef (hz * 1.2f, sr);
    }
    inline float next (float chaos = 0.f) noexcept
    {
        if (--count <= 0)
        {
            count = (int) (period * (0.6f + 0.8f * rng.nextFloat()));
            target = rng.nextFloat() * 2.f - 1.f;
            if (chaos > 0.f && rng.nextFloat() < chaos * 0.35f) y1 = target; // sudden "event"
        }
        y1 += coef * (target - y1);
        y2 += coef * (y1 - y2);
        return juce::jlimit (-1.f, 1.f, y2 * 1.5f);
    }
};

//==============================================================================
struct DelayLine
{
    std::vector<float> buf;
    int mask = 0, w = 0;

    void prepare (int maxSamples)
    {
        int n = 1;
        while (n < maxSamples + 4) n <<= 1;
        buf.assign ((size_t) n, 0.f);
        mask = n - 1; w = 0;
    }
    void reset() { std::fill (buf.begin(), buf.end(), 0.f); w = 0; }
    inline void push (float x) noexcept { buf[(size_t) w] = x; w = (w + 1) & mask; }
    // d = samples behind the most recently pushed sample
    inline float readInt (int d) const noexcept { return buf[(size_t) ((w - 1 - d) & mask)]; }
    inline float readLinear (float d) const noexcept
    {
        const float pos = (float) (w - 1) - d;
        const int i = (int) std::floor (pos);
        const float f = pos - (float) i;
        const float a = buf[(size_t) (i & mask)], b = buf[(size_t) ((i + 1) & mask)];
        return a + f * (b - a);
    }
    inline float readCubic (float d) const noexcept
    {
        const float pos = (float) (w - 1) - d;
        const int i = (int) std::floor (pos);
        const float f = pos - (float) i;
        const float ym1 = buf[(size_t) ((i - 1) & mask)], y0 = buf[(size_t) (i & mask)];
        const float y1 = buf[(size_t) ((i + 1) & mask)], y2 = buf[(size_t) ((i + 2) & mask)];
        const float c1 = 0.5f * (y1 - ym1), c2 = ym1 - 2.5f * y0 + 2.f * y1 - 0.5f * y2, c3 = 0.5f * (y2 - ym1) + 1.5f * (y0 - y1);
        return ((c3 * f + c2) * f + c1) * f + y0;
    }
};

//==============================================================================
struct EnvFollower
{
    float env = 0, att = 0.1f, rel = 0.001f;
    void prepare (double sr, float attMs, float relMs) { att = timeCoef (attMs, sr); rel = timeCoef (relMs, sr); }
    inline float process (float x) noexcept
    {
        const float a = std::abs (x);
        env += (a > env ? att : rel) * (a - env);
        return env;
    }
};

//==============================================================================
// HEAT - focused distortion. Only the focus band is distorted, and only the
// *difference* the distortion makes is added back, so the rest stays untouched.
struct Heat
{
    double sr = 48000;
    Biquad hp[2], lp[2];
    float toneLp[2] {}, holdV[2] {}, prevDrive = 0, prevMix = 0;
    int holdCount = 0;
    bool useHp = false, useLp = false;
    float lastLo = -1, lastHi = -1;
    std::atomic<float> level { 0 };

    void prepare (double s) { sr = s; reset(); lastLo = lastHi = -1; }
    void reset() { for (int c = 0; c < 2; ++c) { hp[c].reset(); lp[c].reset(); toneLp[c] = holdV[c] = 0; } holdCount = 0; }

    void setFocus (float lo, float hi)
    {
        if (std::abs (lo - lastLo) < 0.5f && std::abs (hi - lastHi) < 2.f) return;
        lastLo = lo; lastHi = hi;
        useHp = lo > 22.f; useLp = hi < 19000.f;
        for (int c = 0; c < 2; ++c) { if (useHp) hp[c].highpass (sr, lo, 0.707); if (useLp) lp[c].lowpass (sr, hi, 0.707); }
    }

    static inline float shape (int mode, float v) noexcept
    {
        switch (mode)
        {
            case 0: { constexpr float b = 0.12f; return std::tanh (v + b) - std::tanh (b); }                // tape sat (asymmetric)
            case 1: return v >= 0.f ? 1.f - std::exp (-v) : -0.8f * (1.f - std::exp (1.25f * v));          // tube
            case 2: return v / (1.f + std::abs (v)) * 1.25f;                                                // transformer
            case 3: return std::sin (v);                                                                     // wavefolder
            case 5: { const float r = 0.65f * v + 0.45f * std::abs (v); return std::tanh (r); }            // rectify (octave grit)
            case 6: { const float c = juce::jlimit (-1.f, 1.f, v * 3.f); return 1.5f * c - 0.5f * c * c * c; } // fuzz
            default: return v;
        }
    }

    // amount: 0..1 (already scaled by AGE etc.), osFactor: oversampling factor of this domain
    void process (float* const* ch, int n, int mode, float amount, float tone, float mix, int osFactor)
    {
        const float driveDb = amount * (mode == 6 ? 26.f : 30.f);
        const float targetDrive = dbToGain (driveDb);
        const float toneA = onePoleCoef (1200.f, sr);
        float lvl = 0;

        if (mode == 4) // crush: bit depth + sample-rate reduction
        {
            const float bits = 16.f - 12.5f * amount;
            const float steps = std::pow (2.f, bits - 1.f);
            const int hold = std::max (1, (int) ((1.f + amount * amount * 22.f) * (float) osFactor));
            for (int i = 0; i < n; ++i)
            {
                const bool take = (holdCount <= 0);
                if (take) holdCount = hold;
                --holdCount;
                for (int c = 0; c < 2; ++c)
                {
                    float x = ch[c][i];
                    float band = x;
                    if (useHp) band = hp[c].process (band);
                    if (useLp) band = lp[c].process (band);
                    if (take) holdV[c] = std::round (band * steps) / steps;
                    float delta = holdV[c] - band;
                    toneLp[c] += toneA * (delta - toneLp[c]);
                    delta += tone * (delta - toneLp[c]);
                    ch[c][i] = x + mix * delta;
                    lvl = std::max (lvl, std::abs (delta));
                }
            }
            level.store (juce::jlimit (0.f, 1.f, lvl * 4.f));
            return;
        }

        const float d0 = prevDrive <= 0 ? targetDrive : prevDrive, m0 = prevMix;
        const float invN = 1.f / (float) std::max (1, n);
        for (int i = 0; i < n; ++i)
        {
            const float t = (float) (i + 1) * invN;
            const float g = d0 + (targetDrive - d0) * t;
            const float mx = m0 + (mix - m0) * t;
            const float comp = std::pow (g, mode == 6 ? -0.95f : -0.88f);
            for (int c = 0; c < 2; ++c)
            {
                const float x = ch[c][i];
                float band = x;
                if (useHp) band = hp[c].process (band);
                if (useLp) band = lp[c].process (band);
                float delta = shape (mode, band * g) * comp - band;
                toneLp[c] += toneA * (delta - toneLp[c]);
                delta += tone * (delta - toneLp[c]);
                ch[c][i] = x + mx * delta;
                lvl = std::max (lvl, std::abs (delta));
            }
        }
        prevDrive = targetDrive; prevMix = mix;
        level.store (juce::jlimit (0.f, 1.f, lvl * 3.f));
    }
};

//==============================================================================
// TAPE - pre-emphasis -> saturation -> de-emphasis -> head bump -> HF loss,
// plus the "magnetic" side: slow volume drift and random dropouts.
struct Tape
{
    double sr = 48000;
    Biquad pre[2], de[2], bump[2], lp[2];
    SmoothRandom volShared, volL, volR, fluxR;
    juce::Random rng;
    float dropEnv = 1.f, dropDepth = 0.f, dropPhase = 0.f, dropLen = 1.f;
    bool dropping = false;
    float lastKey[6] { -1, -1, -1, -1, -1, -1 };
    float prevDrive = 1.f, prevMix = 0.f;

    void prepare (double s)
    {
        sr = s;
        volShared.seed (11); volL.seed (12); volR.seed (13); fluxR.seed (14);
        reset();
    }
    void reset()
    {
        for (int c = 0; c < 2; ++c) { pre[c].reset(); de[c].reset(); bump[c].reset(); lp[c].reset(); }
        for (auto& k : lastKey) k = -1;
        dropping = false; dropEnv = 1.f;
    }

    void process (float* const* ch, int n, float amount, int speed, float bumpAmt, float wear, float drops,
                  float volDrift, float mix, float flux, float fluxRate, float chaos)
    {
        static const float hfTable[3] = { 9500.f, 15000.f, 19500.f };
        static const float bumpF[3] = { 48.f, 72.f, 110.f };
        const float k = std::min (1.f, amount * 2.f);
        fluxR.setRate (0.15f + fluxRate * 2.f, sr);
        const float fl = fluxR.next (chaos); // control-rate flux sample for this chunk
        for (int i = 1; i < n; ++i) fluxR.next (chaos);

        const float emph = 7.f * k;
        float fc = 21000.f * (1.f - k) + hfTable[speed] * k;
        fc *= (1.f - 0.62f * wear * k) * (1.f + 0.14f * flux * fl);
        const float bumpDb = bumpAmt * 4.5f * k;
        const float keys[6] = { emph, fc, bumpDb, (float) speed, 0, 0 };
        if (std::abs (keys[0] - lastKey[0]) > 0.02f || std::abs (keys[1] - lastKey[1]) > 15.f || std::abs (keys[2] - lastKey[2]) > 0.02f || std::abs (keys[3] - lastKey[3]) > 0.5f)
        {
            for (int c = 0; c < 2; ++c)
            {
                pre[c].highShelf (sr, 3200, 0.6, emph);
                de[c].highShelf (sr, 3200, 0.6, -emph);
                bump[c].peak (sr, bumpF[speed], 0.9, bumpDb);
                lp[c].lowpass (sr, std::min (fc, (float) sr * 0.45f), 0.6);
            }
            std::copy (keys, keys + 6, lastKey);
        }

        const float drive = dbToGain (amount * 16.f + flux * fl * 1.5f);
        volShared.setRate (0.35f + fluxRate * 0.8f, sr);
        volL.setRate (0.5f, sr); volR.setRate (0.5f, sr);
        const float driftDepthDb = volDrift * 3.5f;
        const float dropProb = drops * drops * 2.5f / (float) sr; // events per sample
        const float d0 = prevDrive, m0 = prevMix, invN = 1.f / (float) std::max (1, n);

        for (int i = 0; i < n; ++i)
        {
            const float t = (float) (i + 1) * invN;
            const float g = d0 + (drive - d0) * t, mx = m0 + (mix - m0) * t;
            const float comp = 1.f / g;

            // dropouts
            if (! dropping && dropProb > 0.f && rng.nextFloat() < dropProb)
            {
                dropping = true; dropPhase = 0;
                dropLen = (0.025f + 0.12f * rng.nextFloat()) * (float) sr;
                dropDepth = 1.f - dbToGain (-(3.f + 13.f * rng.nextFloat() * drops));
            }
            float dropGain = 1.f;
            if (dropping)
            {
                const float p = dropPhase / dropLen;
                dropGain = 1.f - dropDepth * 0.5f * (1.f - std::cos (kTwoPi * p));
                if (++dropPhase >= dropLen) dropping = false;
            }
            const float sh = volShared.next (chaos);
            const float gl = dbToGain (driftDepthDb * (0.85f * sh + 0.15f * volL.next())) * dropGain;
            const float gr = dbToGain (driftDepthDb * (0.85f * sh + 0.15f * volR.next())) * dropGain;

            for (int c = 0; c < 2; ++c)
            {
                const float x = ch[c][i];
                float y = pre[c].process (x);
                constexpr float b = 0.08f;
                y = (std::tanh (y * g + b) - std::tanh (b)) * comp;
                y = de[c].process (y);
                y = bump[c].process (y);
                y = lp[c].process (y);
                y *= (c == 0 ? gl : gr);
                ch[c][i] = x + mx * (y - x);
            }
        }
        prevDrive = drive; prevMix = mix;
    }
};

//==============================================================================
// DRIFT - modulated delay. Wow/flutter, Juno chorus, Dimension, Ensemble, Vibrato.
// Runs with a constant 2.5 ms centre delay (reported as latency) so "dry" stays aligned.
struct Drift
{
    double sr = 48000;
    DelayLine dl[2];
    float phase[4] {};
    SmoothRandom flux[2], fluxShared;
    Biquad wetLp[2];
    float centre = 120.f;
    std::atomic<float> vis { 0 };

    void prepare (double s)
    {
        sr = s;
        for (auto& d : dl) d.prepare ((int) (0.04 * sr));
        centre = (float) std::round (0.0025 * sr);
        flux[0].seed (21); flux[1].seed (22); fluxShared.seed (23);
        for (int c = 0; c < 2; ++c) wetLp[c].lowpass (sr, 9000, 0.7);
        reset();
    }
    void reset() { for (auto& d : dl) d.reset(); for (auto& p : phase) p = 0; }
    int latency() const { return (int) centre; }

    inline float tri (float p) const { p -= std::floor (p); return 4.f * std::abs (p - 0.5f) - 1.f; }

    void process (float* const* ch, int n, bool on, int mode, float amount, float rate, float spread, float mix,
                  float fluxAmt, float fluxRate, float chaos)
    {
        const float ms = (float) sr * 0.001f;
        const float spreadPh = spread * (mode == 0 ? 0.25f : 0.5f);
        for (auto& f : flux) f.setRate (0.2f + fluxRate * 3.f, sr);
        fluxShared.setRate (0.2f + fluxRate * 3.f, sr);
        float visAcc = 0;

        float f1 = 1, f2 = 1;
        switch (mode)
        {
            case 0: f1 = 0.3f + 1.3f * rate; f2 = 5.f + 9.f * rate; break;
            case 1: f1 = 0.25f + 0.9f * rate; break;
            case 2: f1 = 0.15f + 0.5f * rate; break;
            case 3: f1 = 2.f + 6.f * rate; f2 = 0.2f; break;
            default: f1 = 2.f + 7.f * rate; break;
        }
        const float inc1 = f1 / (float) sr, inc2 = f2 / (float) sr;

        for (int i = 0; i < n; ++i)
        {
            phase[0] += inc1; if (phase[0] >= 1.f) phase[0] -= 1.f;
            phase[1] += inc2; if (phase[1] >= 1.f) phase[1] -= 1.f;
            const float fs = fluxShared.next (chaos);
            const float fr[2] = { 0.85f * fs + 0.15f * flux[0].next (chaos), 0.85f * fs + 0.15f * flux[1].next (chaos) };

            for (int c = 0; c < 2; ++c)
            {
                const float x = ch[c][i];
                dl[c].push (x);
                if (! on) { ch[c][i] = dl[c].readInt ((int) centre); continue; }

                const float dry = dl[c].readInt ((int) centre);
                const float ph = phase[0] + (c == 1 ? spreadPh : 0.f);
                const float fluxMs = fluxAmt * amount * 0.6f * fr[c];
                float wet = 0, y;
                switch (mode)
                {
                    case 0:
                    {
                        const float d = 1.6f * amount * std::sin (kTwoPi * ph) + 0.12f * amount * std::sin (kTwoPi * (phase[1] + c * 0.31f)) + fluxMs;
                        wet = dl[c].readCubic (centre + d * ms);
                        y = dry + mix * (wet - dry);
                        if (c == 0) visAcc += d;
                        break;
                    }
                    case 1:
                    {
                        const float d = 1.0f + 1.8f * amount * tri (ph) + fluxMs;
                        wet = wetLp[c].process (dl[c].readCubic (centre + d * ms));
                        y = dry + mix * 0.5f * (wet - dry);
                        if (c == 0) visAcc += d - 1.f;
                        break;
                    }
                    case 2:
                    {
                        const float d1 = 0.6f * amount * std::sin (kTwoPi * ph) + fluxMs;
                        const float d2 = 0.6f * amount * std::sin (kTwoPi * (ph + 0.5f)) + fluxMs;
                        wet = 0.5f * (dl[c].readCubic (centre + (0.4f + d1) * ms) + dl[c].readCubic (centre + (1.6f + d2) * ms));
                        y = dry + mix * 0.5f * (wet - dry);
                        if (c == 0) visAcc += d1;
                        break;
                    }
                    case 3:
                    {
                        for (int v = 0; v < 3; ++v)
                        {
                            const float pv = ph + (float) v / 3.f;
                            const float d = 0.35f * amount * std::sin (kTwoPi * pv) + 0.8f * amount * std::sin (kTwoPi * (phase[1] + (float) v / 3.f)) + fluxMs;
                            wet += dl[c].readCubic (centre + (1.2f + d) * ms);
                        }
                        wet *= 0.4f;
                        y = dry + mix * 0.5f * (wet - dry * 0.8f);
                        if (c == 0) visAcc += 0.35f * amount * std::sin (kTwoPi * ph);
                        break;
                    }
                    default:
                    {
                        const float d = 2.0f * amount * std::sin (kTwoPi * ph) + fluxMs;
                        wet = dl[c].readCubic (centre + d * ms);
                        y = dry + mix * (wet - dry);
                        if (c == 0) visAcc += d * 0.5f;
                        break;
                    }
                }
                ch[c][i] = y;
            }
        }
        vis.store (visAcc / (float) std::max (1, n));
    }
};

//==============================================================================
// GLUE - stereo-linked compressor with four characters.
struct Glue
{
    double sr = 48000;
    Biquad sc[2];
    float lastSc = -1, env = 0, ms = 0;
    std::atomic<float> grDb { 0 };

    void prepare (double s) { sr = s; reset(); lastSc = -1; }
    void reset() { env = 0; ms = 0; for (auto& b : sc) b.reset(); }

    void process (float* const* ch, int n, bool on, int mode, float amount, float attackMs, float releaseMs, bool autoRel,
                  float scHz, float mix, float fluxOffsetDb)
    {
        if (! on) { env += (0.f - env) * 0.5f; grDb.store (0); return; }
        if (std::abs (scHz - lastSc) > 0.5f) { lastSc = scHz; for (auto& b : sc) b.highpass (sr, scHz, 0.707); }

        float ratio = 2.5f, knee = 6.f, att = attackMs, rel = releaseMs;
        bool rms = false, sat = false;
        switch (mode)
        {
            case 1: ratio = 3.f; att = std::max (attackMs, 8.f) * 1.5f; rms = true; break;
            case 2: ratio = 6.f; att = attackMs * 0.25f; rel = releaseMs * 0.6f; sat = true; knee = 3.f; break;
            case 3: ratio = 12.f; att = 0.5f; knee = 2.f; break;
            default: break;
        }
        const float thr = -amount * 36.f + fluxOffsetDb;
        const float makeup = -thr * (1.f - 1.f / ratio) * 0.3f;
        const float aAtt = timeCoef (att, sr);
        const float rmsA = timeCoef (8.f, sr);
        float minGr = 0;

        for (int i = 0; i < n; ++i)
        {
            const float l = sc[0].process (ch[0][i]), r = sc[1].process (ch[1][i]);
            float det;
            if (rms) { ms += rmsA * (0.5f * (l * l + r * r) - ms); det = std::sqrt (2.f * ms); }
            else det = std::max (std::abs (l), std::abs (r));
            const float lv = gainToDb (det);
            const float over = lv - thr;
            float gr;
            if (2.f * over < -knee) gr = 0;
            else if (2.f * std::abs (over) <= knee) { const float o = over + knee * 0.5f; gr = (1.f / ratio - 1.f) * o * o / (2.f * knee); }
            else gr = (1.f / ratio - 1.f) * over;

            float relNow = rel;
            if (autoRel && mode != 3) relNow = rel * (0.35f + 1.6f * clamp01 (-env / 12.f));
            if (mode == 1) relNow *= (env < -6.f ? 2.2f : 1.f);
            const float a = gr < env ? aAtt : timeCoef (relNow, sr);
            env += a * (gr - env);
            minGr = std::min (minGr, env);

            const float g = dbToGain (env + makeup);
            for (int c = 0; c < 2; ++c)
            {
                const float x = ch[c][i];
                float w = x * g;
                if (sat) w = std::tanh (w * 1.15f) / 1.15f;
                ch[c][i] = x + mix * (w - x);
            }
        }
        grDb.store (minGr);
    }
};

//==============================================================================
// GRAIN - granular delay. Writes into a 4 s buffer, sprays windowed grains back out.
struct Grain
{
    struct G { bool on = false; double pos = 0; float inc = 1, len = 1, age = 0, gl = 1, gr = 1; };
    static constexpr int kMaxGrains = 48;
    double sr = 48000;
    std::vector<float> buf[2];
    int mask = 0, w = 0;
    G grains[kMaxGrains];
    float hann[1025] {};
    float spawnCount = 0;
    Biquad fHp[2], fLp[2];
    float lastLo = -1, lastHi = -1;
    float fb[2] {}, wetGain = 0, duckGain = 1;
    EnvFollower duckEnv;
    juce::Random rng { 777 };
    SmoothRandom fluxR;
    std::atomic<int> active { 0 };

    void prepare (double s)
    {
        sr = s;
        int sz = 1; while (sz < (int) (4.2 * sr)) sz <<= 1;
        for (auto& b : buf) b.assign ((size_t) sz, 0.f);
        mask = sz - 1; w = 0;
        for (int i = 0; i <= 1024; ++i) hann[i] = 0.5f - 0.5f * std::cos (kTwoPi * (float) i / 1024.f);
        duckEnv.prepare (sr, 5.f, 250.f);
        fluxR.seed (31);
        lastLo = lastHi = -1;
        reset();
    }
    void reset()
    {
        for (auto& b : buf) std::fill (b.begin(), b.end(), 0.f);
        for (auto& g : grains) g.on = false;
        fb[0] = fb[1] = 0; spawnCount = 0; wetGain = 0;
        for (int c = 0; c < 2; ++c) { fHp[c].reset(); fLp[c].reset(); }
    }

    inline float readBuf (int c, double p) const noexcept
    {
        const int i = (int) std::floor (p);
        const float f = (float) (p - i);
        const float a = buf[c][(size_t) (i & mask)], b = buf[c][(size_t) ((i + 1) & mask)];
        return a + f * (b - a);
    }

    // in: dry input (read only); out: wet only (added by caller)
    void process (const float* const* in, float* const* out, int n, bool on, float amount, float delayMs, float sizeMs,
                  float density, float spray, float pitch, float reverse, float feedback, float duck, bool freeze,
                  float lo, float hi, float fluxAmt, float fluxRate, float chaos)
    {
        if (std::abs (lo - lastLo) > 0.5f || std::abs (hi - lastHi) > 2.f)
        {
            lastLo = lo; lastHi = hi;
            for (int c = 0; c < 2; ++c) { fHp[c].highpass (sr, lo, 0.6); fLp[c].lowpass (sr, hi, 0.6); }
        }
        const float targetWet = on ? amount : 0.f;
        if (! on && wetGain < 1.0e-4f)
        {
            for (int c = 0; c < 2; ++c) juce::FloatVectorOperations::clear (out[c], n);
            // keep the buffer rolling so turning it on is instant
            for (int i = 0; i < n; ++i) { buf[0][(size_t) w] = in[0][i]; buf[1][(size_t) w] = in[1][i]; w = (w + 1) & mask; }
            active.store (0);
            return;
        }

        fluxR.setRate (0.3f + fluxRate * 4.f, sr);
        const float densHz = 3.f + density * 57.f;
        const float lenS = sizeMs * 0.001f * (float) sr;
        const float delayS = std::max (delayMs * 0.001f * (float) sr, 16.f);
        const float norm = 1.f / std::sqrt (std::max (1.f, densHz * sizeMs * 0.001f));
        const float wetA = timeCoef (20.f, sr);
        const float maxBack = (float) mask - 8.f;

        for (int i = 0; i < n; ++i)
        {
            const float fl = fluxR.next (chaos) * fluxAmt;

            // spawn
            if (--spawnCount <= 0)
            {
                spawnCount = (float) sr / densHz * (1.f + spray * 0.6f * (rng.nextFloat() * 2.f - 1.f));
                for (auto& g : grains)
                {
                    if (g.on) continue;
                    const float len = std::max (32.f, lenS * (1.f + 0.4f * fl));
                    const float semis = pitch + fl * 0.35f;
                    const float r = std::pow (2.f, semis / 12.f);
                    const bool rev = rng.nextFloat() < reverse;
                    float back = delayS * (1.f + spray * rng.nextFloat()) + spray * rng.nextFloat() * len;
                    if (! rev) back = std::max (back, (r - 1.f) * len + len * 0.0f + 8.f);
                    else back = std::max (back, 8.f);
                    back = std::min (back, maxBack - r * len);
                    g.on = true; g.age = 0; g.len = len;
                    g.inc = rev ? -r : r;
                    g.pos = (double) w - back;
                    const float pan = (rng.nextFloat() * 2.f - 1.f) * (0.3f + 0.6f * spray);
                    g.gl = std::cos ((pan + 1.f) * kPi * 0.25f) * 1.4142f;
                    g.gr = std::sin ((pan + 1.f) * kPi * 0.25f) * 1.4142f;
                    break;
                }
            }

            float sl = 0, srr = 0;
            int nAct = 0;
            for (auto& g : grains)
            {
                if (! g.on) continue;
                ++nAct;
                const float ph = g.age / g.len;
                const float win = hann[(int) (ph * 1024.f)];
                sl += readBuf (0, g.pos) * win * g.gl;
                srr += readBuf (1, g.pos) * win * g.gr;
                g.pos += g.inc;
                if (++g.age >= g.len) g.on = false;
            }
            if (i == 0) active.store (nAct);

            float wl = fLp[0].process (fHp[0].process (sl * norm));
            float wr = fLp[1].process (fHp[1].process (srr * norm));
            fb[0] = wl; fb[1] = wr;

            if (! freeze)
            {
                buf[0][(size_t) w] = in[0][i] + feedback * std::tanh (fb[0]);
                buf[1][(size_t) w] = in[1][i] + feedback * std::tanh (fb[1]);
                w = (w + 1) & mask;
            }

            const float e = duckEnv.process (0.5f * (in[0][i] + in[1][i]));
            const float dTarget = 1.f - duck * clamp01 (e * 5.f);
            duckGain += 0.002f * (dTarget - duckGain);
            wetGain += wetA * (targetWet - wetGain);
            const float gOut = wetGain * duckGain * 1.2f;
            out[0][i] = wl * gOut;
            out[1][i] = wr * gOut;
        }
    }
};

//==============================================================================
// SPACE - 8-line feedback delay network with input diffusion, modulation and damping.
struct Space
{
    static constexpr int N = 8;
    struct Allpass
    {
        std::vector<float> b; int idx = 0, len = 1; float g = 0.6f;
        void prepare (int maxLen) { b.assign ((size_t) std::max (2, maxLen), 0.f); idx = 0; }
        void setLen (int l) { len = juce::jlimit (1, (int) b.size(), l); if (idx >= len) idx = 0; }
        void reset() { std::fill (b.begin(), b.end(), 0.f); }
        inline float process (float x) noexcept
        {
            const float d = b[(size_t) idx];
            const float y = d - g * x;
            b[(size_t) idx] = x + g * y;
            if (++idx >= len) idx = 0;
            return y;
        }
    };
    struct AP1 { float a = 0.6f, z = 0; inline float process (float x) noexcept { const float y = a * x + z; z = x - a * y; return y; } };

    double sr = 48000;
    DelayLine lines[N], pre[2];
    float curLen[N] {}, damp[N] {}, phase[N] {}, phInc[N] {};
    Allpass diff[2][4];
    AP1 spring[2][24];
    Biquad springHp[2], springLp[2], inHp[2], inLp[2];
    float lastLo = -1, lastHi = -1, wetGain = 0, duckGain = 1;
    EnvFollower duckEnv;
    SmoothRandom fluxR;
    std::atomic<float> level { 0 };

    static constexpr float baseMs[N] = { 31.7f, 37.1f, 41.9f, 47.3f, 53.9f, 59.3f, 67.7f, 73.1f };
    static constexpr float diffMs[4] = { 4.7f, 3.6f, 12.7f, 9.3f };

    void prepare (double s)
    {
        sr = s;
        for (int i = 0; i < N; ++i)
        {
            lines[i].prepare ((int) (0.2 * sr));
            curLen[i] = baseMs[i] * 0.001f * (float) sr * 0.5f;
            phInc[i] = (0.11f + 0.13f * (float) i) / (float) sr;
            phase[i] = (float) i / N;
        }
        for (auto& p : pre) p.prepare ((int) (0.2 * sr));
        for (int c = 0; c < 2; ++c)
        {
            for (int k = 0; k < 4; ++k) diff[c][k].prepare ((int) (0.03 * sr));
            springHp[c].highpass (sr, 280, 0.7); springLp[c].lowpass (sr, 4600, 0.7);
        }
        duckEnv.prepare (sr, 5.f, 300.f);
        fluxR.seed (41);
        lastLo = lastHi = -1;
        reset();
    }
    void reset()
    {
        for (auto& l : lines) l.reset();
        for (auto& p : pre) p.reset();
        for (auto& d : damp) d = 0;
        for (int c = 0; c < 2; ++c) { for (auto& a : diff[c]) a.reset(); for (auto& a : spring[c]) a.z = 0; inHp[c].reset(); inLp[c].reset(); }
        wetGain = 0;
    }

    void process (const float* const* in, float* const* out, int n, bool on, int mode, float amount, float preMs,
                  float size, float decayS, float dampAmt, float mod, float width, float duck, float lo, float hi,
                  float fluxAmt, float fluxRate, float chaos)
    {
        if (std::abs (lo - lastLo) > 0.5f || std::abs (hi - lastHi) > 2.f)
        {
            lastLo = lo; lastHi = hi;
            for (int c = 0; c < 2; ++c) { inHp[c].highpass (sr, lo, 0.6); inLp[c].lowpass (sr, hi, 0.6); }
        }
        const float targetWet = on ? amount : 0.f;
        if (! on && wetGain < 1.0e-4f)
        {
            for (int c = 0; c < 2; ++c) juce::FloatVectorOperations::clear (out[c], n);
            level.store (0);
            return;
        }

        //               Room   Plate  Spring Hall  Haze
        static const float sMin[5]   = { 0.18f, 0.35f, 0.30f, 0.80f, 1.20f };
        static const float sMax[5]   = { 0.55f, 0.75f, 0.50f, 1.60f, 2.00f };
        static const float diffG[5]  = { 0.60f, 0.75f, 0.50f, 0.70f, 0.75f };
        static const float modSc[5]  = { 0.5f,  0.6f,  0.3f,  1.0f,  4.0f };
        static const float bright[5] = { 1.0f,  1.3f,  0.8f,  0.9f,  0.55f };

        const float scale = sMin[mode] + (sMax[mode] - sMin[mode]) * size;
        for (int c = 0; c < 2; ++c)
            for (int k = 0; k < 4; ++k)
            {
                diff[c][k].g = diffG[mode];
                diff[c][k].setLen ((int) (diffMs[k] * (0.5f + 0.5f * scale) * (c == 1 ? 1.07f : 1.f) * 0.001f * (float) sr));
            }
        const float dampHz = juce::jlimit (800.f, 20000.f, (1500.f + 16500.f * std::pow (1.f - dampAmt, 1.5f)) * bright[mode]);
        const float dampA = onePoleCoef (dampHz, sr);
        const float modDepth = mod * modSc[mode] * 0.0009f * (float) sr;
        fluxR.setRate (0.2f + fluxRate * 2.f, sr);
        const float lenA = 0.0005f;
        const float wetA = timeCoef (25.f, sr);
        float targetLen[N], g[N];
        for (int k = 0; k < N; ++k)
        {
            targetLen[k] = baseMs[k] * scale * 0.001f * (float) sr;
            g[k] = std::pow (10.f, -3.f * (curLen[k] / (float) sr) / std::max (0.1f, decayS));
        }
        float lvl = 0;

        for (int i = 0; i < n; ++i)
        {
            const float fl = fluxR.next (chaos) * fluxAmt;
            const float preS = std::max (1.f, (preMs + 6.f * std::abs (fl)) * 0.001f * (float) sr);
            float inj[2];
            for (int c = 0; c < 2; ++c)
            {
                float x = inLp[c].process (inHp[c].process (in[c][i]));
                pre[c].push (x);
                x = pre[c].readLinear (preS);
                for (auto& a : diff[c]) x = a.process (x);
                if (mode == 2)
                {
                    for (auto& a : spring[c]) x = a.process (x);
                    x = springLp[c].process (springHp[c].process (x)) * 1.4f;
                }
                inj[c] = x * 0.4f;
            }

            float v[N], sum = 0;
            for (int k = 0; k < N; ++k)
            {
                phase[k] += phInc[k] * (1.f + 0.5f * fl);
                if (phase[k] >= 1.f) phase[k] -= 1.f;
                curLen[k] += lenA * (targetLen[k] - curLen[k]);
                const float d = curLen[k] + modDepth * std::sin (kTwoPi * phase[k]);
                float y = lines[k].readLinear (std::max (2.f, d));
                damp[k] += dampA * (y - damp[k]);
                v[k] = damp[k] * g[k];
                sum += v[k];
            }
            const float hh = sum * (2.f / N);
            for (int k = 0; k < N; ++k)
                lines[k].push (v[k] - hh + (k < 4 ? inj[0] : inj[1]));

            float wl = 0.5f * (v[0] - v[2] + v[4] - v[6]);
            float wr = 0.5f * (v[1] - v[3] + v[5] - v[7]);
            const float m = 0.5f * (wl + wr), s = 0.5f * (wl - wr) * width;
            wl = m + s; wr = m - s;

            const float e = duckEnv.process (0.5f * (in[0][i] + in[1][i]));
            duckGain += 0.0015f * ((1.f - duck * clamp01 (e * 5.f)) - duckGain);
            wetGain += wetA * (targetWet - wetGain);
            const float gOut = wetGain * duckGain * 1.2f;
            out[0][i] = wl * gOut;
            out[1][i] = wr * gOut;
            lvl = std::max (lvl, std::abs (wl));
        }
        level.store (juce::jlimit (0.f, 1.f, lvl * 3.f));
    }
};

//==============================================================================
// DUST - procedurally generated noise layer, added outside the effect chain.
struct Dust
{
    double sr = 48000;
    juce::Random rng { 99 };
    Biquad hissHp[2], hissLp[2], roomLp[2], rumbleLp[2], clickHp[2];
    float brown[2] {}, click[2] {}, clickDecay = 0.9f, humPh = 0, humAmp = 1;
    SmoothRandom humWobble;
    EnvFollower env;
    float follow = 0;

    void prepare (double s)
    {
        sr = s;
        for (int c = 0; c < 2; ++c)
        {
            hissHp[c].highpass (sr, 900, 0.5); hissLp[c].lowpass (sr, 13000, 0.6);
            roomLp[c].lowpass (sr, 600, 0.6); rumbleLp[c].lowpass (sr, 70, 0.7);
            clickHp[c].highpass (sr, 1200, 0.7);
        }
        clickDecay = std::exp (-1.f / (0.00025f * (float) sr));
        humWobble.seed (51); humWobble.setRate (0.4f, sr);
        env.prepare (sr, 10.f, 400.f);
    }

    void process (const float* const* follow_in, float* const* out, int n, int type, float level)
    {
        if (level <= 0.0001f) return;
        const float base = level * level * 0.035f;
        for (int i = 0; i < n; ++i)
        {
            const float e = env.process (0.5f * (follow_in[0][i] + follow_in[1][i]));
            const float target = 0.3f + 0.7f * clamp01 (e * 6.f);
            follow += 0.0005f * (target - follow);
            const float gn = base * follow;
            float hum = 0;
            if (type == 3)
            {
                humPh += 50.f / (float) sr; if (humPh >= 1.f) humPh -= 1.f;
                const float wob = 1.f + 0.25f * humWobble.next();
                hum = (std::sin (kTwoPi * humPh) + 0.5f * std::sin (2 * kTwoPi * humPh) + 0.35f * std::sin (3 * kTwoPi * humPh)
                       + 0.15f * std::sin (5 * kTwoPi * humPh)) * 0.6f * wob;
            }
            for (int c = 0; c < 2; ++c)
            {
                const float wn = rng.nextFloat() * 2.f - 1.f;
                float s = 0;
                switch (type)
                {
                    case 0: s = hissLp[c].process (hissHp[c].process (wn)) * 1.2f; break;
                    case 1:
                    {
                        if (rng.nextFloat() < 14.f / (float) sr) click[c] = (rng.nextFloat() * 2.f - 1.f) * (0.6f + 2.5f * rng.nextFloat() * rng.nextFloat());
                        click[c] *= clickDecay;
                        brown[c] = 0.995f * brown[c] + 0.02f * wn;
                        s = clickHp[c].process (click[c] + wn * 0.02f) * 2.f + rumbleLp[c].process (brown[c]) * 2.f;
                        break;
                    }
                    case 2:
                        brown[c] = 0.997f * brown[c] + 0.03f * wn;
                        s = roomLp[c].process (brown[c]) * 3.f + wn * 0.03f;
                        break;
                    default: s = hum + wn * 0.02f; break;
                }
                out[c][i] += s * gn;
            }
        }
    }
};

//==============================================================================
// Loudness match: K-weighted mean-square of the plugin input vs. the processed
// signal, with a slow, gated gain trim so A/B and bypass compare tone, not volume.
struct LoudnessMatch
{
    double sr = 48000;
    Biquad kin1[2], kin2[2], kout1[2], kout2[2];
    double msIn = 0, msOut = 0;
    float gain = 1, avgA = 0, gainA = 0;
    std::atomic<float> deltaDb { 0 };

    void prepare (double s)
    {
        sr = s;
        for (int c = 0; c < 2; ++c)
        {
            kin1[c].highShelf (sr, 1681, 0.707, 4.0); kin2[c].highpass (sr, 38, 0.5);
            kout1[c] = kin1[c]; kout2[c] = kin2[c];
        }
        avgA = timeCoef (1200.f, sr);
        gainA = timeCoef (300.f, sr);
        msIn = msOut = 0; gain = 1;
    }

    void measureIn (const float* const* x, int n)
    {
        for (int i = 0; i < n; ++i)
        {
            double acc = 0;
            for (int c = 0; c < 2; ++c) { const float k = kin2[c].process (kin1[c].process (x[c][i])); acc += (double) k * k; }
            msIn += avgA * (acc * 0.5 - msIn);
        }
    }

    void apply (float* const* x, int n, bool on)
    {
        for (int i = 0; i < n; ++i)
        {
            double acc = 0;
            for (int c = 0; c < 2; ++c) { const float k = kout2[c].process (kout1[c].process (x[c][i])); acc += (double) k * k; }
            msOut += avgA * (acc * 0.5 - msOut);

            float target = gain;
            if (! on) target = 1.f;
            else if (msIn > 1.0e-8 && msOut > 1.0e-10) target = (float) juce::jlimit (0.06, 8.0, std::sqrt (msIn / msOut));
            gain += gainA * (target - gain);
            x[0][i] *= gain; x[1][i] *= gain;
        }
        if (msIn > 1.0e-8 && msOut > 1.0e-10) deltaDb.store ((float) (10.0 * std::log10 (msOut / msIn)));
    }
};
} // namespace hd
