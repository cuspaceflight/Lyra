# Lyra

## Build Instructions
### With nix

Run
```
nix build
```
which should download the required packages and build the program

### Other
Requires `pico-sdk`

```
cmake -DPICO_SDK_PATH='PICO_SDK_PATH' -B build .
cmake --build build
```
which should generate `build/src/Lyra.uf2`
