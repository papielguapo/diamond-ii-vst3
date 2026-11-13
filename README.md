# DIAMOND II — Papielguapo Edition

![DIAMOND II - Papielguapo Edition VST3](assets/screenshot.png)

A C++20 / JUCE 8 parallel tape-colour processor, with the supplied silver-metal / purple-knob skin, five saturation characters and three spectral profiles. Builds VST3 and Standalone for mono/stereo float processing. Resize by dragging the corner or right-click the panel to select 25–200%; dimensions are saved per instance with the plugin state. The reference artwork is embedded in the binary, and the knob faces rotate as the real parameters change. Numeric readouts remain editable. The base design is 1252×380; the original raster resolution determines sharpness when enlarged. Input trim is ±10 dB and Output trim is ±6 dB, matching the printed scales. The minimum size is 313×95 (25%); 33%, 50%, 60%, 75%, 100%, 125%, 150% and 200% presets are also available. Negative scale percentages have no meaningful window geometry. Type uses five bounded detents aimed at the label rows, with less than 90 degrees of total pointer travel; the labels are clickable and track host automation.

**Independent measured approximation, not a verified 1:1 clone of Phoenix II.** The current DSP was fitted to mono PCM24 AudioSuite captures of the installed Phoenix II in Pro Tools 12.5.0 at 48 kHz, Input 0 dB, Output -6 dB and Process 100. All five Types and all three Brightness settings have separate measured models. Dark Essence/Gold also has measured Input -6/+3/+6 dB captures and a new Process 87.5% capture. No original source or binary is distributed; there is no affiliation with Crane Song or Dave Hill. See [calibration results and limits](Calibration/README.md).

## Measured signal architecture

For input/output gains `gi`, `go` and Process `p` in [0,1]:

`dry = gi*x; u = Hpre(dry); wet = Hpost(c(u)); residual = wet-dry; y = go*(dry + p*residual + correction)`.

For Luster and Dark Essence in Gold only, `correction = 4*p*(1-p)*(a*dry+b*residual)`, using the measured coefficients in `processCorrection`. It vanishes at Process 0 and 100. Other combinations use zero correction. The coefficients were fitted to original 50% captures using the original 100% endpoints, then checked at 25% and 75%. This is a small empirical correction, not the recovered original control law.

Fourteen Type/Brightness combinations use a fitted pre-shelf, parametric curve and post-shelf:

$$f(u)=\frac{g u}{(1+|k u|^q)^{r/q}}.$$

The tail exponent `r` defaults to 1. Iridescent/Sapphire has a separately fitted exponent and a causal amplitude-dependent phase correction: `v[n]=f(u[n]); c[n]=v[n]+m*u[n]^2/(1+u[n]^2)*(v[n]-v[n-1])`; `Hpost` receives `c`. Its previous shaped sample starts at zero and is independent per channel. Other parametric combinations have `m=0`. Dark Essence/Gold uses the residual model below. This empirical one-sample state is not a recovered physical tape model.

Dark Essence/Gold uses an odd residual spline `D(u)` with knots at `[0,.03,.07,.12,.2,.3,.4,.5,.6,.7,.8,.9,1]`. Its residual is zero outside |u|<1; `D(0)=D(1)=0`, `D''(0)=0`, and `D'(1)=0`. The spline is an empirical model, not a claim about the original code. Its phase-corrected residual passes through its post shelf and is added to the unfiltered dry signal. See [Calibration/residual-dark-gold.json](Calibration/residual-dark-gold.json) and the generated [Source/ResidualCalibration.h](Source/ResidualCalibration.h).

To preserve measured intermediate behavior, both Dark Essence/Gold cores keep running. Their residuals blend with `w=clamp(4*(p-.75),0,1)`: `residual=legacyResidual+w*(newResidual-legacyResidual)`. This keeps the measured 25/50/75% responses and uses the new branch fully at 100%. The 87.5% blend was checked against a new original capture. The existing Process correction still applies. Both cores have separate per-channel IIR states; changing Type/Brightness copies them during the 5 ms crossfade. No audio-thread allocation is introduced.

The exact fitted coefficients are in [Source/CalibrationData.h](Source/CalibrationData.h) and [Calibration/all-colors-48k-fit.json](Calibration/all-colors-48k-fit.json). This odd-symmetric curve replaces the prototype's invented even-harmonic additions. Cubic Hermite tables with 8192 intervals over [0,32] evaluate it during audio processing; the table input magnitude clamps at 32. Tables are built once at module load. No per-sample transcendental calls, allocations, oversampling, lookahead or added latency are used.

The high-shelf implementation uses `K=tan(pi*fc/fs)`, `V=10^(dB/20)`, damping `d` and `N=1/(1+d*K+K*K)`. Its coefficients are `(b0,b1,b2)=N*(V+sqrt(V)*d*K+K*K, 2*(K*K-V), V-sqrt(V)*d*K+K*K)` and `(a1,a2)=N*(2*(K*K-1),1-d*K+K*K)`. Damping is fitted in the refined models; the exact values are in their parameter tables. This adds two measured shape parameters without extra filter stages or latency.

At exactly 0 dB Input, 0% Process and 0 dB Output, the processor returns the original float directly, preserving its bit pattern including signed zero. The wet filters keep running while dry. Input/output gain and Process transitions are smoothed over 15 ms; exact unity requests take effect immediately. Type and Brightness changes crossfade processing states over 5 ms. Rapid repeated changes are not guaranteed artifact-free. Output is not limited: allow headroom.

**Calibration boundaries:** comparison to the original generally covers 48 kHz mono, zero input trim and full Process; Dark Essence/Gold additionally covers Input -6/+3/+6 dB at full Process and 87.5% Process at Input 0 dB. Other sample rates recalculate shelves but have not been matched to original captures. Process uses parallel interpolation with the limited measured correction described above. All five Gold modes were captured at 50%; Luster and Dark Essence also at 25% and 75%. Further intermediate settings remain unmeasured, and the control law is not exact. Stereo interaction, music material, further input-trim sweeps and realtime versus AudioSuite equivalence remain unmeasured. Existing sessions retain parameter IDs and the skin, but their processed sound changes with this calibration.

### Requested prototype equations: historical appendix

The initial unmeasured prototype used the following Padé approximants:

$$P_{3/2}(u)=\frac{u(27+u^2)}{27+9u^2},\qquad P_{5/4}(u)=\frac{u(945+105u^2+u^4)}{945+420u^2+15u^4}.$$

The helper functions remain unit-tested, with clamps at ±3 and ±4. They are no longer in the production audio path. Its Chebyshev basis was:

$$T_2(t)=2t^2-1,\quad T_3(t)=4t^3-3t,\quad T_5(t)=16t^5-20t^3+5t.$$

Its one-state memory was `m[n]=(1-alpha)*x[n]+alpha*m[n-1]`, `alpha=exp(-1/(30e-6*fs))`, `u[n]=x[n]+0.035*(m[n-1]-x[n])`. These polynomial additions and that magnetic-memory approximation were removed after measuring the original. Current memory consists of per-channel IIR states and one-sample shaping/residual states in the refined models. None of these historical formulas is established as Phoenix II's proprietary algorithm.

## Build

Requires Git, CMake ≥3.24, a C++20 compiler, internet access on first configure, and JUCE platform dependencies. JUCE 8.0.12 is pinned to commit `29396c22c93392d6738e021b83196283d6e4d850` and fetched by CMake. A local checkout can be selected with `-DDIAMOND_LOCAL_JUCE=ON -DJUCE_SOURCE_DIR=/absolute/path/to/JUCE`.

Windows, with Visual Studio C++ tools and Windows SDK installed:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
& ./build/GeneratePluginScreenshot_artefacts/Release/GeneratePluginScreenshot.exe "$PWD/assets/screenshot.png"
```

macOS, with Xcode command-line tools:

```sh
cmake -S . -B build -G Xcode -DCMAKE_OSX_ARCHITECTURES='arm64;x86_64'
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
./build/GeneratePluginScreenshot_artefacts/Release/GeneratePluginScreenshot "$PWD/assets/screenshot.png"
```

Linux (Debian/Ubuntu example), with an active graphical session for GUI tests:

```sh
sudo apt install build-essential cmake git libasound2-dev libjack-jackd2-dev libx11-dev libxext-dev libxinerama-dev libxrandr-dev libxcursor-dev libfreetype6-dev libfontconfig1-dev libgl1-mesa-dev
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
./build/GeneratePluginScreenshot_artefacts/Release/GeneratePluginScreenshot "$PWD/assets/screenshot.png"
```

Find plugins under `build/DiamondII_artefacts/Release/VST3/` and the executable under `Standalone/`. Copy the complete `.vst3` bundle to your DAW's VST3 folder and rescan. Windows user location: `%LOCALAPPDATA%/Programs/Common/VST3`; system location: `C:/Program Files/Common Files/VST3`. Standalone audio device settings are available through JUCE's application controls. Binaries are unsigned. macOS and Linux build instructions are provided but have not been executed in this Windows environment.

## Verification

`DspVerificationTest` tests the actual AudioProcessor, not a separate mock. The finite randomized null test covers all 5 types × 3 brightness settings × 5 sample rates (44.1, 48, 88.2, 96 and 192 kHz), including positive and negative zero, using bitwise IEEE-754 comparisons. Thirty captured-reference checks cover two synthetic probes across all 15 combinations at full Process; 18 more cover intermediate Gold Process values and three more cover strong-level/ultrasonic Iridescent/Sapphire probes. Fifteen additional captured-reference checks cover Dark Essence/Gold input trims and Process 87.5%. Per-case regression limits equal the measured baseline relative RMS times 1.10 plus 0.001; passing them guards this approximation against regression, not against all audible differences. Additional tests stress maximum trims (+10 dB input / +6 dB output) and full processing, channel isolation, reset silence, active nonlinearity, rational unit slopes, UI resize bounds, state round-trip, and exact unity after an active wet state. Release checks throw on failure; they do not depend on disabled `assert()` macros. Passing a finite test suite does not prove every possible input or host is supported.

`GeneratePluginScreenshot` initializes `ScopedJuceInitialiser_GUI`, instantiates the actual processor/editor, sets +2.50 dB / 48% / −1.20 dB / Gold / Iridescent, and captures a 980×300 logical component at 2×. The reference skin fits proportionally with narrow black letterboxing because its aspect ratio differs slightly from 1960×600. The tool fails unless the PNG is 1960×600 and exceeds 50 KiB. Additional PNGs record each resize preset. It is an offscreen render of the real component, not a photograph of a DAW window. Host resize negotiation and interaction in third-party DAWs have not been tested.

See [assets/verification.txt](assets/verification.txt) for actual local build/test results. The exact dry null is `0.0f` difference / `−inf dBFS`; it does not establish bit-identical wet processing with another product.

## License

Original project sources are MIT licensed (see LICENSE). The supplied skin in `assets/skin-reference.png` is user-provided artwork and is excluded from the source-code license grant. JUCE is a separate dependency with its own AGPLv3/commercial licensing requirements: review [JUCE's licensing](https://juce.com/legal/juce-8-licence/) before distributing proprietary binaries. No third-party plugin DSP code is included.
