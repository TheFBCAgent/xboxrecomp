/**
 * Generic NV2A pushbuffer frame capture + replay (plan step 2.H.2).
 *
 * Capture: while RECOMP_PB_CAPTURE=<path> is set, every decoded pushbuffer
 * method is mirrored into a shadow table (last value per (subchannel, method))
 * and, for one frame, into a method stream. RECOMP_PB_CAPTURE_FLIP=<N> picks
 * the frame (the Nth FLIP_STALL); the default is the first frame. Alternatively
 * RECOMP_PB_CAPTURE_MS=<T> arms at the first FLIP_STALL on/after T ms since
 * boot (it overrides the flip count), which is the reproducible choice when
 * the frame to capture follows a wall-clock trigger such as a scripted pad
 * event. At the frame boundary the tool writes a self-contained file holding:
 *
 *   - the three guest-memory regions the executor's draws read, snapshotted at
 *     the start of the frame: main RAM (VA 0, 64 MiB, the image + heap that
 *     dma_resolve can fall through to), the contiguous window (0x80000000,
 *     64 MiB, textures / vertex arrays / surfaces) and the NV2A aperture
 *     (0xFD000000, 16 MiB, RAMHT + PRAMIN DMA-object tables);
 *   - a "prelude": the shadow state replayed as ordinary commands, plus the
 *     vertex program and constants re-uploaded as load + data sequences;
 *   - the frame's own decoded (subchannel, method, param) stream.
 *
 * Nothing here snapshots the executor's own structs, so the same capture feeds
 * any renderer (the software executor now, xemu's PGRAPH later): each renderer
 * replays "prelude + frame" and ends in the same state.
 *
 * Replay: RECOMP_PB_REPLAY=<path> restores the three memory regions, replays
 * the prelude then the frame through the software executor, and writes a
 * 24-bit BMP of the frame's drawn surface next to the capture file.
 */
#ifndef XBOXRECOMP_NV2A_PB_CAPTURE_H
#define XBOXRECOMP_NV2A_PB_CAPTURE_H

#include <stdint.h>

/* Called for every decoded method, before the executor handles it. */
void nv2a_pb_capture_method(uint32_t subch, uint32_t method, uint32_t param);

/* Called at a frame boundary (FLIP_STALL): arms/disarms and writes the file. */
void nv2a_pb_capture_flip(void);

/* Replay a captured frame: stop the ack thread, restore the snapshotted
 * memory, run prelude + frame, dump a BMP. Returns 0 on success. Called from
 * main.c instead of booting the game. */
int nv2a_pb_capture_replay(const char *path);

#endif
