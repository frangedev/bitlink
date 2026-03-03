# BitLink

**BitLink** is an open-source, high-energy sampler plugin designed to bring the gritty, 8-bit soul of the Commodore Amiga's PAULA chip and early Akai hardware into the modern DAW.

Inspired by the workflow of 90s jungle, hardcore, and breakbeat production, BitLink focuses on instant resampling, "dirty" time-stretching, and seamless loop slicing.

## Key Features

* **The Crunch Engine:** Automatic real-time downsampling. Choose between 8-bit and 12-bit modes with a fixed 22kHz resampling rate to recreate authentic aliasing and quantization noise.
* **Vampire Slicer:** An intelligent transient detection system that maps slices across your MIDI controller instantly. Includes a "Thru" playback mode for classic jungle "re-trigger" effects.
* **Cyclic Time-Stretch:** A granular stretching algorithm that locks grain size to the original sample pitch, recreating the metallic, artifact-heavy stretch found in vintage S1000 samplers.
* **Aura UI:** A customizable pixel-art interface with interchangeable color palettes, inspired by 1990s tracker software (OctaMED/Protracker).
* **Legacy Export:** The only modern sampler that allows you to export your processed "crushed" sounds directly to **IFF/8SVX** formats for use on actual Amiga hardware.

## Technical Stack

* **Framework:** Built with **JUCE (C++)** for high-performance audio processing.
* **Formats:** VST3, AU, and CLAP support.
* **DSP:** Custom linear interpolation algorithms for authentic vintage resampling artifacts.

## Installation

### For Users

1. Download the latest installer from the [Releases](https://www.google.com/search?q=https://github.com/frangedev/bitlink/releases) page.
2. Move the `.vst3` or `.component` file to your system's plugin folder.
3. Rescan your DAW.

### For Developers

To build BitLink from source:

```bash
git clone https://github.com/frangedev/bitlink.git
cd bitlink
# Initialize submodules (JUCE)
git submodule update --init --recursive
# Use CMake to build
cmake -B build
cmake --build build
```

**Scripts** (from repo root):

- `scripts/build.sh` — Configure and build Release.
- `scripts/run.sh` — Build if needed, then launch the standalone app (macOS: `open BitLink.app`).
- `scripts/clean.sh` — Remove the `build/` directory.

## Usage

1. **Load:** Drag and drop any WAV or AIFF file into the waveform display, or use **Load sample**. BitLink applies the Crunch settings on load.
2. **Preview:** Click and hold on the waveform to audition from that position; release to stop.
3. **Slice:** Click **Auto-Slice** to map transients to keys (tune with the sensitivity slider). Double-click the waveform to add a slice; right-click to remove one.
4. **Stretch:** Turn on **Stretch** and use the dial to time-stretch without changing pitch (hoover/rave style).
5. **Tools:** **Clear**, **Norm**, **Rev**, **Base note**, **Gain** as above.
6. **Export:** **8SVX** / **WAV** for Amiga or 24-bit WAV.
7. **Presets:** **Save preset** / **Load preset** store and recall the full state (all settings + sample + slices) as `.bitlinkpreset` files.
8. **Sound:** **Save sound** / **Load sound** save or load only the current (cropped/sliced) sample as WAV plus a `.bitlink-slices` file for slice positions. Use for moving sounds between projects without full preset data.
9. **Info / Settings:** Click **Info / Settings** (or press **I**) to open a panel with sample info and a list of **keyboard shortcuts**.

### Keyboard shortcuts

| Key | Action |
|-----|--------|
| L | Load sample |
| C | Clear |
| A | Auto-Slice |
| N | Normalize |
| R | Reverse |
| 8 | Export 8SVX |
| W | Export WAV |
| S | Save sound |
| Shift+S | Save preset |
| O | Load preset |
| I | Toggle Info / Settings panel |

## Contributing

We welcome contributions! Whether it’s improving the DSP stretching algorithm, adding new UI skins, or fixing bugs. Please read `CONTRIBUTING.md` before submitting a Pull Request.

## License

BitLink is released under the **GPL-3.0 License**. Keep audio software free and open.

---

*Created with passion for the 14.4k modem generation.*
