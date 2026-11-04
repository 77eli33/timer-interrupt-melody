#ifndef MELODIES_H
#define MELODIES_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t frequency_hz;
    uint32_t duration_ms;
} MelodyNote;

typedef struct {
    const MelodyNote *notes;
    size_t length;
} Melody;

static const MelodyNote melody_ascending[] = {
    {1047u, 250u}, {1319u, 250u}, {1568u, 250u}, {2093u, 500u}
};

static const MelodyNote melody_descending[] = {
    {2093u, 200u}, {1760u, 200u}, {1397u, 200u}, {1047u, 500u}
};

static const MelodyNote melody_signal[] = {
    {1568u, 150u}, {1976u, 150u}, {1568u, 150u}, {2093u, 450u}
};

#define MELODY_LENGTH(array) (sizeof(array) / sizeof((array)[0]))

static const Melody melodies[] = {
    {melody_ascending, MELODY_LENGTH(melody_ascending)},
    {melody_descending, MELODY_LENGTH(melody_descending)},
    {melody_signal, MELODY_LENGTH(melody_signal)}
};

#endif
