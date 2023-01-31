.org @NextFreeSpace

.global init_save_with_tweaks
init_save_with_tweaks:
; Function start stuff
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

bl item_func_wind_tact ; Wind Waker
bl item_func_tact_song1 ; Wind's Requiem
bl progressive_sail_item_func ; Normal sail

lis r5, sword_mode@ha
addi r5, r5, sword_mode@l
lbz r5, 0 (r5)
cmpwi r5, 0 ; Start with Sword
beq start_with_sword
cmpwi r5, 2 ; Swordless
beq break_barrier_for_swordless
b after_sword_mode_initialization

start_with_sword:
bl item_func_sword
b after_sword_mode_initialization

break_barrier_for_swordless:
lis r3,gameInfo_ptr@ha
lwz r3,gameInfo_ptr@l(r3)
addi r3,r3, 0x644
li r4, 0x2C02 ; BARRIER_DOWN
bl onEventBit
li r4, 0x3B08 ; Another event flag set by the barrier. This one seems to have no effect, but set it anyway just to be safe.
bl onEventBit

after_sword_mode_initialization:

bl item_func_pirates_omamori ; Pirate's Charm

; Start the player with 30 bombs and arrows. (But not the ability to actually use them.)
; This change is so we can remove the code that sets your current bombs/arrows to 30 when you first get the bombs/bow.
; That code would be bad if the player got a bomb bag/quiver upgrade beforehand, as then that code would reduce the max.
lis r3,gameInfo_ptr@ha
lwz r3,gameInfo_ptr@l(r3)
addi r3,r3, 0x89
li r4, 30
stb r4, 0 (r3) ; Current arrows
stb r4, 1 (r3) ; Current bombs
stb r4, 6 (r3) ; Max arrows
stb r4, 7 (r3) ; Max bombs

; Set the player's magic meter size based on the value of a custom symbol.
lis r3,gameInfo_ptr@ha
lwz r3,gameInfo_ptr@l(r3)
addi r3,r3, 0x33
lis r4, starting_magic@ha
addi r4, r4, starting_magic@l
lbz r4, 0 (r4) ; Load starting magic address into r4, then load byte at address into r4
stb r4, 0 (r3) ; Max magic meter
stb r4, 1 (r3) ; Current magic meter

; Give user-selected custom starting items
bl init_starting_gear

lis r3,gameInfo_ptr@ha
lwz r3,gameInfo_ptr@l(r3)
addi r3,r3, 0x644
lis r4, 0x1
subi r4, r4, 0x4001
lis r5, outset_pig_color@ha
addi r5, r5, outset_pig_color@l
lbz r5, 0 (r5) ; Load pig color
bl setEventReg
li r4, 0x3510 ; HAS_SEEN_INTRO
bl onEventBit
li r4, 0x2A80 ; HAS_HEROS_CLOTHES (This should be set even if the player wants to wear casual clothes, it's overridden elsewhere)
bl onEventBit
li r4, 0x0280 ; SAW_TETRA_IN_FOREST_OF_FAIRIES
bl onEventBit
li r4, 0x0520 ; GOSSIP_STONE_AT_FF1 (Causes Aryll and the pirates to disappear from Outset)
bl onEventBit
li r4, 0x2E01 ; WATCHED_MEETING_KORL_CUTSCENE (Necessary for Windfall music to play when warping there)
bl onEventBit
li r4, 0x0F80 ; KORL_UNLOCKED_AND_SPAWN_ON_WINDFALL
bl onEventBit
li r4, 0x0908 ; SAIL_INTRODUCTION_TEXT_AND_MAP_UNLOCKED
bl onEventBit
li r4, 0x2A08 ; ENTER_KORL_FOR_THE_FIRST_TIME_AND_SPAWN_ANYWHERE
bl onEventBit
li r4, 0x0902 ; SAW_DRAGON_ROOST_ISLAND_INTRO
bl onEventBit
li r4, 0x1F40 ; SAW_QUILL_CUTSCENE_ON_DRI
bl onEventBit
li r4, 0x0A80 ; KORL_DINS_PEARL_TEXT_ALLOWING_YOU_TO_ENTER_HIM
bl onEventBit
li r4, 0x0901 ; TRIGGERED_MAP_FISH
bl onEventBit
li r4, 0x0A20 ; WATCHED_FOREST_HAVEN_INTRO_CUTSCENE
bl onEventBit
li r4, 0x1801 ; WATCHED_DEKU_TREE_CUTSCENE
bl onEventBit
li r4, 0x0A08 ; TALKED_TO_KORL_AFTER_LEAVING_FH
bl onEventBit
li r4, 0x0808 ; Needed so that exiting the pirate ship takes you to Windfall instead of the tutorial
bl onEventBit
li r4, 0x1F02 ; TALKED_TO_KORL_AFTER_GETTING_BOMBS
bl onEventBit
li r4, 0x2F20 ; Talked to KoRL after getting Nayru's Pearl
bl onEventBit
li r4, 0x3840 ; TALKED_TO_KORL_POST_TOWER_CUTSCENE
bl onEventBit
li r4, 0x2D04 ; MASTER_SWORD_CUTSCENE
bl onEventBit
li r4, 0x3802 ; COLORS_IN_HYRULE
bl onEventBit
li r4, 0x2D01 ; ANIMATION_SET_2 (Saw cutscene before Helmaroc King where Aryll is rescued)
bl onEventBit
li r4, 0x2D02 ; TETRA_TO_ZELDA_CUTSCENE
bl onEventBit
li r4, 0x3201 ; KoRL told you about the sages
bl onEventBit
li r4, 0x3380 ; KoRL told you about the Triforce shards
bl onEventBit
li r4, 0x1001 ; WATCHED_FIRE_AND_ICE_ARROWS_CUTSCENE
bl onEventBit
li r4, 0x2E04 ; MEDLI_IN_EARTH_TEMPLE_ENTRANCE
bl onEventBit
li r4, 0x2920 ; MEDLI_IN_EARTH_TEMPLE
bl onEventBit
li r4, 0x1620 ; Medli is in dungeon mode and can be lifted/called
bl onEventBit
li r4, 0x3304 ; Saw event where Medli calls to you from within jail
bl onEventBit
li r4, 0x2910 ; MAKAR_IN_WIND_TEMPLE
bl onEventBit
li r4, 0x1610 ; Makar is in dungeon mode and can be lifted/called
bl onEventBit
li r4, 0x3440 ; Saw event where Makar calls to you from within jail
bl onEventBit
li r4, 0x3A20 ; Fishman and KoRL talked about Forsaken Fortress after you beat Molgera
bl onEventBit
li r4, 0x2D08 ; HYRULE_3_WARP_CUTSCENE
bl onEventBit
li r4, 0x3980 ; HYRULE_3_ELECTRICAL_BARRIER_CUTSCENE_1
bl onEventBit
li r4, 0x3B02 ; Saw cutscene before Puppet Ganon fight
bl onEventBit
li r4, 0x4002 ; Saw cutscene before Ganondorf fight
bl onEventBit

lis r3,gameInfo_ptr@ha
lwz r3,gameInfo_ptr@l(r3)
addi r3,r3, 0x1178
li r4, 0x0310 ; Saw event where Grandma gives you the Hero's Clothes
bl onEventBit


; Set four switch bits (0, 1, 3, 7) for several events that happen in the Fairy Woods on Outset.
; Setting these switches causes the Tetra hanging from a tree and rescuing her from Bokoblins events to be marked as finished.
; Also set the switch (9) for having seen the event where you enter the Rito Aerie for the first time and get the Delivery Bag.
; Also set the switch (8) for having unclogged the pond, since that boulder doesn't respond to normal bombs which would be odd.
; Also set the the switch (1E) for having seen the intro to the interior of the Forest Haven, where the camera pans up.
; Also set the the switch (13) for having seen the camera panning towards the treasure chest in Windfall Town Jail.
lis r3,gameInfo_ptr@ha
lwz r3,gameInfo_ptr@l(r3)
addi r3,r3, 0x52C
lis r4, 0x4008
addi r4, r4, 0x038B
stw r4, 4 (r3)

; Set two switch bits (3E and 3F) for having unlocked the song tablets in the Earth and Wind Temple entrances.
lis r4, 0xC000
stw r4, 8 (r3)

; Set a switch bit (19) for the event on Greatfish Isle so that the endless night never starts.
lis r3,gameInfo_ptr@ha
lwz r3,gameInfo_ptr@l(r3)
addi r3,r3, 0x3a0
lis r4, 0x0200
stw r4, 4 (r3)
; Also set a switch bit (3F) for having seen the Windfall Island intro scene.
lis r4, 0x8000
stw r4, 8 (r3)
; Also set a switch bit (58) for having seen the short event when you enter Forsaken Fortress 2 for the first time.
; Also set a switch (0x50) for having seen the event where the camera pans around the Flight Control Platform.
; Also set a switch (0x47) for having hit the switch at Forest Haven to open up the hatch to the Nintendo Gallery.
; Also set a switch (0x5E) for the ladder leading up to the Nintendo Gallery being lowered.
lis r4, 0x4101
addi r4, r4, 0x0080
stw r4, 0xC (r3)

; Set a switch (21) for having seen the gossip stone event in DRC where KoRL tells you about giving bait to rats.
; Also set a switch (09) for having seen the event where the camera pans up to Valoo when you go outside.
; Also set a switch (46) for having seen the event where the camera pans around when you first enter DRC.
lis r3,gameInfo_ptr@ha
lwz r3,gameInfo_ptr@l(r3)
addi r3,r3, 0x40c
li r4, 0x0200
stw r4, 4 (r3)
li r4, 0x0002
stw r4, 8 (r3)
li r4, 0x0040
stw r4, 0xC (r3)

; Set a switch (36) for having seen the event where the camera pans around the first room when you first enter FW.
lis r3,gameInfo_ptr@ha
lwz r3,gameInfo_ptr@l(r3)
addi r3,r3, 0x430
lis r4, 0x0040
stw r4, 8 (r3)

; Set a switch (2D) for having seen the event where the camera pans around when you go outside at the top of TotG.
; Also set a switch (63) for having seen the event where the camera pans around the first room when you first enter TotG.
lis r3,gameInfo_ptr@ha
lwz r3,gameInfo_ptr@l(r3)
addi r3,r3, 0x454
li r4, 0x2000
stw r4, 8 (r3)
li r4, 0x0008
stw r4, 0x10 (r3)

; Set a switch (2A) for having seen the gossip stone event where KoRL tells you Medli shows up on the compass.
lis r3,gameInfo_ptr@ha
lwz r3,gameInfo_ptr@l(r3)
addi r3,r3, 0x478
li r4, 0x0400
stw r4, 8 (r3)

; Set a switch (12) for having seen the camera moving around event when you first enter Hyrule.
; Also set a switch (6) for having completed the Triforce pushable blocks puzzle.
lis r3,gameInfo_ptr@ha
lwz r3,gameInfo_ptr@l(r3)
addi r3,r3, 0x4e4
lis r4, 0x0004
addi r4, r4, 0x0040
stw r4, 4 (r3)

; Set a switch (0D) for having seen the camera panning around when you first enter Ganon's Tower.
; Also set a switch (1C) for having seen the camera panning around looking at the 4 lights in the room where you can drop down to the maze.
; Also set a switch (1D) for having seen the camera panning around looking at the 4 boomerang switches in the room with the warp up to Forsaken Fortress.
; Also set a switch (1E) for having seen the cutscene before the Puppet Ganon fight.
; Also set a switch (12) for having seen the cutscene after the Puppet Ganon fight.
; Also set a switch (1F) for having seen the cutscene before the Ganondorf fight.
lis r3,gameInfo_ptr@ha
lwz r3,gameInfo_ptr@l(r3)
addi r3,r3, 0x4c0
lis r4, 0xF004
addi r4, r4, 0x2000
stw r4, 4 (r3)


li r3, 3 ; DRC stage ID
li r4, 5 ; Seen the boss intro bit index
bl generic_on_dungeon_bit
li r3, 4 ; FW stage ID
li r4, 5 ; Seen the boss intro bit index
bl generic_on_dungeon_bit
li r3, 5 ; TotG stage ID
li r4, 5 ; Seen the boss intro bit index
bl generic_on_dungeon_bit
li r3, 6 ; ET stage ID
li r4, 5 ; Seen the boss intro bit index
bl generic_on_dungeon_bit
li r3, 7 ; WT stage ID
li r4, 5 ; Seen the boss intro bit index
bl generic_on_dungeon_bit


; Make the game think the player has previously owned every type of spoil and bait so they don't get the item get animation the first time they pick each type up.
lis r3,gameInfo_ptr@ha
lwz r3,gameInfo_ptr@l(r3)
addi r3,r3, 0xB4
li r4, 0xFF
stb r4, 0 (r3) ; 803C4C9C, bitfield of what spoils bag items you've ever owned
stb r4, 1 (r3) ; 803C4C9D, bitfield of what bait bag items you've ever owned

; Give the player the number of Triforce Shards they want to start with.
lis r5, num_triforce_shards_to_start_with@ha
addi r5, r5, num_triforce_shards_to_start_with@l
lbz r5, 0 (r5) ; Load number of Triforce Shards to start with
lis r3,gameInfo_ptr@ha
lwz r3,gameInfo_ptr@l(r3)
addi r3,r3, 0xDE
; Convert the number of shards to a bitfield with that many bits set.
; e.g. For 5 shards, ((1 << 5) - 1) results in 0x1F (binary 00011111).
li r0, 1
slw r4, r0, r5
subi r4, r4, 1
stb r4, 0 (r3) ; Store the bitfield of shards back
; If the number of starting shards is 8, also set the event flag for seeing the Triforce refuse together.
cmpwi r5, 8
blt after_starting_triforce_shards
lis r3,gameInfo_ptr@ha
lwz r3,gameInfo_ptr@l(r3)
addi r3,r3, 0x644
li r4, 0x3D04 ; Saw the Triforce refuse
bl onEventBit
after_starting_triforce_shards:


lis r5, skip_rematch_bosses@ha
addi r5, r5, skip_rematch_bosses@l
lbz r5, 0 (r5) ; Load bool of whether rematch bosses should be skipped
cmpwi r5, 1
bne after_skipping_rematch_bosses
lis r3,gameInfo_ptr@ha
lwz r3,gameInfo_ptr@l(r3)
addi r3,r3, 0x644
li r4, 0x3904 ; Recollection Gohma defeated
bl onEventBit
li r4, 0x3902 ; Recollection Kalle Demos defeated
bl onEventBit
li r4, 0x3901 ; Recollection Jalhalla defeated
bl onEventBit
li r4, 0x3A80 ; Recollection Molgera defeated
bl onEventBit
after_skipping_rematch_bosses:

lis r12, gameInfo_ptr@ha ; replace the line we overwrote to jump here

; Function end stuff
lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


; This function reads from an array of user-selected starting item IDs and adds them to your inventory.
.global init_starting_gear
init_starting_gear:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)
stw r31, 0xC (sp)

lis r31, starting_gear@ha
addi r31, r31, starting_gear@l
lbz r3, 0 (r31)
b init_starting_gear_check_continue_loop

init_starting_gear_begin_loop:
b starting_gear_check_custom_item
init_starting_gear_regular_items:
bl convert_progressive_item_id
bl execItemGet
init_starting_gear_next_item:
lbzu r3, 1(r31)
init_starting_gear_check_continue_loop:
cmplwi r3, 255
bne+ init_starting_gear_begin_loop

end_init_starting_gear:
lwz r31, 0xC (sp)
lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global starting_gear_check_custom_item
starting_gear_check_custom_item:
cmpwi r3, 0x13
beq starting_gear_add_drc_small_key
cmpwi r3, 0x14
beq starting_gear_add_drc_big_key
cmpwi r3, 0x1B
beq starting_gear_add_drc_dungeon_map
cmpwi r3, 0x1C
beq starting_gear_add_drc_compass
cmpwi r3, 0x1D
beq starting_gear_add_fw_small_key
cmpwi r3, 0x1F
beq starting_gear_add_joy_pendant
cmpwi r3, 0x40
beq starting_gear_add_fw_big_key
cmpwi r3, 0x41
beq starting_gear_add_fw_dungeon_map
cmpwi r3, 0x45
beq starting_gear_add_skull_necklace
cmpwi r3, 0x46
beq starting_gear_add_boko_baba_seed
cmpwi r3, 0x47
beq starting_gear_add_golden_feather
cmpwi r3, 0x48
beq starting_gear_add_knights_crest
cmpwi r3, 0x49
beq starting_gear_add_red_chu_jelly
cmpwi r3, 0x4A
beq starting_gear_add_green_chu_jelly
cmpwi r3, 0x4B
beq starting_gear_add_blue_chu_jelly
cmpwi r3, 0x5A
beq starting_gear_add_fw_compass
cmpwi r3, 0x5B
beq starting_gear_add_totg_small_key
cmpwi r3, 0x5C
beq starting_gear_add_totg_big_key
cmpwi r3, 0x5D
beq starting_gear_add_totg_dungeon_map
cmpwi r3, 0x5E
beq starting_gear_add_totg_compass
cmpwi r3, 0x5F
beq starting_gear_add_ff_dungeon_map
cmpwi r3, 0x60
beq starting_gear_add_ff_compass
cmpwi r3, 0x73
beq starting_gear_add_et_small_key
cmpwi r3, 0x74
beq starting_gear_add_et_big_key
cmpwi r3, 0x75
beq starting_gear_add_et_dungeon_map
cmpwi r3, 0x76
beq starting_gear_add_et_compass
cmpwi r3, 0x81
beq starting_gear_add_wt_small_key
cmpwi r3, 0x84
beq starting_gear_add_wt_big_key
cmpwi r3, 0x85
beq starting_gear_add_wt_dungeon_map
cmpwi r3, 0x86
beq starting_gear_add_wt_compass
cmpwi r3, 0xA3
beq starting_gear_add_dragon_tingle_statue
cmpwi r3, 0xA4
beq starting_gear_add_forbidden_tingle_statue
cmpwi r3, 0xA5
beq starting_gear_add_goddess_tingle_statue
cmpwi r3, 0xA6
beq starting_gear_add_earth_tingle_statue
cmpwi r3, 0xA7
beq starting_gear_add_wind_tingle_statue
b init_starting_gear_regular_items

.global starting_gear_add_drc_small_key
starting_gear_add_drc_small_key:
bl drc_small_key_item_get_func
b init_starting_gear_next_item

.global starting_gear_add_drc_big_key
starting_gear_add_drc_big_key:
bl drc_big_key_item_get_func
b init_starting_gear_next_item

.global starting_gear_add_drc_dungeon_map
starting_gear_add_drc_dungeon_map:
bl drc_dungeon_map_item_get_func
b init_starting_gear_next_item

.global starting_gear_add_drc_compass
starting_gear_add_drc_compass:
bl drc_compass_item_get_func
b init_starting_gear_next_item

.global starting_gear_add_fw_small_key
starting_gear_add_fw_small_key:
bl fw_small_key_item_get_func
b init_starting_gear_next_item

.global starting_gear_add_fw_big_key
starting_gear_add_fw_big_key:
bl fw_big_key_item_get_func
b init_starting_gear_next_item

.global starting_gear_add_fw_dungeon_map
starting_gear_add_fw_dungeon_map:
bl fw_dungeon_map_item_get_func
b init_starting_gear_next_item

.global starting_gear_add_fw_compass
starting_gear_add_fw_compass:
bl fw_compass_item_get_func
b init_starting_gear_next_item

.global starting_gear_add_totg_small_key
starting_gear_add_totg_small_key:
bl totg_small_key_item_get_func
b init_starting_gear_next_item

.global starting_gear_add_totg_big_key
starting_gear_add_totg_big_key:
bl totg_big_key_item_get_func
b init_starting_gear_next_item

.global starting_gear_add_totg_dungeon_map
starting_gear_add_totg_dungeon_map:
bl totg_dungeon_map_item_get_func
b init_starting_gear_next_item

.global starting_gear_add_totg_compass
starting_gear_add_totg_compass:
bl totg_compass_item_get_func
b init_starting_gear_next_item

.global starting_gear_add_ff_dungeon_map
starting_gear_add_ff_dungeon_map:
bl ff_dungeon_map_item_get_func
b init_starting_gear_next_item

.global starting_gear_add_ff_compass
starting_gear_add_ff_compass:
bl ff_compass_item_get_func
b init_starting_gear_next_item

.global starting_gear_add_et_small_key
starting_gear_add_et_small_key:
bl et_small_key_item_get_func
b init_starting_gear_next_item

.global starting_gear_add_et_big_key
starting_gear_add_et_big_key:
bl et_big_key_item_get_func
b init_starting_gear_next_item

.global starting_gear_add_et_dungeon_map
starting_gear_add_et_dungeon_map:
bl et_dungeon_map_item_get_func
b init_starting_gear_next_item

.global starting_gear_add_et_compass
starting_gear_add_et_compass:
bl et_compass_item_get_func
b init_starting_gear_next_item

.global starting_gear_add_wt_small_key
starting_gear_add_wt_small_key:
bl wt_small_key_item_get_func
b init_starting_gear_next_item

.global starting_gear_add_wt_big_key
starting_gear_add_wt_big_key:
bl wt_big_key_item_get_func
b init_starting_gear_next_item

.global starting_gear_add_wt_dungeon_map
starting_gear_add_wt_dungeon_map:
bl wt_dungeon_map_item_get_func
b init_starting_gear_next_item

.global starting_gear_add_wt_compass
starting_gear_add_wt_compass:
bl wt_compass_item_get_func
b init_starting_gear_next_item

.global starting_gear_add_dragon_tingle_statue
starting_gear_add_dragon_tingle_statue:
bl dragon_tingle_statue_item_get_func
b init_starting_gear_next_item

.global starting_gear_add_forbidden_tingle_statue
starting_gear_add_forbidden_tingle_statue:
bl forbidden_tingle_statue_item_get_func
b init_starting_gear_next_item

.global starting_gear_add_goddess_tingle_statue
starting_gear_add_goddess_tingle_statue:
bl goddess_tingle_statue_item_get_func
b init_starting_gear_next_item

.global starting_gear_add_earth_tingle_statue
starting_gear_add_earth_tingle_statue:
bl earth_tingle_statue_item_get_func
b init_starting_gear_next_item

.global starting_gear_add_wind_tingle_statue
starting_gear_add_wind_tingle_statue:
bl wind_tingle_statue_item_get_func
b init_starting_gear_next_item

.global starting_gear_add_joy_pendant ; Max amount of 40
starting_gear_add_joy_pendant:
bl item_func_joy_pendant
b init_starting_gear_next_item

.global starting_gear_add_skull_necklace ; Max amount of 23
starting_gear_add_skull_necklace:
bl item_func_skull_necklace
b init_starting_gear_next_item

.global starting_gear_add_boko_baba_seed ; Max amount of 10
starting_gear_add_boko_baba_seed:
bl item_func_boko_baba_seed
b init_starting_gear_next_item

.global starting_gear_add_golden_feather ; Max amount of 20
starting_gear_add_golden_feather:
bl item_func_golden_feather
b init_starting_gear_next_item

.global starting_gear_add_knights_crest ; Max amount of 10
starting_gear_add_knights_crest:
bl item_func_knights_crest
b init_starting_gear_next_item

.global starting_gear_add_red_chu_jelly ; Max amount of 15
starting_gear_add_red_chu_jelly:
bl item_func_red_chu_jelly
b init_starting_gear_next_item

.global starting_gear_add_green_chu_jelly ; Max amount of 15
starting_gear_add_green_chu_jelly:
bl item_func_green_chu_jelly
b init_starting_gear_next_item

.global starting_gear_add_blue_chu_jelly ; Max amount of 15
starting_gear_add_blue_chu_jelly:
bl item_func_blue_chu_jelly
b init_starting_gear_next_item

.global num_triforce_shards_to_start_with
num_triforce_shards_to_start_with:
.byte 0 ; By default start with no Triforce Shards
.global should_start_with_heros_clothes
should_start_with_heros_clothes:
.byte 1 ; By default start with the Hero's Clothes
.global sword_mode
sword_mode:
.byte 0 ; By default Start with Sword
.global skip_rematch_bosses
skip_rematch_bosses:
.byte 1 ; By default skip them

.global starting_gear
starting_gear:
.space 256, 0xFF ; Allocate space for up to 256 additional items. This is enough space for the max allowed spoil counts and regular starting items (when changing this also update the constant in options.hpp)
.byte 0xFF
.align 1 ; Align to the next 2 bytes

.global starting_quarter_hearts
starting_quarter_hearts:
.short 12 ; By default start with 12 quarter hearts (3 heart containers)

.global starting_magic
starting_magic:
.byte 16 ; By default start with 16 units of magic (small magic meter)

.global outset_pig_color
outset_pig_color:
.byte 0

.align 2 ; Align to the next 4 bytes



.global isEventBit_wrapper
isEventBit_wrapper:
	stwu sp, -0x10(sp)
	mflr r0
	stw r0, 0x14(sp)
	mr r4, r3
	lis r3,gameInfo_ptr@ha
	lwz r3,gameInfo_ptr@l(r3)
	addi r3,r3,0x644
	bl isEventBit
	lwz r0, 0x14 (sp)
	mtlr r0
	addi sp, sp, 0x10
	blr

.global convert_progressive_item_id
convert_progressive_item_id:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

cmpwi r3, 0x38
beq convert_progressive_sword_id
cmpwi r3, 0x39
beq convert_progressive_sword_id
cmpwi r3, 0x3A
beq convert_progressive_sword_id
cmpwi r3, 0x3D
beq convert_progressive_sword_id
cmpwi r3, 0x3E
beq convert_progressive_sword_id

cmpwi r3, 0x3B
beq convert_progressive_shield_id
cmpwi r3, 0x3C
beq convert_progressive_shield_id

cmpwi r3, 0x27
beq convert_progressive_bow_id
cmpwi r3, 0x35
beq convert_progressive_bow_id
cmpwi r3, 0x36
beq convert_progressive_bow_id

cmpwi r3, 0xAB
beq convert_progressive_wallet_id
cmpwi r3, 0xAC
beq convert_progressive_wallet_id

cmpwi r3, 0xAD
beq convert_progressive_bomb_bag_id
cmpwi r3, 0xAE
beq convert_progressive_bomb_bag_id

cmpwi r3, 0xAF
beq convert_progressive_quiver_id
cmpwi r3, 0xB0
beq convert_progressive_quiver_id

cmpwi r3, 0x23
beq convert_progressive_picto_box_id
cmpwi r3, 0x26
beq convert_progressive_picto_box_id

cmpwi r3, 0xB1
beq convert_progressive_magic_meter_id
cmpwi r3, 0xB2
beq convert_progressive_magic_meter_id

cmpwi r3, 0x78
beq convert_progressive_sail_id
cmpwi r3, 0x77
beq convert_progressive_sail_id

b convert_progressive_item_id_func_end


convert_progressive_sword_id:
lis r3,gameInfo_ptr@ha
lwz r3,gameInfo_ptr@l(r3)
addi r3,r3,0xd4
lbz r4, 0 (r3) ; Bitfield of swords you own
cmpwi r4, 0
beq convert_progressive_sword_id_to_normal_sword
cmpwi r4, 1
beq convert_progressive_sword_id_to_powerless_master_sword
cmpwi r4, 3
beq convert_progressive_sword_id_to_half_power_master_sword
cmpwi r4, 7
beq convert_progressive_sword_id_to_full_power_master_sword
li r3, 0x38
b convert_progressive_item_id_func_end

convert_progressive_sword_id_to_normal_sword:
li r3, 0x38
b convert_progressive_item_id_func_end
convert_progressive_sword_id_to_powerless_master_sword:
li r3, 0x39
b convert_progressive_item_id_func_end
convert_progressive_sword_id_to_half_power_master_sword:
li r3, 0x3A
b convert_progressive_item_id_func_end
convert_progressive_sword_id_to_full_power_master_sword:
li r3, 0x3E
b convert_progressive_item_id_func_end


convert_progressive_shield_id:
lis r3, gameInfo_ptr@ha
lwz r3, gameInfo_ptr@l(r3)
addi r3,r3,0xd5 ;Check if this is right location for shield bitfield
lbz r4, 0 (r3)
cmpwi r4, 0
beq convert_progressive_shield_id_to_heros_shield
cmpwi r4, 1
beq convert_progressive_shield_id_to_mirror_shield
li r3, 0x3B
b convert_progressive_item_id_func_end

convert_progressive_shield_id_to_heros_shield:
li r3, 0x3B
b convert_progressive_item_id_func_end
convert_progressive_shield_id_to_mirror_shield:
li r3, 0x3C
b convert_progressive_item_id_func_end


convert_progressive_bow_id:
lis r3, gameInfo_ptr@ha
lwz r3, gameInfo_ptr@l(r3)
addi r3,r3,0x7d
lbz r4, 0 (r3) ; Bitfield of arrow types you own
cmpwi r4, 0
beq convert_progressive_bow_id_to_heros_bow
cmpwi r4, 1
beq convert_progressive_bow_id_to_fire_and_ice_arrows
cmpwi r4, 3
beq convert_progressive_bow_id_to_light_arrows
li r3, 0x27 ; Invalid bow state; this shouldn't happen so just return the base bow ID
b convert_progressive_item_id_func_end

convert_progressive_bow_id_to_heros_bow:
li r3, 0x27
b convert_progressive_item_id_func_end
convert_progressive_bow_id_to_fire_and_ice_arrows:
li r3, 0x35
b convert_progressive_item_id_func_end
convert_progressive_bow_id_to_light_arrows:
li r3, 0x36
b convert_progressive_item_id_func_end


convert_progressive_wallet_id:
lis r3, gameInfo_ptr@ha
lwz r3, gameInfo_ptr@l(r3)
addi r3,r3,0x32
lbz r4, 0 (r3) ; Which wallet you have
cmpwi r4, 0
beq convert_progressive_wallet_id_to_1000_rupee_wallet
cmpwi r4, 1
beq convert_progressive_wallet_id_to_5000_rupee_wallet
li r3, 0xAB ; Invalid wallet state; this shouldn't happen so just return the base wallet ID
b convert_progressive_item_id_func_end

convert_progressive_wallet_id_to_1000_rupee_wallet:
li r3, 0xAB
b convert_progressive_item_id_func_end
convert_progressive_wallet_id_to_5000_rupee_wallet:
li r3, 0xAC
b convert_progressive_item_id_func_end


convert_progressive_bomb_bag_id:
lis r3, gameInfo_ptr@ha
lwz r3, gameInfo_ptr@l(r3)
addi r3,r3,0x8a
lbz r4, 6 (r3) ; Max number of bombs the player can currently hold
cmpwi r4, 30
beq convert_progressive_bomb_bag_id_to_60_bomb_bomb_bag
cmpwi r4, 60
beq convert_progressive_bomb_bag_id_to_99_bomb_bomb_bag
li r3, 0xAD ; Invalid bomb bag state; this shouldn't happen so just return the base bomb bag ID
b convert_progressive_item_id_func_end

convert_progressive_bomb_bag_id_to_60_bomb_bomb_bag:
li r3, 0xAD
b convert_progressive_item_id_func_end
convert_progressive_bomb_bag_id_to_99_bomb_bomb_bag:
li r3, 0xAE
b convert_progressive_item_id_func_end


convert_progressive_quiver_id:
lis r3, gameInfo_ptr@ha
lwz r3, gameInfo_ptr@l(r3)
addi r3,r3,0x89
lbz r4, 6 (r3) ; Max number of arrows the player can currently hold
cmpwi r4, 30
beq convert_progressive_quiver_id_to_60_arrow_quiver
cmpwi r4, 60
beq convert_progressive_quiver_id_to_99_arrow_quiver
li r3, 0xAF ; Invalid quiver state; this shouldn't happen so just return the base quiver ID
b convert_progressive_item_id_func_end

convert_progressive_quiver_id_to_60_arrow_quiver:
li r3, 0xAF
b convert_progressive_item_id_func_end
convert_progressive_quiver_id_to_99_arrow_quiver:
li r3, 0xB0
b convert_progressive_item_id_func_end


convert_progressive_picto_box_id:
lis r3, gameInfo_ptr@ha
lwz r3, gameInfo_ptr@l(r3)
addi r3,r3,0x79
lbz r4, 0 (r3) ; Bitfield of picto boxes you own
cmpwi r4, 0
beq convert_progressive_picto_box_id_to_normal_picto_box
cmpwi r4, 1
beq convert_progressive_picto_box_id_to_deluxe_picto_box
li r3, 0x23 ; Invalid picto box state; this shouldn't happen so just return the base picto box ID
b convert_progressive_item_id_func_end

convert_progressive_picto_box_id_to_normal_picto_box:
li r3, 0x23
b convert_progressive_item_id_func_end
convert_progressive_picto_box_id_to_deluxe_picto_box:
li r3, 0x26
b convert_progressive_item_id_func_end

convert_progressive_sail_id:
lis r3, gameInfo_ptr@ha
lwz r3, gameInfo_ptr@l(r3)
addi r3,r3,0x5D
lbz r4, 0 (r3) ; equipped sail ID
cmpwi r4, 0xFF ; no sail
beq convert_progressive_sail_id_to_normal_sail
cmpwi r4, 0x78 ; normal sail
beq convert_progressive_sail_id_to_swift_sail
li r3, 0x78 ; Invalid sail state; this shouldn't happen so just return the base sail ID
b convert_progressive_item_id_func_end

convert_progressive_sail_id_to_normal_sail:
li r3, 0x78
b convert_progressive_item_id_func_end
convert_progressive_sail_id_to_swift_sail:
li r3, 0x77
b convert_progressive_item_id_func_end


convert_progressive_magic_meter_id:
lis r3,gameInfo_ptr@ha
lwz r3,gameInfo_ptr@l(r3)
addi r3,r3, 0x33
lbz r4, 0 (r3) ; Max magic meter
cmpwi r4, 0
beq convert_progressive_magic_meter_id_to_normal_magic_meter
cmpwi r4, 16
beq convert_progressive_magic_meter_id_to_magic_meter_upgrade
li r3, 0xB1 ; Invalid magic meter state; this shouldn't happen so just return the base magic meter ID
b convert_progressive_item_id_func_end

convert_progressive_magic_meter_id_to_normal_magic_meter:
li r3, 0xB1
b convert_progressive_item_id_func_end
convert_progressive_magic_meter_id_to_magic_meter_upgrade:
li r3, 0xB2
b convert_progressive_item_id_func_end

convert_progressive_item_id_func_end:
lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global progressive_sword_item_func
progressive_sword_item_func:
; Function start stuff
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

lis r3, gameInfo_ptr@ha
lwz r3, gameInfo_ptr@l(r3)
addi r3,r3,0xd4
lbz r4, 0 (r3) ; Bitfield of swords you own
cmpwi r4, 0
beq get_normal_sword
cmpwi r4, 1
beq get_powerless_master_sword
cmpwi r4, 3
beq get_half_power_master_sword
cmpwi r4, 7
beq get_full_power_master_sword
b sword_func_end

get_normal_sword:
bl item_func_sword
b sword_func_end

get_powerless_master_sword:
bl item_func_master_sword
b sword_func_end

get_half_power_master_sword:
bl item_func_lv3_sword
b sword_func_end

get_full_power_master_sword:
bl item_func_master_sword_ex

sword_func_end:
; Function end stuff
lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global progressive_shield_item_func
progressive_shield_item_func:
; Function start stuff
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

lis r3, gameInfo_ptr@ha
lwz r3, gameInfo_ptr@l(r3)
addi r3,r3,0xd5 ;Check if this is right location for shield bitfield
lbz r4, 0 (r3) ; Bitfield of shields you own
cmpwi r4, 0
beq get_heros_shield
cmpwi r4, 1
beq get_mirror_shield
b shield_func_end

get_heros_shield:
bl item_func_shield
b shield_func_end

get_mirror_shield:
bl item_func_mirror_shield

shield_func_end:
; Function end stuff
lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global progressive_bow_item_func
progressive_bow_item_func:
; Function start stuff
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

lis r3, gameInfo_ptr@ha
lwz r3, gameInfo_ptr@l(r3)
addi r3,r3,0x7d
lbz r4, 0 (r3) ; Bitfield of arrow types you own
cmpwi r4, 0
beq get_heros_bow
cmpwi r4, 1
beq get_fire_and_ice_arrows
cmpwi r4, 3
beq get_light_arrows
b bow_func_end

get_heros_bow:
bl item_func_bow
b bow_func_end

get_fire_and_ice_arrows:
bl item_func_magic_arrow
b bow_func_end

get_light_arrows:
bl item_func_light_arrow

bow_func_end:
; Function end stuff
lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global progressive_wallet_item_func
progressive_wallet_item_func:

lis r3, gameInfo_ptr@ha
lwz r3, gameInfo_ptr@l(r3)
addi r3,r3,0x32
lbz r4, 0 (r3) ; Which wallet you have
cmpwi r4, 0
beq get_1000_rupee_wallet
cmpwi r4, 1
beq get_5000_rupee_wallet
b wallet_func_end

get_1000_rupee_wallet:
li r4, 1
stb r4, 0 (r3) ; Which wallet you have
b wallet_func_end

get_5000_rupee_wallet:
li r4, 2
stb r4, 0 (r3) ; Which wallet you have

wallet_func_end:
blr

.global progressive_bomb_bag_item_func
progressive_bomb_bag_item_func:

lis r3, gameInfo_ptr@ha
lwz r3, gameInfo_ptr@l(r3)
addi r3,r3,0x8a
lbz r4, 6 (r3) ; Max number of bombs the player can currently hold
cmpwi r4, 30
beq get_60_bomb_bomb_bag
cmpwi r4, 60
beq get_99_bomb_bomb_bag
b bomb_bag_func_end

get_60_bomb_bomb_bag:
li r4, 60
stb r4, 0 (r3) ; Current num bombs
stb r4, 6 (r3) ; Max num bombs
b bomb_bag_func_end

get_99_bomb_bomb_bag:
li r4, 99
stb r4, 0 (r3) ; Current num bombs
stb r4, 6 (r3) ; Max num bombs

bomb_bag_func_end:
blr

.global progressive_quiver_item_func
progressive_quiver_item_func:

lis r3, gameInfo_ptr@ha
lwz r3, gameInfo_ptr@l(r3)
addi r3,r3,0x89
lbz r4, 6 (r3) ; Max number of arrows the player can currently hold
cmpwi r4, 30
beq get_60_arrow_quiver
cmpwi r4, 60
beq get_99_arrow_quiver
b quiver_func_end

get_60_arrow_quiver:
li r4, 60
stb r4, 0 (r3) ; Current num arrows
stb r4, 6 (r3) ; Max num arrows
b quiver_func_end

get_99_arrow_quiver:
li r4, 99
stb r4, 0 (r3) ; Current num arrows
stb r4, 6 (r3) ; Max num arrows

quiver_func_end:
blr

.global progressive_picto_box_item_func
progressive_picto_box_item_func:
; Function start stuff
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

lis r3, gameInfo_ptr@ha
lwz r3, gameInfo_ptr@l(r3)
addi r3,r3,0x79
lbz r4, 0 (r3) ; Bitfield of picto boxes you own
cmpwi r4, 0
beq get_normal_picto_box
cmpwi r4, 1
beq get_deluxe_picto_box
b picto_box_func_end

get_normal_picto_box:
bl item_func_camera
b picto_box_func_end

get_deluxe_picto_box:
bl item_func_camera2

picto_box_func_end:
; Function end stuff
lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global progressive_sail_item_func
progressive_sail_item_func:
; Function start stuff
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

lis r3, gameInfo_ptr@ha
lwz r3, gameInfo_ptr@l(r3)
addi r3,r3,0x5D
lbz r4, 0 (r3) ; Equipped sail
cmpwi r4, 0xFF ; no sail
beq get_normal_sail
cmpwi r4, 0x78 ; normal sail
beq get_swift_sail
b sail_func_end

get_normal_sail:
bl item_func_normal_sail
b sail_func_end

get_swift_sail:
bl item_func_swift_sail

sail_func_end:
; Function end stuff
lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global progressive_magic_meter_item_func
progressive_magic_meter_item_func:
; Function start stuff
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)


lis r3,gameInfo_ptr@ha
lwz r3,gameInfo_ptr@l(r3)
addi r3,r3, 0x33
lbz r4, 0 (r3) ; Max magic meter
cmpwi r4, 0
beq progressive_magic_meter_item_func_get_normal_magic_meter
cmpwi r4, 16
beq progressive_magic_meter_item_func_get_magic_meter_upgrade
b progressive_magic_meter_item_func_end

progressive_magic_meter_item_func_get_normal_magic_meter:
bl normal_magic_meter_item_func
b progressive_magic_meter_item_func_end

progressive_magic_meter_item_func_get_magic_meter_upgrade:
bl item_func_max_mp_up1


progressive_magic_meter_item_func_end:
; Function end stuff
lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr




; Write our own base magic meter item get function because the vanilla one (item_func_magic_power__Fv) was just a placeholder.
.global normal_magic_meter_item_func
normal_magic_meter_item_func:
; Function start stuff
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

bl FUN_025200d4
li r4, 16
sth r4, 0x5b60 (r3) ; MP to gradually add to the current meter
sth r4, 0x5b64 (r3) ; MP to gradually add to the max meter

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr




.global hurricane_spin_item_func
hurricane_spin_item_func:
; Function start stuff
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

lis r3, gameInfo_ptr@ha
lwz r3, gameInfo_ptr@l(r3)
li r4,0x6901
addi r3,r3,0x644
bl onEventBit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global custom_createItem
custom_createItem:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

; If the item ID is FF, no item will be spawned.
; In order to avoid issues we need to return -1.
; Also don't bother even trying to spawn the item - it wouldn't do anything.
cmpwi r4, 0xFF
beq custom_createItem_invalid_item_id

; Create the item by calling createItem, which will load the item's model if necessary.
mr r9, r5
mr r5, r8
mr r8, r6
mr r6, r9
mr r10, r7
li r7, 3 ; Don't fade out
li r9, 5 ; Item action, how it behaves. 5 causes it to make a ding sound so that it's more obvious.
bl fopAcM_createItem

; Return the actor's unique ID that createItem returned.
b custom_createItem_func_end

custom_createItem_invalid_item_id:
li r3, -1 ; Return -1 to indicate no actor was created.

custom_createItem_func_end:
lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global generic_on_dungeon_bit
generic_on_dungeon_bit:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

mr r5, r3 ; Argument r3 to this func is the stage ID of the dungeon to add this item for
mr r6, r4 ; Argument r4 to this func is the bit index to set

lis r3, gameInfo_ptr@ha
lwz r3, gameInfo_ptr@l(r3)
addi r3,r3,0x7bc ; stage ID of the current stage
lbz r4, 0 (r3)
cmpw r4, r5 ; Check if we're currently in the right dungeon for this key
beq generic_on_dungeon_bit_in_correct_dungeon

lis r3, gameInfo_ptr@ha
lwz r3, gameInfo_ptr@l(r3)
addi r3,r3,0x3a0
mulli r4, r5, 0x24 ; Use stage ID of the dungeon as the index, each entry in the list is 0x24 bytes long
add r3, r3, r4
b generic_on_dungeon_bit_func_end

generic_on_dungeon_bit_in_correct_dungeon:
; In the correct dungeon for this dungeon bit.
lis r3, gameInfo_ptr@ha
lwz r3, gameInfo_ptr@l(r3)
addi r3,r3,0x798

generic_on_dungeon_bit_func_end:
; Now call onDungeonBit with argument r3 being the stage info that was determined above.
mr r4, r6 ; Argument r4 is the bit index
bl onDungeonItem

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global generic_small_key_item_get_func
generic_small_key_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

mr r5, r3 ; Argument r3 is the stage ID of the dungeon to add this item for
lis r3, gameInfo_ptr@ha
lwz r3, gameInfo_ptr@l(r3)
addi r3,r3,0x7bc ; stage ID of the current stage
lbz r4, 0 (r3)
cmpw r4, r5 ; Check if we're currently in the right dungeon for this key
bne generic_small_key_item_get_func_not_in_correct_dungeon

bl FUN_025200d4
lwzu r12,0x5150(r3)
lwz r12,0x15c(r12)
mtctr r12
bctrl
lbz r0,9(r3)
rlwinm. r0, r0, 0, 31, 31
beq generic_small_key_item_get_func_in_non_dungeon_room_of_correct_dungeon
b generic_small_key_item_get_func_in_correct_dungeon

generic_small_key_item_get_func_not_in_correct_dungeon:
; Not in the correct dungeon for this small key.
; We need to bypass the normal small key adding method.
; Instead we add directly to the small key count for the correct dungeon's stage info.
lis r3, gameInfo_ptr@ha
lwz r3, gameInfo_ptr@l(r3)
addi r3,r3,0x3a0
mulli r4, r5, 0x24 ; Use stage ID of the dungeon as the index, each entry in the list is 0x24 bytes long
add r3, r3, r4
lbz r4, 0x20 (r3) ; Current number of keys for the correct dungeon
addi r4, r4, 1
stb r4, 0x20 (r3) ; Current number of keys for the correct dungeon
b generic_small_key_item_get_func_end

generic_small_key_item_get_func_in_non_dungeon_room_of_correct_dungeon:
lis r3, gameInfo_ptr@ha
lwz r3, gameInfo_ptr@l(r3)
addi r3,r3,0x798
lbz r4, 0x20 (r3) ; Current number of keys for the current dungeon
addi r4, r4, 1
stb r4, 0x20 (r3) ; Current number of keys for the current dungeon
b generic_small_key_item_get_func_end

generic_small_key_item_get_func_in_correct_dungeon:
; In the correct dungeon for this small key.
; Simply call the normal small key func, as it will work correctly in this case.
bl item_func_small_key

generic_small_key_item_get_func_end:
lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global drc_small_key_item_get_func
drc_small_key_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 3 ; DRC stage ID
bl generic_small_key_item_get_func

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global fw_small_key_item_get_func
fw_small_key_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 4 ; FW stage ID
bl generic_small_key_item_get_func

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global totg_small_key_item_get_func
totg_small_key_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 5 ; TotG stage ID
bl generic_small_key_item_get_func

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global et_small_key_item_get_func
et_small_key_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 6 ; ET stage ID
bl generic_small_key_item_get_func

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global wt_small_key_item_get_func
wt_small_key_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 7 ; WT stage ID
bl generic_small_key_item_get_func

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr




.global drc_big_key_item_get_func
drc_big_key_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 3 ; DRC stage ID
li r4, 2 ; Big key bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global fw_big_key_item_get_func
fw_big_key_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 4 ; FW stage ID
li r4, 2 ; Big key bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global totg_big_key_item_get_func
totg_big_key_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 5 ; TotG stage ID
li r4, 2 ; Big key bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global et_big_key_item_get_func
et_big_key_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 6 ; ET stage ID
li r4, 2 ; Big key bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global wt_big_key_item_get_func
wt_big_key_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 7 ; WT stage ID
li r4, 2 ; Big key bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr




.global drc_dungeon_map_item_get_func
drc_dungeon_map_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 3 ; DRC stage ID
li r4, 0 ; Dungeon map bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global fw_dungeon_map_item_get_func
fw_dungeon_map_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 4 ; FW stage ID
li r4, 0 ; Dungeon map bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global totg_dungeon_map_item_get_func
totg_dungeon_map_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 5 ; TotG stage ID
li r4, 0 ; Dungeon map bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global ff_dungeon_map_item_get_func
ff_dungeon_map_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 2 ; FF stage ID
li r4, 0 ; Dungeon map bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global et_dungeon_map_item_get_func
et_dungeon_map_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 6 ; ET stage ID
li r4, 0 ; Dungeon map bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global wt_dungeon_map_item_get_func
wt_dungeon_map_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 7 ; WT stage ID
li r4, 0 ; Dungeon map bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr




.global drc_compass_item_get_func
drc_compass_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 3 ; DRC stage ID
li r4, 1 ; Compass bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global fw_compass_item_get_func
fw_compass_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 4 ; FW stage ID
li r4, 1 ; Compass bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global totg_compass_item_get_func
totg_compass_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 5 ; TotG stage ID
li r4, 1 ; Compass bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global ff_compass_item_get_func
ff_compass_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 2 ; FF stage ID
li r4, 1 ; Compass bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global et_compass_item_get_func
et_compass_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 6 ; ET stage ID
li r4, 1 ; Compass bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global wt_compass_item_get_func
wt_compass_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 7 ; WT stage ID
li r4, 1 ; Compass bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global dragon_tingle_statue_item_get_func
dragon_tingle_statue_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

lis r3, gameInfo_ptr@ha
lwz r3, gameInfo_ptr@l(r3)
addi r3,r3,0x644
li r4, 0x6A04 ; Unused event bit we use for Dragon Tingle Statue
bl onEventBit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global forbidden_tingle_statue_item_get_func
forbidden_tingle_statue_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

lis r3, gameInfo_ptr@ha
lwz r3, gameInfo_ptr@l(r3)
addi r3,r3,0x644
li r4, 0x6A08 ; Unused event bit we use for Forbidden Tingle Statue
bl onEventBit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global goddess_tingle_statue_item_get_func
goddess_tingle_statue_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

lis r3, gameInfo_ptr@ha
lwz r3, gameInfo_ptr@l(r3)
addi r3,r3,0x644
li r4, 0x6A10 ; Unused event bit we use for Goddess Tingle Statue
bl onEventBit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global earth_tingle_statue_item_get_func
earth_tingle_statue_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

lis r3, gameInfo_ptr@ha
lwz r3, gameInfo_ptr@l(r3)
addi r3,r3,0x644
li r4, 0x6A20 ; Unused event bit we use for Earth Tingle Statue
bl onEventBit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global wind_tingle_statue_item_get_func
wind_tingle_statue_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

lis r3, gameInfo_ptr@ha
lwz r3, gameInfo_ptr@l(r3)
addi r3,r3,0x644
li r4, 0x6A40 ; Unused event bit we use for Wind Tingle Statue
bl onEventBit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global check_tingle_statue_owned
check_tingle_statue_owned:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

cmpwi r4, 0xF ; The opened flag index (argument r4) for tingle statue chests should always be 0xF.
bne check_tingle_statue_owned_invalid

; The stage ID (argument r3) determines which dungeon it's checking.
cmpwi r3, 3
beq check_dragon_tingle_statue_owned
cmpwi r3, 4
beq check_forbidden_tingle_statue_owned
cmpwi r3, 5
beq check_goddess_tingle_statue_owned
cmpwi r3, 6
beq check_earth_tingle_statue_owned
cmpwi r3, 7
beq check_wind_tingle_statue_owned
b check_tingle_statue_owned_invalid

check_dragon_tingle_statue_owned:
li r4, 0x6A04 ; Unused event bit
b check_tingle_statue_owned_event_bit

check_forbidden_tingle_statue_owned:
li r4, 0x6A08 ; Unused event bit
b check_tingle_statue_owned_event_bit

check_goddess_tingle_statue_owned:
li r4, 0x6A10 ; Unused event bit
b check_tingle_statue_owned_event_bit

check_earth_tingle_statue_owned:
li r4, 0x6A20 ; Unused event bit
b check_tingle_statue_owned_event_bit

check_wind_tingle_statue_owned:
li r4, 0x6A40 ; Unused event bit

check_tingle_statue_owned_event_bit:
lis r3, gameInfo_ptr@ha
lwz r3, gameInfo_ptr@l(r3)
addi r3,r3,0x644
bl isEventBit
b check_tingle_statue_owned_end

check_tingle_statue_owned_invalid:
; If the function call was somehow invalid, return false.
li r3, 0

check_tingle_statue_owned_end:
lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global custom_isTbox_for_unloaded_stage_save_info
custom_isTbox_for_unloaded_stage_save_info:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

mulli r0, r3, 0x24 ; Use stage info ID as the index, each entry in the list is 0x24 bytes long
lis r3, gameInfo_ptr@ha
lwz r3, gameInfo_ptr@l(r3)
addi r3,r3,0x3a0
add r3, r3, r0

bl isTbox

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.close
