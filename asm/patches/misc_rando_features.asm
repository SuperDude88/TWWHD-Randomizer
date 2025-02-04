.org 0x025aece4
	bl init_save_with_tweaks

; Set initial HP from a custom symbol and also also allow the initial current HP to be rounded down from the initial max HP (for starting with some heart pieces).
.org 0x025b4dd4
	b set_starting_health
.org 0x025b4ddc
	nop
.org 0x025b4de8
	nop
.org @NextFreeSpace
.global set_starting_health
set_starting_health:
  ; Base address to write health to is still stored in r3 from init__10dSv_save_cFv
  lis r31, starting_quarter_hearts@ha
  addi r31, r31, starting_quarter_hearts@l
  lhz r11, 0x0(r31)
  sth r11, 0x0(r3) ; Store maximum HP (including unfinished heart pieces)
  rlwinm r11, r11, 0, 0, 29
  sth r11, 0x2(r3) ; Store current HP (not including unfinished heart pieces)

  b 0x025b4dd8


; Refill the player's magic meter to full when they load a save.
.org 0x025ba940
	b fully_refill_magic_meter_on_load_save
.org @NextFreeSpace
.global fully_refill_magic_meter_on_load_save
fully_refill_magic_meter_on_load_save:
  lis r3, gameInfo_ptr@ha
  lwz r3, gameInfo_ptr@l(r3)
  addi r3, r3, 0x33
  lbz r4, 0 (r3) ; Load max magic meter
  stb r4, 1 (r3) ; Store to current magic meter

  li r28, 0 ; Replace the line we overwrote to branch here
  b 0x025ba944 ; Return


; Normally the game will give you at least 3 hearts when you reload a save
; This is undesirable when starting the game with less than 3 hearts
; So we need to cap it if your max health is less than 3 hearts
.org 0x025BAA3C ; when saving a file, after checking if current health is less than 3 hearts
  b cap_file_restore_health
.org @NextFreeSpace
.global cap_file_restore_health
cap_file_restore_health:
  lhz r0, 0(r31) ; load max health (r31 has a pointer to dSv_info_c)
  rlwinm r0, r0, 0, 0, 29 ; round down to nearest full heart (exclude extra heart pieces)

  cmplwi r0, 0xC
  ble cap_file_restore_health_return ; use the maximum value if it's already < 3 hearts
  
  li r0, 0xC ; give 3 hearts

cap_file_restore_health_return:
  b 0x025BAA40 ; return to store our health value (from r0)

; Also cap the health you're given after a game over
.org 0x025499EC
  b cap_game_over_restore_health
.org @NextFreeSpace
.global cap_game_over_restore_health
cap_game_over_restore_health:
  lhz r0, 0x20(r12) ; load max health (r12 has a pointer to SaveData)
  rlwinm r0, r0, 0, 0, 29 ; round down to nearest full heart (exclude extra heart pieces)

  cmplwi r0, 0xC
  ble cap_game_over_restore_health_return ; use the maximum value if it's already <3
  
  li r0, 0xC ; give 3 hearts

cap_game_over_restore_health_return:
  b 0x025499F0 ; return to store our health value (from r0)


; Animate the 500 rupee to be a rainbow rupee that animates between the colors of all other rupees.
.org 0x021836d4
	b check_animate_rainbow_rupee_color
; Manually animate rainbow rupees to cycle through all other rupee colors.
; In order to avoid an abrupt change from silver to green when it loops, we make the animation play forward and then backwards before looping, so it's always a smooth transition.
.org @NextFreeSpace
.global check_animate_rainbow_rupee_color
check_animate_rainbow_rupee_color:
  ; Check if the color for this rupee specified in the item resources is 7 (originally unused, we use it as a marker to separate the rainbow rupee from other color rupees).
  cmpwi r0, 7
  beq animate_rainbow_rupee_color

  ; If it's not the rainbow rupee, replace the line of code we overwrote to jump here, and then return to the regular code for normal rupees.
  xoris r0, r0, 0x8000
  b 0x021836d8

  animate_rainbow_rupee_color:

  ; If it is the rainbow rupee, we need to increment the current keyframe (a float) by certain value every frame.
  ; The keyframe is stored to the rupee actor +0x3A0, repurposes a couple fields the rupee doesn't use
  xoris r0, r0, 0x8000
  lfs f0, 0x3A0(r31) ; Read current keyframe


  lis r9, rainbow_rupee_data@ha
  addi r9, r9, rainbow_rupee_data@l
  lfs f13, 0(r9) ; Read amount to add to keyframe per frame
  fadds f0, f0, f13 ; Increase the keyframe value

  lfs f13, 4(r9) ; Read the maximum keyframe value
  fcmpo cr0,f0,f13
  ; If we're less than the max we don't need to reset the value
  blt store_rainbow_rupee_keyframe_value

  ; If we're greater than the max, reset the current keyframe to the minimum.
  ; The minimum is actually the maximum negated. This is to signify that we're playing the animation backwards.
  lfs f0, 8(r9)

  store_rainbow_rupee_keyframe_value:
  stfs f0, 0x3A0(r31) ; Store the keyframe back

  ; Take the absolute value of the keyframe. So instead of going from -6 to +6, the value we pass as the actual keyframe goes from 6 to 0 to 6.
  ; Also do an HD thing and round to single-precision
  fabs f0, f0
  frsp f1, f0

  b 0x021836f4

.global rainbow_rupee_data
rainbow_rupee_data:
  .float 0.15 ; Amount to increment keyframe by every frame a rainbow rupee is being drawn
  .float 6.0 ; Max keyframe, when it should loop
  .float -6.0 ; Minimum keyframe



; Checks if the upcoming message text to parse has a custom text command.
.org 0x02600634
	b check_run_new_text_commands
; Check the ID of the upcoming text command to see if it's a custom one, and runs custom code for it if so.
.org @NextFreeSpace
.global check_run_new_text_commands
check_run_new_text_commands:
	cmplwi r0, 0x7
	bne check_run_new_text_commands_check_failed

	lhz r0, 0x4(r6)
	cmplwi r0, 0x4B
	blt check_run_new_text_commands_check_failed
	cmplwi r0, 0x4F
	bgt check_run_new_text_commands_check_failed

  mr r6, r0
  b exec_curr_num_keys_text_command ; Weird return thing that Ghidra calls CALL_RETURN

check_run_new_text_commands_check_failed:
	li r3, -0x1
	b 0x02600638 ; jump back

; Updates the current message string with the number of keys for a certain dungeon.
; Argument in r6 is the command id
.global exec_curr_num_keys_text_command
exec_curr_num_keys_text_command:
  stwu sp, -0x20(sp)
  stmw r27, 0xC(sp)
  mflr r0
  stw r0, 0x24(sp)
  
  ; Convert the text command ID to the dungeon stage ID.
  ; The text command ID ranges from 0x4B-0x4F, for DRC, FW, TotG, ET, and WT.
  ; The the dungeon stage IDs for those same 5 dungeons range from 3-7.
  ; So just subtract 0x48 to get the right stage ID.
  addi r6, r6, -0x48

  mr r29, r3
  mr r30, r4
  mr r31, r5

  lis r9, gameInfo_ptr@ha
  lwz r9, gameInfo_ptr@l(r9)
  addi r9, r9, 0x7BC ; stage ID of the current stage
  lbz r10, 0(r9)
  cmpw r10, r6 ; Check if we're currently in the right dungeon for this key
  beq exec_curr_num_keys_text_command_in_correct_dungeon

exec_curr_num_keys_text_command_not_in_correct_dungeon:
  ; Read the current number of small keys from that dungeon's stage info.
  lis r9, gameInfo_ptr@ha
  lwz r9, gameInfo_ptr@l(r9)
  addi r9, r9, 0x3A0
  mulli r6, r6, 0x24 ; Use stage ID of the dungeon as the index, each entry in the list is 0x24 bytes long
  add r9, r9, r6
  b exec_curr_num_keys_text_command_after_getting_stage_info

exec_curr_num_keys_text_command_in_correct_dungeon:
  ; Read the current number of small keys from the currently loaded dungeon info.
  lis r9, gameInfo_ptr@ha
  lwz r9, gameInfo_ptr@l(r9)
  addi r9,r9, 0x798

exec_curr_num_keys_text_command_after_getting_stage_info:
  lbz r6, 0x20(r9) ; Current number of keys for the dungeon
  mr r28, r9 ; Remember the correct stage info pointer for later when we check the big key

  mr r3, r29 ; this pointer
  mr r4, r30 ; message pointer
  mr r5, r31 ; current index in message
  ; r6 has the number of keys
  li r7, 0 ; not sure what these do
  li r8, 1 ; not sure what these do
  bl replaceItemCount
  add r31, r31, r3 ; update index in message
  mr r27, r3 ; keep the return value

  ; Check whether the player has the big key or not.
  lbz r6, 0x21 (r28) ; Bitfield of dungeon-specific flags in the appropriate stage info
  rlwinm. r6, r6, 0, 29, 29 ; Extract the has big key bit
  bne exec_curr_num_keys_text_command_has_big_key

  lis r6, no_big_key_label_safestring@ha
  addi r6, r6, no_big_key_label_safestring@l
  b add_big_key_unit_text

exec_curr_num_keys_text_command_has_big_key:
  lis r6, big_key_label_safestring@ha
  addi r6, r6, big_key_label_safestring@l

add_big_key_unit_text:
  mr r3, r29
  mr r4, r30
  mr r5, r31
  ; r6 has pointer to message label
  bl replaceItemUnit
  add r3, r27, r3 ; return length of replacements

  lmw r27, 0xC(sp)
  lwz r0, 0x24(sp)
  mtlr r0
  addi sp, sp, 0x20
  blr

.global no_big_key_label_str
no_big_key_label_str:
  .string "Unit_Key_00"

.global big_key_label_str
big_key_label_str:
  .string "Unit_Key_01"

.global no_big_key_label_safestring
no_big_key_label_safestring:
  .long no_big_key_label_str
  .long safestring_vtbl_1010394c

.global big_key_label_safestring
big_key_label_safestring:
  .long big_key_label_str
  .long safestring_vtbl_1010394c

.align 2



; Make cannons die in 1 hit from the boomerang instead of 2.
.org 0x02330c24
	subi r12, r8, 0x2



; In daPy_lk_c::playerInit
.org 0x024126b8
	bl check_player_in_casual_clothes
	cmpwi r3, 0
	beq 0x024126d4
	b 0x024126e4
.org @NextFreeSpace
.global check_player_in_casual_clothes
check_player_in_casual_clothes:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)

  lis r3, should_start_with_heros_clothes@ha
  addi r3, r3, should_start_with_heros_clothes@l
  lbz r3, 0 (r3) ; Load bool of whether player should start with Hero's clothes
  cmpwi r3, 1
  beq check_player_in_casual_clothes_hero

  check_player_in_casual_clothes_casual:
  li r3, 1
  b check_player_in_casual_clothes_end

  check_player_in_casual_clothes_hero:
  li r3, 0

  check_player_in_casual_clothes_end:
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

; In some J3DPacketS-related function, affects reflection in Puppet Ganon's room
.org 0x02083028
  bl check_player_in_casual_clothes
  cmpwi r3, 0


; Make Aryll always wear her pirate outfit, not just in Second Quest.
.org 0x0227e5a4
	nop
.org 0x022830e0
	b 0x0228314c


; Change the condition for Outset switching to its alternate BGM theme from checking event bit 0E20 (PIRATES_ON_OUTSET, for Aryll being kidnapped) to instead check if the Pirate Ship chest has been opened (since Aryll is in the Pirate Ship in the randomizer).
.org 0x02025200
	bl check_outset_bgm

.org 0x020238cc
	bl check_outset_bgm

.org 0x020268c0
	bl check_outset_bgm

.org 0x020268d0
	bl check_outset_bgm
.org @NextFreeSpace
.global check_outset_bgm
check_outset_bgm:
	stwu sp, -0x10 (sp)
	mflr r0
	stw r0, 0x14 (sp)
	li r3, 13
	li r4, 5
	bl custom_isTbox_for_unloaded_stage_save_info
	lwz r0, 0x14 (sp)
	mtlr r0
	addi sp, sp, 0x10
	blr

 ; Also remove a check for watching the outset intro
 ; HD save init oddities mean the event bit isn't set until after this function runs
.org 0x02026804
	li r3, 1

; Replace the calls to getCollectMapNum on the quest status screen with a call to a custom function that checks the number of owned tingle statues.
.org 0x0263b250
	bl get_num_owned_tingle_statues
.org @NextFreeSpace
.global get_num_owned_tingle_statues
get_num_owned_tingle_statues:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)

  li r6, 0

  li r3, 3 ; Dragon Tingle Statue
  li r4, 0xF
  bl check_tingle_statue_owned
  add r6, r6, r3

  li r3, 4 ; Forbidden Tingle Statue
  li r4, 0xF
  bl check_tingle_statue_owned
  add r6, r6, r3

  li r3, 5 ; Goddess Tingle Statue
  li r4, 0xF
  bl check_tingle_statue_owned
  add r6, r6, r3

  li r3, 6 ; Earth Tingle Statue
  li r4, 0xF
  bl check_tingle_statue_owned
  add r6, r6, r3

  li r3, 7 ; Wind Tingle Statue
  li r4, 0xF
  bl check_tingle_statue_owned
  add r6, r6, r3

  mr r3, r6

  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr


; Give the inter-dungeon warp pots a different smoke particle color compared to other warp pots to help distinguish them.
.org 0x023ba514
	b set_prm_color_for_warp_pot_particles
.org 0x023ba534
	b set_env_color_for_warp_pot_particles
.org @NextFreeSpace
.global set_prm_color_for_warp_pot_particles
set_prm_color_for_warp_pot_particles:
  add r12, r12, r0 ; Replace line overwrote to jump here

  lwz r10, 0x3D4 (r31) ; Event register index. 2 and 5 are for inter-dungeon warp pots.
  cmpwi r10, 2
  beq set_prm_color_for_warp_pot_particles_is_inter_dungeon
  cmpwi r10, 5
  bne set_prm_color_for_warp_pot_particles_is_not_inter_dungeon

  set_prm_color_for_warp_pot_particles_is_inter_dungeon:
  lis r12, custom_warp_pot_prm_color@ha
  addi r12, r12, custom_warp_pot_prm_color@l
  b 0x023ba518 ; Return

  set_prm_color_for_warp_pot_particles_is_not_inter_dungeon:
  b 0x023ba518 ; Return

.global set_env_color_for_warp_pot_particles
set_env_color_for_warp_pot_particles:
  add r10, r10, r0 ; Replace line overwrote to jump here

  lwz r11, 0x3D4 (r31) ; Event register index. 2 and 5 are for inter-dungeon warp pots.
  cmpwi r11, 2
  beq set_env_color_for_warp_pot_particles_is_inter_dungeon
  cmpwi r11, 5
  bne set_env_color_for_warp_pot_particles_is_not_inter_dungeon

  set_env_color_for_warp_pot_particles_is_inter_dungeon:
  lis r10, custom_warp_pot_env_color@ha
  addi r10, r10, custom_warp_pot_env_color@l
  b 0x023ba538 ; Return

  set_env_color_for_warp_pot_particles_is_not_inter_dungeon:
  b 0x023ba538 ; Return

.global custom_warp_pot_prm_color ; The main color
custom_warp_pot_prm_color:
  .int 0xE5101B80 ; Bright red
.global custom_warp_pot_env_color ; The outline color
custom_warp_pot_env_color:
  .int 0x3C379D80 ; Dark purple


 ; set an arbitrary damage multiplier for hero mode
.org 0x023F5218
	b multiply_damage
.org @NextFreeSpace
.global multiply_damage
multiply_damage:
	lis r11, custom_damage_multiplier@ha
	lfs f0, custom_damage_multiplier@l(r11)
	fmuls f31, f31, f0
	b 0x023f521c

.global custom_damage_multiplier
custom_damage_multiplier:
	.float 2.0



; Change the Deku Leaf so that you can still fan it to create a gust of air when you have zero magic.
; In vanilla, you needed at least one magic to fan it, but it didn't consume any magic.
; This is done so that the Deku Leaf still has some usefulness when starting without a magic meter.
; It also allows the fan ability (which allows accessing Horseshoe Island's golf) to be separated from the flying ability.
.org 0x023E77CC ; In daPy_lk_c::setShapeFanLeaf(void)
  ; Remove branch taken when having less than 1 magic.
  nop



 ; Allow the game to load uncompressed szs files
 ; This is super hacky and a terrible idea but saves a ton of time when randomizing (compression is slow)
 ; in sead::SZSDecompressor::decomp
;  .org 0x0275ed6c
;   b load_uncompressed_szs
;   lwz r29, 0xC(sp)
;   lwz r0, 0x1C(sp)
;   lwz r30, 0x10(sp)
;   mtlr r0
;   lwz r31, 0x14(sp)
;   addi sp, sp, 0x18
;   blr
; .org @NextFreeSpace
; .global load_uncompressed_szs
; load_uncompressed_szs:
;   lis r9,0x5341
;   addi r9,r9,0x5243
;   cmplw r3, r9 ; Check if file is SARC (should be)
;   bne return_err_1
;   lwz r5, 0x8(r30)
;   cmplw r29, r5 ; Check if output buffer is large enough
;   blt return_err_2
;   mr r3, r31
;   mr r4, r30
;   li r6, 0
;   .byte 0x49, 0x8C, 0xAE, 0x09 ; OSBlockMove, not sure the proper way to do the rpl call, manually done with a relocation (these bytes are a random absolute branch so Cemu loads it properly)
;   mr r3, r5 ; Return size
;   b 0x0275ed70 ; Branch back
; 
; .global return_err_1
; return_err_1:
;   li r3, -0x1
;   b 0x0275ed88 ; Return -1 if it isn't SARC
; 
; .global return_err_2
; return_err_2:
;   li r3, -0x2
;   b 0x0275edac ; Return -1 if it isn't SARC
; 
;  ; in sead::SZSDecompressor::getDecompSize
; .org 0x0275eba8
;   b get_decompressed_szs_size
; .org @NextFreeSpace
; .global get_decompressed_szs_size
; get_decompressed_szs_size:
;   lwz r4, 0(r3)
;   lis r5, 0x5961
;   addi r5, r5, 0x7a30
;   cmplw r4, r5
;   beq read_yaz0_size
;   lis r5, 0x5341
;   addi r5, r5, 0x5243
;   cmplw r4, r5
;   beq read_sarc_size
;   li r3, -1
;   b 0x0275ebac
; 
; read_yaz0_size:
;   lwz r3, 0x4(r3)
;   b 0x0275ebac
; read_sarc_size:
;   lwz r3, 0x8(r3)
;   b 0x0275ebac
; 
;  ; in sead::SZSDecompressor::getDecompAlignment
; .org 0x0275f180
;   b get_alignment
; .org @NextFreeSpace
; .global get_alignment
; get_alignment:
;   lwz r4, 0(r3)
;   lis r5, 0x5961
;   addi r5, r5, 0x7a30
;   cmplw r4, r5
;   beq read_yaz0_align
;   li r3, 0
;   b 0x0275ebac
; 
; read_yaz0_align:
;   lwz r3, 0x8(r3)
;   b 0x0275ebac
; 
; 
;  ; in sead::SZSDecompressor::readHeader_
; .org 0x0275edfc
;   b check_sarc_magic ; not YAZ0
; .org @NextFreeSpace
; .global check_sarc_magic
; check_sarc_magic:
;   lwz r11, 0(r4)
;   lis r7, 0x5341
;   addi r7, r7, 0x5243
;   cmplw r11, r7
;   bne not_sarc_magic ; not SARC
;   li r7, 1
;   stb r7, 0xF(r12)
;   li r7, 0
;   stb r7, 0x16(r12)
;   li r3, 0x0 ; Return no error
;   blr ; return back to streamDecomp
; not_sarc_magic:
;   b 0x0275ee44
; 
; .org 0x275ef18
;   b copy_sarc_to_output
; .org @NextFreeSpace
; .global copy_sarc_to_output
; copy_sarc_to_output:
;   lbz r3, 0xF(r30)
;   cmpwi r3, 0x1
;   bne continue_decomp
;   lwz r3, 0x0(r30) ; load dst, src already in r4
;   mr r5, r31 ; move source length
;   li r6, 0
;   mflr r0
;   .byte 0x49, 0x8C, 0xAE, 0x09 ; OSBlockMove, not sure the proper way to do the rpl call, manually done with a relocation (these bytes are a random absolute branch so Cemu loads it properly)
;   lwz r3, 0x0(r30)
;   add r3, r3, r5
;   stw r3, 0x0(r30)
;   mr r3, r5
;   b 0x0275f150 ; return from streamDecomp
; continue_decomp:
;   lwz r3, 0x4(r30)
;   b 0x0275ef18
; 
;  ; In some function that loads .pack files
; .org 0x026118b4
;   lis r3, 0x520 ; Increase heap size
; 
; 
;  ; 3D-related heaps
; .org 0x0203e630
;   lis r11, 0x230A ; Increase 3DRootHeap size by 0x3EA00000
; .org 0x0203f1c8
;   addis r31, r31, 0x1AAA ; Increase ModelRes heap size by 0x3EA0000
; .org 0x0203f1f8
;   addis r3, r3, 0x112A ; Increase PermanentResource heap size by 0x3EA0000
; 
;  ; in main()
; .org 0x02005f14
;   lis r0, 0x3CB0 ; Up the root heap size
;   ori r0, r0, 0x0000

; Slightly increase the PermanentResource heap size
; It seems like Nintendo was flying really close to the sun with these and our slightly larger files cause issues
.org 0x0203f1f8
  addis r3, r3, 0x0D45 ; Increase heap size by 0x50000 (320KiB)



.org 0x100f7bd0 ; quadrant index for button index 9
  .int 0
  
.org 0x0267b4fc
  b init_extra_button_location
.org @NextFreeSpace
.global init_extra_button_location
init_extra_button_location:
  stb r31, 0x10(r6) ; replace line we overwrote
  li r31, 0xFD ; -3
  stb r31, 0x12(r6)
  stb r31, 0x13(r6)
  b 0x0267b500

.org 0x02681ce0
  b init_extra_connections
.org @NextFreeSpace
.global init_extra_connections
init_extra_connections:
  ; Add FF connection to Mother and Child button
  bl SetAdjacentWarpButton ; replace line we overwrote
  li r3, 9 ; FF warp index
  bl GetWarpQuadrantByIndex ; ff warp quadrant
  addi r4, sp, 0x34
  stw r3, 0x34(sp)
  lswi r6, r4, 0x2
  addi r5, r1, 0x1c
  mr r4, r30
  mr r3, r31
  stswi r6, r5, 0x2
  li r6, 1
  bl SetAdjacentWarpButton ; set Mother and Child top-left button to be Forsaken Fortress

  ; Also set up the FF button
  li r3, 9 ; FF warp index
  bl GetWarpQuadrantByIndex ; ff warp quadrant
  addi r4, sp, 0x28
  stw r3, 0x28(sp)
  addi r5, sp, 0xA
  lswi r6, r4, 0x2
  mr r4, r5
  mr r3, r31
  stswi r6, r5, 0x2
  bl GetQuadrantAtLocation
  mr. r30, r3
  beq continue_original_connections

  li r3, 0x1
  stb r3, 0x9A(r30)

  li r3, 0 ; m&c warp index
  bl GetWarpQuadrantByIndex ; m&c warp quadrant
  addi r4, sp, 0x34
  stw r3, 0x34(sp)
  lswi r6, r4, 0x2
  addi r5, r1, 0x1c
  mr r4, r30
  mr r3, r31
  stswi r6, r5, 0x2
  li r6, 5
  bl SetAdjacentWarpButton ; set Forsaken Fortress bottom-right button to be Mother and Child

continue_original_connections:
  b 0x02681ce4 ; jump back


.org @NextFreeSpace
.global custom_warp_button_name
custom_warp_button_name:
  .string "L_WarpArea_09"
  
.align 2

.global custom_warp_button_safestring
custom_warp_button_safestring:
  .long custom_warp_button_name
  .long safestring_vtbl_1010394c

.org 0x026f0394
  b load_extra_button_part
.org @NextFreeSpace
.global load_extra_button_part
load_extra_button_part:
  cmpwi r30, 0xB
  bge continue_normal_return

  li r23, 0 ; set offset to 0 so we start at index 0 in our list
  lis r31, custom_warp_button_safestring@ha ; load our own SafeString* into r31
  addic r31, r31, custom_warp_button_safestring@l
  b 0x026f0070 ; loop one more time

continue_normal_return:
  li r3, 1
  b 0x026f0398


.org 0x0267a264 ; increase number of buttons to search when finding quadrant index -> button index
  li r9, 0xA

.org 0x026ef340 ; increase number of buttons to loop through when initializing animations
  li r29, 0xA

.org 0x02681bc8 ; increase number of buttons to zero out
  li r29, 0xA

.org 0x02678784 ; increase number of buttons to loop through when opening confirmation text
  li r29, 0xA


.org @NextFreeSpace
.global custom_ff_label
custom_ff_label:
  .string "00076"

.align 2

.global custom_ff_label_safestring
custom_ff_label_safestring:
  .long custom_ff_label
  .long safestring_vtbl_1010394c

.org 0x026d9ce4
  b set_ff_warp_msg_index
.org @NextFreeSpace
.global set_ff_warp_msg_index
set_ff_warp_msg_index:
  addi r9, r10, 0x8 ; replace line we overwrote
  cmpwi r9, 0x12
  bne not_ff_warp
  li r9, 0x33 ; index 51
not_ff_warp:
  b 0x026d9ce8

.org 0x026d9d28
  b ff_warp_text_check
.org @NextFreeSpace
.global ff_warp_text_check
ff_warp_text_check:
  addi r4, sp, 0x8 ; replace line we overwrote
  cmpwi r9, 0x33 ; ff message index
  bne not_ff_offset
  lis r5, custom_ff_label_safestring@ha
  addic r5, r5, custom_ff_label_safestring@l
not_ff_offset:
  b 0x026d9d2c



.global custom_l_objectName
custom_l_objectName:
  .byte 'S' ; Actor name
  .byte 'w'
  .byte 'O'
  .byte 'p'
  .byte 0
  .byte 0
  .byte 0
  .byte 0
  .short 0x01E4 ; Actor ID
  .byte 0xFF ; Subtype
  .byte 0 ; GBA Name


; Modify the loops that check the actor names list to also check our custom list, allowing us to add new actors without replacing existing ones.
; In dStage_searchName
.org 0x025C10A4
  li r10, 0 ; Change the loop to count up from zero instead of down from 0x329
.org 0x025C10D4
  addi r10, r10, 1 ; Increment instead of decrement
.org 0x025C10DC
  b custom_searchName_loop_check ; After loop, no match was found
.org @NextFreeSpace
.global custom_searchName_loop_check
custom_searchName_loop_check:
  cmplwi r10, 0x329
  blt continue_normal_search_loop
  beq read_custom_l_objectName_loop_for_dStage_searchName_switch_from_vanilla_to_custom

  ; If we're past even the indexes for our custom list, end the loop.
  cmplwi r10, 0x329 + 1 ; Num entries in vanilla list + custom list
  bge read_custom_l_objectName_loop_for_dStage_searchName_end_loop

read_custom_l_objectName_loop_for_dStage_searchName_switch_from_vanilla_to_custom:
  ; Replace the pointer to the current entry of the vanilla l_objectName in r31 with a pointer to the start of our custom one.
  lis r3, custom_l_objectName@ha
  addi r3, r3, custom_l_objectName@l

continue_normal_search_loop:
  b 0x025C10AC

read_custom_l_objectName_loop_for_dStage_searchName_end_loop:
  b 0x025C10E0 ; Return to after the end of the loop


; In dStage_getName
.org 0x025c10f0
  b set_up_custom_loop_counter
.org @NextFreeSpace
.global set_up_custom_loop_counter
set_up_custom_loop_counter:
  li r11, 0
  li r10, 0 ; replace the line we overwrote to jump here
  b 0x025C10F4 ; jump back
.org 0x025C112C
  b increment_custom_loop_counter
.org @NextFreeSpace
.global increment_custom_loop_counter
increment_custom_loop_counter:
  addi r11, r11, 1
  addi r12, r12, 0xC ; replace the line we overwrote to jump here
  b 0x025C1130 ; jump back
.org 0x025C1140
  b read_custom_l_objectName_loop_for_dStage_getName
.org @NextFreeSpace
.global read_custom_l_objectName_loop_for_dStage_getName
read_custom_l_objectName_loop_for_dStage_getName:
  beq has_actor_with_same_proc_name ; replace the line we overwrote to jump here
  cmplwi r11, 0x329 ; custom loop counter
  bgt read_custom_l_objectName_loop_for_dStage_getName_end_loop
read_custom_l_objectName_loop_for_dStage_getName_switch_from_vanilla_to_custom:
  ; Replace the pointer to the current entry of the vanilla l_objectName in r12 with a pointer to the start of our custom one.
  lis r12, custom_l_objectName@ha
  addi r12, r12, custom_l_objectName@l
  ; Then restart the original loop counter so it loops for our custom list.
  li r0, 1 ; Num entries in our custom list
  mtctr r0
  b 0x025C10FC ; Return to the start of the vanilla loop

has_actor_with_same_proc_name:
  b 0x025C1144
read_custom_l_objectName_loop_for_dStage_getName_end_loop:
  b 0x025C1148 ; Return to after the end of the loop
