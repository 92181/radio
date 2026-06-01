# Internet Radio
Internet radio built using FFmpeg and the native audio playback APIs for both Linux and macOS.

Of course, FFmpeg comes with a built-in internet radio player. However, it was not so much the aim of this project to reinvent the wheel, but to gain a deeper understanding of POSIX audio APIs.

## Usage
Before compiling you need to install the FFmpeg development headers, and for Linux also the pipewire one.<br>

- Ubuntu/Debian: `sudo apt install libavcodec-dev libavformat-dev libavutil-dev libpipewire-0.3-dev`
- Arch Linux: `sudo pacman -S ffmpeg pipewire`
- MacOS: `brew install ffmpeg`

C Usage: 
```sh
sh compile.sh && ./play 'http://uk7.internet-radio.com/proxy/boxradiouk?mp=/stream'
```

Python Usage:
```sh
sh compile.sh -py && python3 play.py`
```

## FFmpeg Alternative
```sh
ffplay -autoexit -nodisp "http://uk7.internet-radio.com/proxy/boxradiouk?mp=/stream"
```

## Author
Wolf Pieter Schulz. Public domain, no license.