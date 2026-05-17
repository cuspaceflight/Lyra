# Lyra

## Build Instructions
### With nix

Run
```
nix build
```
which should download the required packages and build the program, placing
the built packages in `result` by default

### Other
Requires `pico-sdk`

```
cmake -DPICO_SDK_PATH='PICO_SDK_PATH' -B build .
cmake --build build
```
which should generate `build/src/Lyra.uf2`

### Logger

The logger can be enabled through additional flags that can
be set in cmake

```
-DLOG_ENABLE=1 # Enable the Logger
-DLOG_LEVEL=LOG_LEVEL_{VERBOSE|INFO|DEBUG|ERROR|CRITICAL} # Set the logging level
-DLOG_ENABLE_COLOUR=1 # Enable colours in the output
```
