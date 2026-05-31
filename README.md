# Internet Radio
Internet radio built using FFmpeg and the native audio playback API's for both Linux and MacOS.
Ofcourse FFmpeg comes with a built in internet radio player, however it was not so much the aim of this project to reinvent 
the wheel but to gain a deeper understanding of POSIX audio API's.

# Usage
C Demo: `sh compile.sh && ./play 'http://uk7.internet-radio.com/proxy/boxradiouk?mp=/stream'`

Python Demo: `sh compile.sh -py && python3 play.py`

# FFmpeg Alternative
ffplay -autoexit -nodisp "http://uk7.internet-radio.com/proxy/boxradiouk?mp=/stream"

# Author
Wolf Pieter Schulz. Public Domain.