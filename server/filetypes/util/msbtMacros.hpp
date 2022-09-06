#pragma once

#include <string>

using namespace std::literals::string_literals; //spooky namespace in header, not sure of workaround

//color commands from "System" group (00)
#define TEXT_SIZE(size)             u"\x0E\x00\x02\x02"s + static_cast<char16_t>(size)
#define TEXT_COLOR_DEFAULT          u"\x0E\x00\x03\x02\xFFFF\x0E\x00\x03\x02\x00"s //resets to color from TSY entry?
#define TEXT_COLOR_RED              u"\x0E\x00\x03\x02\x01"s
#define TEXT_COLOR_GREEN            u"\x0E\x00\x03\x02\x02"s
#define TEXT_COLOR_BLUE             u"\x0E\x00\x03\x02\x03"s
#define TEXT_COLOR_YELLOW           u"\x0E\x00\x03\x02\x04"s
#define TEXT_COLOR_CYAN             u"\x0E\x00\x03\x02\x05"s
#define TEXT_COLOR_MAGENTA          u"\x0E\x00\x03\x02\x06"s
#define TEXT_COLOR_GRAY             u"\x0E\x00\x03\x02\x07"s
#define TEXT_COLOR_ORANGE           u"\x0E\x00\x03\x02\x08"s

// For some reason the rock spire shop messages only function correctly when they end with this
#define TEXT_END                    u"\x0E\x00\x03\x02\xFFFF\x00"s

//commands from "ControlTags" group (01)
#define DRAW_INSTANT                u"\x0E\x01\x00\x00"s
#define DRAW_CHAR                   u"\x0E\x01\x01\x00"s
#define WAIT_DISMISS_PROMPT(frames) u"\x0E\x01\x02\x02"s + (char16_t)frames //input frames as int
#define WAIT_DISMISS(frames)        u"\x0E\x01\x03\x02"s + (char16_t)frames //input frames as int
#define DISMISS(frames)             u"\x0E\x01\x04\x02"s + (char16_t)frames //input frames as int
#define STEP(frames)                u"\x0E\x01\x05\x02"s + (char16_t)frames //input frames as int, exact purpose unknown
#define WAIT(frames)                u"\x0E\x01\x06\x02"s + (char16_t)frames //input frames as int
#define TWO_CHOICES                 u"\x0E\x01\x07\x00"s //two choices in a textbox
#define THREE_CHOICES               u"\x0E\x01\x08\x00"s //three choices in a textbox
#define TWO_CHOICES_LEFT            u"\x0E\x01\x09\x00"s //first choice, left side
#define TWO_CHOICES_RIGHT           u"\x0E\x01\x0A\x00"s //second choice, right side
#define CAPITAL                     u"\x0E\x01\x0B\x00"s //presumably capitalizes following letter
#define CREDIT_POSITION             u"\x0E\x01\x0C\x00"s //unknown effect
#define TWO_CHOICES_LEFT_B          u"\x0E\x01\x0D\x00"s //first choice, (bottom?) left side
#define TWO_CHOICES_RIGHT_B         u"\x0E\x01\x0E\x00"s //second choice, (bottom?) right side


//commands from "ReplaceTags" group (02)
#define REPLACE(replaceNum)         u"\x0E\x02"s + static_cast<char16_t>(replaceNum) + u"\x00"s


//commands from "PictureFontTags" group (03)
#define IMAGE(image)                u"\x0E\x03"s + static_cast<char16_t>(image) + u"\x00"s


//commands from "SoundTags" group (04)
#define SOUND(sound)                u"\x0E\x04"s + static_cast<char16_t>(sound) + u"\x00"s


//commands from "Camera" group (05)
#define CAMERA(camPos)              u"\x0E\x05"s + static_cast<char16_t>(camPos) + u"\x00"s


//commands from "Action" group (06)
#define ACTION(action)              u"\x0E\x05"s + static_cast<char16_t>(action) + u"\x00"s


enum struct ReplaceTags : uint16_t {
    PLAYER_NAME = 0,
    SEA_BATTLE_HIGH_SCORE,
    VASE_COMPENSATION_RUPEES,
    AUCTION_CHARACTER_NAME,
    AUCTION_ITEM_NAME,
    AUCTION_BID_RUPEES,
    AUCTION_INITIAL_BID,
    AUCTION_BID_SELECTOR, //for the player
    SWORD_GAME_HITS,
    PASSWORD,
    SORTED_LETTERS,
    SORTING_PAYMENT_RUPEES,
    LETTERS_ARRIVED,
    REMAINING_KOROKS,
    FOREST_WATER_TIMER,
    BIRDMAN_DISTANCE,
    BIRDMAN_HIGH_SCORE,
    BEEDLE_POINTS,
    MRS_MARIE_PENDANTS_GIVEN,
    MRS_MARIE_PENDANT_TOTAL,
    PIG_GAME_TIMER,
    BOATING_COURSE_RUPEE_TOTAL,
    BOMB_MAX,
    ARROW_MAX,
    LETTER_SORT_RECORD,
    FISHMAN_HIT_COUNT,
    FISHMAN_RUPEE_REWARD,
    BOKOBABA_SEED_COUNT,
    SKULL_NECKLACE_COUNT,
    CHU_JELLY_COUNT,
    JOY_PENDANT_COUNT,
    GOLDEN_FEATHER_COUNT,
    KNIGHTS_CREST_COUNT,
    BEEDLE_RUPEE_OFFER,
    BOKOBABA_SELL_ENTRY,
    SKULL_NECKLACE_SELL_ENTRY,
    CHU_JELLY_SELL_ENTRY,
    JOY_PENDANT_SELL_ENTRY,
    GOLDEN_FEATHER_SELL_ENTRY,
    KNIGHTS_CREST_SELL_ENTRY
    //SetValue 1-4, purpose unknown
};

enum struct ImageTags : uint16_t {
    A = 0,
    B,
    C, //seems unused
    L,
    R,
    X,
    Y,
    Z,
    L_STICK,
    L_ARROW,
    R_ARROW,
    U_ARROW,
    D_ARROW,
    L_STICK_UP,
    L_STICK_DOWN,
    L_STICK_LEFT,
    L_STICK_RIGHT,
    L_STICK_U_D,
    L_STICK_L_R,
    A_FLASH,
    STARBURST, //can hookshot/grapple indicator
    HEART,
    MUSIC_NOTE,
    PLUS,
    MINUS,
    ZL,
    ZR,
    R_STICK,
    R_STICK_UP,
    R_STICK_DOWN,
    R_STICK_LEFT,
    R_STICK_RIGHT,
    R_STICK_U_D,
    R_STICK_L_R,
    DPAD,
    DPAD_UP,
    DPAD_DOWN,
    DPAD_LEFT,
    DPAD_RIGHT,
    PAGE_TURN, //not sure where this is used
    SAIL,
    SWIFT_SAIL
};

//Sounds don't have unique names, no enum needed

enum struct CameraTags : uint16_t {
    BASIC_CONVERSATION = 0,
    CHARACTER_1,
    CHARACTER_2,
    CHARACTER_3,
    CHARACTER_4,
    CHARACTER_5,
    CHARACTER_6,
    CHARACTER_7,
    CHARACTER_8,
    CHARACTER_9,
    CHARACTER_10,
    LINK_SHOT,
    LONG,
    FIRST_PERSON,
    FIRST_PERSON_LINK,
    FIRST_PERSON_SPEAKER,
    LINK_ZOOM,
    SPEAKER_ZOOM,
    LINK_SIDE,
    SPEAKER_SIDE,
    LINK_FRONT,
    SPEAKER_FRONT,
    LINK_BUST,
    SPEAKER_BUST,
    LINK_ANGLE_FRONT,
    SPEAKER_ANGLE_FRONT,
    LINK_ANGLE_DOWN,
    SPEAKER_ANGLE_DOWN,
    SIDE,
    HUGE_FACE_ZOOM,
    SPEAKER_TO_LINK_2,
    LINK_TO_SPEAKER_2
};

enum struct ActionTags : uint16_t {
    LAUGH = 0,
    ANGER,
    CRY,
    SURPRISE,
    TROUBLED,
    BLANK, //called "-" in the files
    SNIFFLE,
    ARYLL_NOTICE,
    ARYLL_SMIRK,
    ARYLL_UNEASY
};
