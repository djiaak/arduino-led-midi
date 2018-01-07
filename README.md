Arduino LED strip MIDI output
=============================

Use an ATmega32U4 based Arduino as a MIDI output device for controlling an AdaFruit DotStar LED strip (https://www.adafruit.com/product/2238). Place the LED strip above a piano keyboard and send MIDI note on/off commands to light up the corresponding keys!

TODO piano and LED layout is still hardcoded in the .ino file (NUM_LEDS, NOTE_FROM, NOTE_TO) - add MIDI system exclusive command support to redefine these
