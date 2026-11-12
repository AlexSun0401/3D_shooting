# Build and run

The game uses direct memory-mapped I/O. Building it and running it are therefore separate concerns: an ordinary desktop toolchain can check the C source, but only a compatible DE1-SoC hardware environment or simulator can execute it safely.

## Repository inputs

A complete build must include:

- every implementation file in `src/`;
- both generated media units in `assets/`; and
- `include/` on the compiler's header search path.

Leaving out an asset unit causes unresolved references to the gun image or sound arrays.

## Host validation

With GNU Make and a C11 compiler installed, run:

```bash
make check
make
```

`make check` performs a syntax-only pass over the complete project. `make` also links the modules and writes `build/3d_shooting`.

Run `make test` for the host-safe DDA and shooting regression checks. This separate test executable links only software modules and in-memory sound state, so it does not access MMIO. It covers cardinal rays, camera projection, grid-corner ties, wall-distance clamping, target ordering/occlusion, visibility, cooldown, scoring, and hit-sound state.

> [!WARNING]
> Do not run the host-built executable. It will attempt to dereference the fixed VGA, PS/2, audio, character-buffer, and HEX addresses declared in `include/hw.h`.

The compiler can be overridden without changing the Makefile:

```bash
make clean
make CC=<target-compiler> CFLAGS="<target flags>"
```

Clean first whenever switching compilers or target flags so that previously generated host objects are not reused.

Use `make clean` to remove only the generated `build/` directory.

## Target integration

Use the startup, linker, and loading flow supplied by your DE1-SoC course environment, simulator, or board-support package.

1. Confirm that the peripheral and SDRAM addresses match `include/hw.h`.
2. Configure the compiler for the processor used by that environment.
3. Add `include/` to the header search path.
4. Compile and link all files matched by `src/*.c` and `assets/*.c`.
5. Load the resulting target image through the environment's normal bare-metal workflow.
6. Attach or enable VGA output, a PS/2 keyboard and mouse, audio output, and the HEX display.

The source assumes a 320 × 240 visible frame with a 512-pixel row stride. Two frame buffers are selected near the upper end of the declared SDRAM range, so that memory must be writable and must not overlap the target image, stack, heap, or other reserved regions.

Mouse initialization sends the PS/2 `0xF4` enable command and waits for a `0xFA` acknowledgement before continuing. The target environment must provide that response or the title screen will not be reached.

## Expected startup

1. The VGA output shows the title screen.
2. Press `G` to initialize a game.
3. The character buffer displays the score, position, and control summary.
4. The HEX display mirrors the score.
5. After success, health depletion, or a manual exit, press `R` to return to the title screen.

## Troubleshooting

| Symptom | Check |
| --- | --- |
| Undefined `gun`, `soundfire`, or `soundshot` symbols | Ensure both files in `assets/` are compiled and linked. |
| Undefined `title_img` or `draw_title` | Ensure `src/cover.c` is compiled and linked. |
| Header not found | Add `-Iinclude` or the equivalent include-path setting. |
| Host executable crashes immediately | This is expected if it is run outside the target memory map. |
| Title screen never appears | Confirm that the PS/2 mouse is present and returns the `0xFA` enable acknowledgement. |
| VGA remains blank or buffer swapping stalls | Verify the pixel-buffer controller address, SDRAM range, and status-register semantics. |
| Keyboard works but the mouse does not | Verify the second PS/2 port, its receive-valid bit, and the `0xF4` enable response. |
| Audio is silent | Verify the audio-core address, FIFO-space field, and left/right data registers. |
