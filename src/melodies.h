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

// Super Mario Bros. (NES) course-clear fanfare, one octave up for the buzzer.
static const MelodyNote melody_mario_course_clear[] = {
    // C/G arpeggio
    {392u, 132u}, {523u, 132u}, {659u, 132u}, {784u, 132u},
    {1047u, 132u}, {1319u, 132u}, {1568u, 264u}, {1319u, 192u},
    // Ab arpeggio
    {415u, 132u}, {523u, 132u}, {622u, 132u}, {831u, 132u},
    {1047u, 132u},
    {1245u, 132u}, {1661u, 264u}, {1245u, 192u},
    // Bb arpeggio and final C
    {466u, 132u}, {587u, 132u}, {698u, 132u}, {932u, 132u},
    {1175u, 132u},
    {1397u, 132u}, {1865u, 264u}, {1865u, 192u}, {1865u, 192u},
    {1865u, 192u}, {2093u, 720u}
};

static const MelodyNote melody_descending[] = {
    {2093u, 200u}, {1760u, 200u}, {1397u, 200u}, {1047u, 500u}
};

// Four 4/4 bars at 108 BPM (8889 ms total, rounded to whole milliseconds).
static const MelodyNote melody_last_christmas[] = {
    // Bar 1
    {1319u, 833u}, {1319u, 278u}, {1175u, 1111u},
    // Bar 2
    {880u, 277u}, {1319u, 278u}, {1319u, 278u},
    {1480u, 278u}, {1175u, 1111u},
    // Bar 3
    {988u, 278u}, {988u, 278u}, {1319u, 278u},
    {1319u, 278u}, {1480u, 555u}, {1175u, 555u},
    // Bar 4
    {988u, 278u}, {1109u, 278u}, {1175u, 278u},
    {1109u, 278u}, {988u, 1111u}
};

#define MELODY_LENGTH(array) (sizeof(array) / sizeof((array)[0]))

static const Melody melodies[] = {
    {melody_mario_course_clear, MELODY_LENGTH(melody_mario_course_clear)},
    {melody_descending, MELODY_LENGTH(melody_descending)},
    {melody_last_christmas, MELODY_LENGTH(melody_last_christmas)}
};

#endif
