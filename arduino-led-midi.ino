#include <EEPROM.h>
#include "MIDIUSB.h"
#include "FastLED.h"

#define NUM_LEDS 144
#define NOTE_FROM_DEFAULT 29
#define NOTE_TO_DEFAULT 101


#define COLOR_ORDER GRB
#define DATA_PIN 5
#define CLOCK_PIN 4
#define LED_TYPE APA102
#define MIDI_NOTE_COUNT 128

#define MIDI_NOTE_OFF 0x80
#define MIDI_NOTE_ON 0x90

#define FRAMES_PER_SECOND  120
#define BRIGHTNESS          96

#define NOTE_ON_COLOR_NATURAL CRGB(200,200,200)
#define NOTE_ON_COLOR_ACCIDENTAL CRGB(10,200,200)
#define NOTE_OFF_COLOR CRGB(0,0,0)
#define NOTES_PER_OCTAVE 12

//#define DEBUG

CRGB leds[NUM_LEDS];

int midiToLedMapping[MIDI_NOTE_COUNT];

int noteFrom;
int noteTo;

float KEY_COORDS[NOTES_PER_OCTAVE] = { 0.07, 0.14, 0.21, 0.28, 0.36, 0.5, 0.57, 0.64, 0.71, 0.78, 0.86, 0.93 };

/**
   C   C#  D   D#  E   F   F#  G   G#  A   A#  B
0  0   1   2   3   4   5   6   7   8   9   10  11
1  12  13  14  15  16  17  18  19  20  21  22  23
2  24  25  26  27  28  29  30  31  32  33  34  35
3  36  37  38  39  40  41  42  43  44  45  46  47
4  48  49  50  51  52  53  54  55  56  57  58  59
5  60* 61  62  63  64  65  66  67  68  69  70  71
6  72  73  74  75  76  77  78  79  80  81  82  83
7  84  85  86  87  88  89  90  91  92  93  94  95
8  96  97  98  99  100 101 102 103 104 105 106 107
9  108 109 110 111 112 113 114 115 116 117 118 119
10 120 121 122 123 124 125 126 127
**/

bool isAccidental(int note) {
  int inFirstOctave = note % NOTES_PER_OCTAVE;
  return inFirstOctave == 1 || inFirstOctave == 3 || inFirstOctave == 6 || inFirstOctave == 8 || inFirstOctave == 10;
}

float getNoteCoord(int note) {
  int inFirstOctave = note % NOTES_PER_OCTAVE;
  int noteOctave = note / NOTES_PER_OCTAVE;
  return (float)KEY_COORDS[inFirstOctave] + noteOctave;
}

int getMidiToLedMapping(int note, int noteFrom, int noteTo, int ledCount) {
  int noteCount = noteTo - noteFrom + 1;
  float ledsPerNote = ledCount / (float)noteCount;
  float noteCoord = getNoteCoord(note);
  float noteFromCoord = getNoteCoord(noteFrom);
  int ledMapping = (int)((noteCoord - noteFromCoord) * NOTES_PER_OCTAVE * ledsPerNote);
  
  return ledMapping;
}

void initMidiToLedMapping(int noteFrom, int noteTo, int ledCount) {
  for (int i=noteFrom; i <= noteTo; i++) {
    midiToLedMapping[i] = getMidiToLedMapping(i, noteFrom, noteTo, ledCount);
  }
}

void testLeds() {
  for (int i=0; i<MIDI_NOTE_COUNT; i++) {
    setLedStatus(i, true);
  }
  FastLED.show();
}


void clearMidiToLedMapping() {
  for (int i=0; i<MIDI_NOTE_COUNT; i++) {
    midiToLedMapping[i] = -1;
  }
}

void setDefaultMidiToLedMapping() {
  for (int i=0; i<MIDI_NOTE_COUNT && i<NUM_LEDS; i++) {
    midiToLedMapping[i] = i;
  }
}

void clearLeds() {
  for (int i=0; i<NUM_LEDS; i++) {
    leds[i] = NOTE_OFF_COLOR;
  }
  FastLED.show();
}

void setLedStatus(int noteNumber, boolean ledState) {
  int index = midiToLedMapping[noteNumber];
  if (index==-1) return;
  CRGB noteOnColor = isAccidental(noteNumber) ? NOTE_ON_COLOR_ACCIDENTAL : NOTE_ON_COLOR_NATURAL;
  leds[index] = ledState ? noteOnColor : NOTE_OFF_COLOR;
}

void loadNoteRange() {
  byte value = EEPROM.read(0);
  int low = value >> 4;
  int high = value & 0xF;
  if ((low == 0 && high == 0) || 
    low >= high) {
    noteFrom = NOTE_FROM_DEFAULT;
    noteTo = NOTE_TO_DEFAULT;  
  } else {
    noteFrom = low;
    noteTo = high;
  }
}

void setup() {
  #ifdef DEBUG 
  Serial.begin(115200);
  while (!Serial);
  #endif

  loadNoteRange();
  clearMidiToLedMapping();
  initMidiToLedMapping(noteFrom, noteTo, NUM_LEDS);
   
  FastLED.addLeds<LED_TYPE,DATA_PIN,CLOCK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  LEDS.setBrightness(BRIGHTNESS);
  clearLeds();

  testLeds();
  delay(2000);
  clearLeds();

  #ifdef DEBUG 
  Serial.println("Arduino ready.");
  #endif
}

void loop() {
  midiEventPacket_t rx;
  do {
    rx = MidiUSB.read();
    if (rx.header != 0) {
      #ifdef DEBUG
      Serial.print("Received: ");
      Serial.print(rx.header, HEX);
      Serial.print("-");
      Serial.print(rx.byte1, HEX);
      Serial.print("-");
      Serial.print(rx.byte2, HEX);
      Serial.print("-");
      Serial.println(rx.byte3, HEX);
      #endif

      switch (rx.byte1) {
        case MIDI_NOTE_OFF:
          setLedStatus(rx.byte2, false);
          FastLED.show();
          FastLED.delay(1000/FRAMES_PER_SECOND);  
          break;
         case MIDI_NOTE_ON:
          setLedStatus(rx.byte2, true);
          FastLED.show();
          FastLED.delay(1000/FRAMES_PER_SECOND);  
          break;
      }
    }
  } while (rx.header != 0);
  
}
