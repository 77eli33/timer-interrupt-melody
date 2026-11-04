#ifndef MELODIES_H
#define MELODIES_H

#include <stddef.h>
#include <stdint.h>

/*
 * Melody data stays separate from playback control. The timer module reads
 * each pair as a frequency in hertz and a duration in milliseconds. A zero
 * frequency with a positive duration is a timed rest.
 */
typedef struct {
    uint32_t frequency_hz;
    uint32_t duration_ms;
} MelodyNote;

// The explicit length lets main() stop before reading past the note array.
typedef struct {
    const MelodyNote *notes;
    size_t length;
} Melody;

// Melody 1: an approximate, single-voice NES course-clear fanfare. The
// arpeggios are one octave above the transcription for audible buzzer notes;
// the note durations were later lengthened by 20 percent.
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

// Melody 2: default Microsoft Teams incoming-call ringtone, reduced from
// its multi-voice audio to the upper notes that one buzzer can reproduce.
// The B-flat and F notes are one octave above the audio reference so the
// passive buzzer can reproduce them clearly. Zero hertz marks a timed rest.
static const MelodyNote melody_teams_ring[] = {
    // First ring phrase: B-flat, B-flat, B-flat, F, B-flat, B-flat.
    {932u, 300u}, {0u, 400u}, {932u, 300u}, {0u, 150u},
    {932u, 250u}, {0u, 200u}, {1397u, 160u}, {0u, 100u},
    {932u, 300u}, {0u, 400u}, {932u, 300u}, {0u, 690u},
    // The original ringtone repeats the phrase once.
    {932u, 300u}, {0u, 400u}, {932u, 300u}, {0u, 150u},
    {932u, 250u}, {0u, 200u}, {1397u, 160u}, {0u, 100u},
    {932u, 300u}, {0u, 400u}, {932u, 300u}
};

// Melody 3: four 4/4 bars of a simplified Last Christmas refrain at 108 BPM.
// Durations are rounded to milliseconds (8889 ms total).
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

// Compute each array's element count at compile time; no sentinel is needed.
#define MELODY_LENGTH(array) (sizeof(array) / sizeof((array)[0]))

// S1 cycles through these entries in order; S2 plays the selected entry.
static const Melody melodies[] = {
    {melody_mario_course_clear, MELODY_LENGTH(melody_mario_course_clear)},
    {melody_teams_ring, MELODY_LENGTH(melody_teams_ring)},
    {melody_last_christmas, MELODY_LENGTH(melody_last_christmas)}
};

#endif
