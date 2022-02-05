#pragma once



//commands from "System" group (00)
#define TEXT_COLOR_DEFAULT u"\x0E\x00\x03\x02\xFF\xFF0E\x00\x03\x02\x00" //looks to set the text color to that textbox's default, specified in the TSY entry
#define TEXT_COLOR_RED u"\x0E\x00\x03\x02\x01"
#define TEXT_COLOR_GREEN u"\x0E\x00\x03\x02\x02"
#define TEXT_COLOR_BLUE u"\x0E\x00\x03\x02\x03"
#define TEXT_COLOR_YELLOW u"\x0E\x00\x03\x02\x04"
#define TEXT_COLOR_CYAN u"\x0E\x00\x03\x02\x05"
#define TEXT_COLOR_MAGENTA u"\x0E\x00\x03\x02\x06"
#define TEXT_COLOR_GRAY u"\x0E\x00\x03\x02\x07"
#define TEXT_COLOR_ORANGE u"\x0E\x00\x03\x02\x08"


//commands from "ControlTags" group (01)
#define DRAW_INSTANT u"\x0E\x01\x00\x00"
#define DRAW_CHAR u"\x0E\x01\x01\x00"

#define WAIT_DISMISS_PROMPT(frames) u"\x0E\x01\x02\x02" frames //input frames as a hex string
#define WAIT_DISMISS(frames) u"\x0E\x01\x03\x02" frames //input frames as a hex string
#define DISMISS(frames) u"\x0E\x01\x04\x02" frames //input frames as a hex string
#define STEP(frames) u"\x0E\x01\x05\x02" frames //input frames as a hex string, exact purpose unknown
#define WAIT(frames) u"\x0E\x01\x06\x02" frames //input frames as a hex string

#define TWO_CHOICES u"\x0E\x01\x07\x00" //two choices in a textbox
#define THREE_CHOICES u"\x0E\x01\x08\x00" //three choices in a textbox
#define TWO_CHOICES_LEFT u"\x0E\x01\x09\x00" //first choice, left side
#define TWO_CHOICES_RIGHT u"\x0E\x01\x0A\x00" //second choice, right side

#define CAPITAL u"\x0E\x01\x0B\x00" //presumably capitalizes following letter


//commands from "ReplaceTags" group (02)
#define PLAYER_NAME u"\x0E\x02\x00\x00"


//commands from "PictureFontTags" group (03)
#define IMAGE(image) u"\x0E\x03" image u"\x00" //input image as a hex string


//commands from "SoundTags" group (04)
#define SOUND(sound) u"\x0E\x04" sound u"\x00" //input image as a hex string
