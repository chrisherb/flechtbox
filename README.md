# flechtbox

Terminal-based groovebox.

![flechtbox screenshot](screenshot.png)

## features

- 9 tracks with probability-based step sequencers
- instance of Mutable Instruments Plaits per track with 24 synthesizer engines
- master track with independent sequences for pitch, octave and velocity
- slave tracks derive pitch/octave/velocity from master track
- all step sequencers can be set to arbitrary length from 2 to 10

Yet to be implemented:

- per-track and/or global modulation source
- send fx
- quantizer with selectable global scale
- midi sync
- (per-step) clock divison

Uses [PortAudio](https://github.com/PortAudio/portaudio) and [FTXUI](https://github.com/ArthurSonzogni/FTXUI/).

## how to control

To navigate between controls, use either the arrow keys or hjkl.
To manipulate parameters, use ctrl+up/down or K/J.

Other keybinds:

- 1-9: select track
- 0: select master track
- F1: start/stop
- m: mute selected track

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
