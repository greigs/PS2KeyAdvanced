/*  Simple keyboard to serial port at 115200 baud

  PS2KeyAdvanced library example

  Advanced support PS2 Keyboard to get every key code byte from a PS2 Keyboard
  for testing purposes.

  IMPORTANT WARNING

    If using a DUE or similar board with 3V3 I/O you MUST put a level translator
    like a Texas Instruments TXS0102 or FET circuit as the signals are
    Bi-directional (signals transmitted from both ends on same wire).

    Failure to do so may damage your Arduino Due or similar board.

  Test History
    September 2014 Uno and Mega 2560 September 2014 using Arduino V1.6.0
    January 2016   Uno, Mega 2560 and Due using Arduino 1.6.7 and Due Board
                    Manager V1.6.6

  This is for a LATIN style keyboard using Scan code set 2. See various
  websites on what different scan code sets use. Scan Code Set 2 is the
  default scan code set for PS2 keyboards on power up.

  Will support most keyboards even ones with multimedia keys or even 24 function keys.

  The circuit:
   * KBD Clock (PS2 pin 1) to an interrupt pin on Arduino ( this example pin 3 )
   * KBD Data (PS2 pin 5) to a data pin ( this example pin 4 )
   * +5V from Arduino to PS2 pin 4
   * GND from Arduino to PS2 pin 3

   The connector to mate with PS2 keyboard is a 6 pin Female Mini-Din connector
   PS2 Pins to signal
    1       KBD Data
    3       GND
    4       +5V
    5       KBD Clock

   Keyboard has 5V and GND connected see plenty of examples and
   photos around on Arduino site and other sites about the PS2 Connector.

 Interrupts

   Clock pin from PS2 keyboard MUST be connected to an interrupt
   pin, these vary with the different types of Arduino

  PS2KeyAdvanced requires both pins specified for begin()

    keyboard.begin( data_pin, irq_pin );

  Valid irq pins:
     Arduino Uno:  2, 3
     Arduino Due:  All pins, except 13 (LED)
     Arduino Mega: 2, 3, 18, 19, 20, 21
     Teensy 2.0:   All pins, except 13 (LED)
     Teensy 2.0:   5, 6, 7, 8
     Teensy 1.0:   0, 1, 2, 3, 4, 6, 7, 16
     Teensy++ 2.0: 0, 1, 2, 3, 18, 19, 36, 37
     Teensy++ 1.0: 0, 1, 2, 3, 18, 19, 36, 37
     Sanguino:     2, 10, 11

  Read method Returns an UNSIGNED INT containing
        Make/Break status
        Caps status
        Shift, CTRL, ALT, ALT GR, GUI keys
        Flag for function key not a displayable/printable character
        8 bit key code

  Code Ranges (bottom byte of unsigned int)
        0       invalid/error
        1-1F    Functions (Caps, Shift, ALT, Enter, DEL... )
        1A-1F   Functions with ASCII control code
                    (DEL, BS, TAB, ESC, ENTER, SPACE)
        20-61   Printable characters noting
                    0-9 = 0x30 to 0x39 as ASCII
                    A to Z = 0x41 to 0x5A as upper case ASCII type codes
                    8B Extra European key
        61-A0   Function keys and other special keys (plus F2 and F1)
                    61-78 F1 to F24
                    79-8A Multimedia
                    8B NOT included
                    8C-8E ACPI power
                    91-A0 and F2 and F1 - Special multilingual
        A8-FF   Keyboard communications commands (note F2 and F1 are special
                codes for special multi-lingual keyboards)

    By using these ranges it is possible to perform detection of any key and do
    easy translation to ASCII/UTF-8 avoiding keys that do not have a valid code.

    Top Byte is 8 bits denoting as follows with defines for bit code

        Define name bit     description
        PS2_BREAK   15      1 = Break key code
                   (MSB)    0 = Make Key code
        PS2_SHIFT   14      1 = Shift key pressed as well (either side)
                            0 = NO shift key
        PS2_CTRL    13      1 = Ctrl key pressed as well (either side)
                            0 = NO Ctrl key
        PS2_CAPS    12      1 = Caps Lock ON
                            0 = Caps lock OFF
        PS2_ALT     11      1 = Left Alt key pressed as well
                            0 = NO Left Alt key
        PS2_ALT_GR  10      1 = Right Alt (Alt GR) key pressed as well
                            0 = NO Right Alt key
        PS2_GUI      9      1 = GUI key pressed as well (either)
                            0 = NO GUI key
        PS2_FUNCTION 8      1 = FUNCTION key non-printable character (plus space, tab, enter)
                            0 = standard character key

  Error Codes
     Most functions return 0 or 0xFFFF as error, other codes to note and
     handle appropriately
        0xAA   keyboard has reset and passed power up tests
               will happen if keyboard plugged in after code start
        0xFC   Keyboard General error or power up fail

  See PS2Keyboard.h file for returned definitions of Keys

  Note defines starting
            PS2_KEY_* are the codes this library returns
            PS2_*     remaining defines for use in higher levels

  To get the key as ASCII/UTF-8 single byte character conversion requires use
  of PS2KeyMap library AS WELL.

  Written by Paul Carpenter, PC Services <sales@pcserviceselectronics.co.uk>
*/

#include <PS2KeyAdvanced.h>

/* Keyboard constants  Change to suit your Arduino
   define pins used for data and clock from keyboard */
#define DATAPIN 8
#define IRQPIN  3

char val;
uint16_t c;

PS2KeyAdvanced keyboard;

char asciiMap[256] = {
' ', // 0
' ', // 1
' ', // 2
' ', // 3
' ', // 4
' ', // 5
' ', // 6
' ', // 7
' ', // 8
' ', // 9
' ', // 10
' ', // 11
' ', // 12
' ', // 13
' ', // 14
' ', // 15
' ', // 16
' ', // 17
' ', // 18
' ', // 19
' ', // 20
' ', // 21
' ', // 22
' ', // 23
' ', // 24
' ', // 25
' ', // 26
' ', // 27
' ', // 28
' ', // 29
' ', // 30
' ', // 31
' ', // 32
'!', // 33
'"', // 34
'#', // 35
'$', // 36
'%', // 37
'&', // 38
'\'', // 39
'(', // 40
')', // 41
'*', // 42
'+', // 43
',', // 44
'-', // 45
'.', // 46
'/', // 47
'0', // 48
'1', // 49
'2', // 50
'3', // 51
'4', // 52
'5', // 53
'6', // 54
'7', // 55
'8', // 56
'9', // 57
':', // 58
';', // 59
'<', // 60
'=', // 61
'>', // 62
'?', // 63
'@', // 64
'A', // 65
'B', // 66
'C', // 67
'D', // 68
'E', // 69
'F', // 70
'G', // 71
'H', // 72
'I', // 73
'J', // 74
'K', // 75
'L', // 76
'M', // 77
'N', // 78
'O', // 79
'P', // 80
'Q', // 81
'R', // 82
'S', // 83
'T', // 84
'U', // 85
'V', // 86
'W', // 87
'X', // 88
'Y', // 89
'Z', // 90
'[', // 91
'\\', // 92
']', // 93
'^', // 94
'_', // 95
'`', // 96
'a', // 97
'b', // 98
'c', // 99
'd', // 100
'e', // 101
'f', // 102
'g', // 103
'h', // 104
'i', // 105
'j', // 106
'k', // 107
'l', // 108
'm', // 109
'n', // 110
'o', // 111
'p', // 112
'q', // 113
'r', // 114
's', // 115
't', // 116
'u', // 117
'v', // 118
'w', // 119
'x', // 120
'y', // 121
'z', // 122
'{', // 123
'|', // 124
'}', // 125
'~', // 126
'', // 127
'€', // 128
'', // 129
'‚', // 130
'ƒ', // 131
'„', // 132
'…', // 133
'†', // 134
'‡', // 135
'ˆ', // 136
'‰', // 137
'Š', // 138
'‹', // 139
'Œ', // 140
'', // 141
'Ž', // 142
'', // 143
'', // 144
'‘', // 145
'’', // 146
'“', // 147
'”', // 148
'•', // 149
'–', // 150
'—', // 151
'˜', // 152
'™', // 153
'š', // 154
'›', // 155
'œ', // 156
'', // 157
'ž', // 158
'Ÿ', // 159
' ', // 160
'¡', // 161
'¢', // 162
'£', // 163
'¤', // 164
'¥', // 165
'¦', // 166
'§', // 167
'¨', // 168
'©', // 169
'ª', // 170
'«', // 171
'¬', // 172
'­', // 173
'®', // 174
'¯', // 175
'°', // 176
'±', // 177
'²', // 178
'³', // 179
'´', // 180
'µ', // 181
'¶', // 182
'·', // 183
'¸', // 184
'¹', // 185
'º', // 186
'»', // 187
'¼', // 188
'½', // 189
'¾', // 190
'¿', // 191
'À', // 192
'Á', // 193
'Â', // 194
'Ã', // 195
'Ä', // 196
'Å', // 197
'Æ', // 198
'Ç', // 199
'È', // 200
'É', // 201
'Ê', // 202
'Ë', // 203
'Ì', // 204
'Í', // 205
'Î', // 206
'Ï', // 207
'Ð', // 208
'Ñ', // 209
'Ò', // 210
'Ó', // 211
'Ô', // 212
'Õ', // 213
'Ö', // 214
'×', // 215
'Ø', // 216
'Ù', // 217
'Ú', // 218
'Û', // 219
'Ü', // 220
'Ý', // 221
'Þ', // 222
'ß', // 223
'à', // 224
'á', // 225
'â', // 226
'ã', // 227
'ä', // 228
'å', // 229
'æ', // 230
'ç', // 231
'è', // 232
'é', // 233
'ê', // 234
'ë', // 235
'ì', // 236
'í', // 237
'î', // 238
'ï', // 239
'ð', // 240
'ñ', // 241
'ò', // 242
'ó', // 243
'ô', // 244
'õ', // 245
'ö', // 246
'÷', // 247
'ø', // 248
'ù', // 249
'ú', // 250
'û', // 251
'ü', // 252
'ý', // 253
'þ', // 254
'ÿ' // 255
};



char dvorakMap[256] = {
' ', // 0
' ', // 1
' ', // 2
' ', // 3
' ', // 4
' ', // 5
' ', // 6
' ', // 7
' ', // 8
' ', // 9
' ', // 10
' ', // 11
' ', // 12
' ', // 13
' ', // 14
' ', // 15
' ', // 16
' ', // 17
' ', // 18
' ', // 19
' ', // 20
' ', // 21
' ', // 22
' ', // 23
' ', // 24
' ', // 25
' ', // 26
' ', // 27
' ', // 28
' ', // 29
' ', // 30
' ', // 31
' ', // 32
'!', // 33
'"', // 34
'#', // 35
'$', // 36
'%', // 37
'&', // 38
'\'', // 39
'(', // 40
')', // 41
'*', // 42
'+', // 43
',', // 44
'-', // 45
'.', // 46
'/', // 47
'0', // 48
'1', // 49
'2', // 50
'3', // 51
'4', // 52
'5', // 53
'6', // 54
'7', // 55
'8', // 56
'9', // 57
':', // 58
';', // 59
'<', // 60
'=', // 61
'>', // 62
'?', // 63
'@', // 64
'A', // 65
'B', // 66
'C', // 67
'D', // 68
'O', // 69
'F', // 70
'G', // 71
'H', // 72
'I', // 73
'J', // 74
'K', // 75
'L', // 76
'M', // 77
'N', // 78
'O', // 79
'P', // 80
'D', // 81
'R', // 82
'S', // 83
'A', // 84
'U', // 85
'V', // 86
'V', // 87
'X', // 88
'K', // 89
'Z', // 90
'[', // 91
'\\', // 92
']', // 93
'^', // 94
'_', // 95
'`', // 96
'a', // 97
'b', // 98
'c', // 99
'd', // 100
'e', // 101
'f', // 102
'g', // 103
'h', // 104
'i', // 105
'j', // 106
'k', // 107
'l', // 108
'm', // 109
'n', // 110
'o', // 111
'p', // 112
'q', // 113
'r', // 114
's', // 115
't', // 116
'u', // 117
'v', // 118
'w', // 119
'x', // 120
'y', // 121
'z', // 122
'{', // 123
'|', // 124
'}', // 125
'~', // 126
'', // 127
'€', // 128
'', // 129
'‚', // 130
'ƒ', // 131
'„', // 132
'…', // 133
'†', // 134
'‡', // 135
'ˆ', // 136
'‰', // 137
'Š', // 138
'‹', // 139
'Œ', // 140
'', // 141
'Ž', // 142
'', // 143
'', // 144
'‘', // 145
'’', // 146
'“', // 147
'”', // 148
'•', // 149
'–', // 150
'—', // 151
'˜', // 152
'™', // 153
'š', // 154
'›', // 155
'œ', // 156
'', // 157
'ž', // 158
'Ÿ', // 159
' ', // 160
'¡', // 161
'¢', // 162
'£', // 163
'¤', // 164
'¥', // 165
'¦', // 166
'§', // 167
'¨', // 168
'©', // 169
'ª', // 170
'«', // 171
'¬', // 172
'­', // 173
'®', // 174
'¯', // 175
'°', // 176
'±', // 177
'²', // 178
'³', // 179
'´', // 180
'µ', // 181
'¶', // 182
'·', // 183
'¸', // 184
'¹', // 185
'º', // 186
'»', // 187
'¼', // 188
'½', // 189
'¾', // 190
'¿', // 191
'À', // 192
'Á', // 193
'Â', // 194
'Ã', // 195
'Ä', // 196
'Å', // 197
'Æ', // 198
'Ç', // 199
'È', // 200
'É', // 201
'Ê', // 202
'Ë', // 203
'Ì', // 204
'Í', // 205
'Î', // 206
'Ï', // 207
'Ð', // 208
'Ñ', // 209
'Ò', // 210
'Ó', // 211
'Ô', // 212
'Õ', // 213
'Ö', // 214
'×', // 215
'Ø', // 216
'Ù', // 217
'Ú', // 218
'Û', // 219
'Ü', // 220
'Ý', // 221
'Þ', // 222
'ß', // 223
'à', // 224
'á', // 225
'â', // 226
'ã', // 227
'ä', // 228
'å', // 229
'æ', // 230
'ç', // 231
'è', // 232
'é', // 233
'ê', // 234
'ë', // 235
'ì', // 236
'í', // 237
'î', // 238
'ï', // 239
'ð', // 240
'ñ', // 241
'ò', // 242
'ó', // 243
'ô', // 244
'õ', // 245
'ö', // 246
'÷', // 247
'ø', // 248
'ù', // 249
'ú', // 250
'û', // 251
'ü', // 252
'ý', // 253
'þ', // 254
'ÿ' // 255
};


bool doTheDvorakMap = false;


void setup()
{
// Configure the keyboard library
keyboard.begin( DATAPIN, IRQPIN );
Serial.begin( 115200 );
#if defined(ARDUINO_ARCH_AVR)
Serial.println( F( "PS2 Advanced Key Simple Test:" ) );
#elif defined(ARDUINO_ARCH_SAM)
Serial.println( "PS2 Advanced Key Simple Test:" );
#endif
}


void loop()
{
if( keyboard.available() )
  {
  // read the next key
  c = keyboard.read();
  if( c > 0 )
    {
#if defined(ARDUINO_ARCH_AVR)
//    Serial.print( F( "Value " ) );
//    Serial.print( c, HEX );
//    Serial.print( F( " - Status Bits " ) );
//    Serial.print( c >> 8, HEX );
//    Serial.print( F( "  Code " ) );
#elif defined(ARDUINO_ARCH_SAM)
//    Serial.print( "Value " );
//    Serial.print( c, HEX );
//   Serial.print( " - Status Bits " );
//    Serial.print( c >> 8, HEX );
//    Serial.print( "  Code " );
#endif
    //Serial.println( c & 0xFF, HEX );

    val = mapVal(c & 0xFF);
    if (val == ','){
      doTheDvorakMap = true;
    }
    if (val == '.'){
      doTheDvorakMap = false;
    }

    if (doTheDvorakMap){
      val = mapValDvorak(c & 0xFF);
    }
    
    if (c >> 8 == 0){
      Serial.print( val );
    }
    }
  }
}

char mapVal(byte b)
{
  return asciiMap[b];
}

char mapValDvorak(byte b)
{
  return dvorakMap[b];
}



