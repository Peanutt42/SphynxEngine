## Building
- Use cmake
- Platforms supported: Windows and Linux
- Linux specific:
    - For audio (openal-soft), install a audio backend, like:
        - PulseAudio: 'libpulse-dev' or PipeWire: 'libpipewire-0.3-dev' or JACK: 'libjack-dev'
    - Install packages needed to compile: libxrandr-dev, libxinerama-dev, libxcursor-dev, libxi-dev, libopengl-dev
    - For better stacktraces, install: binutils-dev
    - Example command:
        ```bash
        sudo apt install libpipewire-0.3-dev binutils-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libopengl-dev
        ```