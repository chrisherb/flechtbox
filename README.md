# flechtbox

Terminal-based groovebox.

Features:

- 9 tracks with probability-based step sequencers
- instance of Mutable Instruments Plaits per track with 24 synthesizer engines
- master track with independent sequences for pitch, octave and velocity
- slave tracks derive pitch/octave/velocity from master track
- all step sequencers can be set to arbitrary length from 2 to 10

Yet to be implemented:

- per-track and/or global modulation source
- keyboard shortcuts for quick navigation

Uses [PortAudio](https://github.com/PortAudio/portaudio) and [FTXUI](https://github.com/ArthurSonzogni/FTXUI/).

## how to build

install dependencies (for debian-based systems):

```bash
sudo apt install git build-essential cmake libportaudio2 portaudio19-dev libftxui-dev
```

clone repository:

```bash
git clone --recursive https://github.com/chrisherb/flechtbox.git
```

build:

```bash
cd flechtbox
mkdir build && cd build
cmake ..
make
```

run (from build folder):

```bash
./flechtbox
```
