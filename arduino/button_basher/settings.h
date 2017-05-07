#include "Arduino.h"

/**************************
*  KEY CODES AND OPTIONS  *
***************************/

int keyCodesAndOptions[NUM_INPUTS][OPTIONS_PER_INPUT] =
{
  // Output 1,     Output 2,       Output 3,     Option 4,     key,    // Position on MM
  { KEY_LEFT_CTRL, KEY_LEFT_SHIFT, KEY_LEFT_ALT, 'a' },  // left arrow pad
  { KEY_LEFT_CTRL, KEY_LEFT_SHIFT, KEY_LEFT_ALT, 'b' },  // pin D5
  { KEY_LEFT_CTRL, KEY_LEFT_SHIFT, KEY_LEFT_ALT, 'c' },  // pin D4
  { KEY_LEFT_CTRL, KEY_LEFT_SHIFT, KEY_LEFT_ALT, 'd' },  // pin D3
  { KEY_LEFT_CTRL, KEY_LEFT_SHIFT, KEY_LEFT_ALT, 'e' },  // pin D2
  { KEY_LEFT_CTRL, KEY_LEFT_SHIFT, KEY_LEFT_ALT, 'f' },  // pin D1
  { KEY_LEFT_CTRL, KEY_LEFT_SHIFT, KEY_LEFT_ALT, 'g' },  // pin D0
  { KEY_LEFT_CTRL, KEY_LEFT_SHIFT, KEY_LEFT_ALT, 'h' },  // pin A5
  { KEY_LEFT_CTRL, KEY_LEFT_SHIFT, KEY_LEFT_ALT, 'i' },  // pin A4
  { KEY_LEFT_CTRL, KEY_LEFT_SHIFT, KEY_LEFT_ALT, 'j' },  // pin A3
  { KEY_LEFT_CTRL, KEY_LEFT_SHIFT, KEY_LEFT_ALT, 'k' },  // pin A2
  { KEY_LEFT_CTRL, KEY_LEFT_SHIFT, KEY_LEFT_ALT, 'l' },  // pin A1
  { KEY_LEFT_CTRL, KEY_LEFT_SHIFT, KEY_LEFT_ALT, 'm' },  // pin A0
};

///////////////////////////
// NOISE CANCELLATION /////
///////////////////////////
#define SWITCH_THRESHOLD_OFFSET_PERC  5    // number between 1 and 49
                                           // larger value protects better against noise oscillations, but makes it harder to press and release
                                           // recommended values are between 2 and 20
                                           // default value is 5

#define SWITCH_THRESHOLD_CENTER_BIAS 55   // number between 1 and 99
                                          // larger value makes it easier to "release" keys, but harder to "press"
                                          // smaller value makes it easier to "press" keys, but harder to "release"
                                          // recommended values are between 30 and 70
                                          // 50 is "middle" 2.5 volt center
                                          // default value is 55
                                          // 100 = 5V (never use this high)
                                          // 0 = 0 V (never use this low
