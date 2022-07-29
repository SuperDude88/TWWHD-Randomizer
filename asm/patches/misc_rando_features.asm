 ; save init needs more research before it can be added
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
  lhz r11, 0 (r31)
  sth r11, 0 (r3) ; Store maximum HP (including unfinished heart pieces)
  rlwinm r11,r11,0,0,29
  sth r11, 2 (r3) ; Store current HP (not including unfinished heart pieces)
  
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


; Animate the 500 rupee to be a rainbow rupee that animates between the colors of all other rupees.

.org 0x021836d4 ; branch to custom code replaces a load from .data, need to remove relocation to keep branch offset intact (.rela.text::0007b6fc)
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
  ; (Note: The way this is coded would increase it by this value multiplied by the number of rainbow rupees being drawn. This is fine since there's only one rainbow rupee but would cause issues if we placed multiple of them. Would need to find a different place to increment the keyframe in that case, somewhere only called once per frame.)
  xoris r0, r0, 0x8000
  lis r8, rainbow_rupee_keyframe@ha
  addi r8, r8, rainbow_rupee_keyframe@l
  lfs f0, 0 (r8) ; Read current keyframe
  lfs f13, 4 (r8) ; Read amount to add to keyframe per frame
  fadds f0, f0, f13 ; Increase the keyframe value
  
  lfs f13, 8 (r8) ; Read the maximum keyframe value
  fcmpo cr0,f0,f13
  ; If we're less than the max we don't need to reset the value
  blt store_rainbow_rupee_keyframe_value
  
  ; If we're greater than the max, reset the current keyframe to the minimum.
  ; The minimum is actually the maximum negated. This is to signify that we're playing the animation backwards.
  lfs f0, 0xC (r8)
  
  store_rainbow_rupee_keyframe_value:
  stfs f0, 0 (r8) ; Store the new keyframe value back
  
  ; Take the absolute value of the keyframe. So instead of going from -6 to +6, the value we pass as the actual keyframe goes from 6 to 0 to 6.
  ; Also do an HD thing and round to single-precision
  fabs f0, f0
  frsp f1, f0
  
  b 0x021836f4

.global rainbow_rupee_keyframe
rainbow_rupee_keyframe:
  .float 0.0 ; Current keyframe, acts as a global variable modified every frame
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
  mr r30, r0
  stwu sp, -0x18(sp)
  stw r30, 0x10(sp)
  stw r31, 0x14(sp)
  stw r29, 0xC(sp)
  mflr r0
  stw r0, 0x1C(sp)
	bl exec_curr_num_keys_text_command
  b 0x025fdfe4

check_run_new_text_commands_check_failed:
	li r3, -0x1
	b 0x02600638 ; jump back

; Updates the current message string with the number of keys for a certain dungeon.
.global exec_curr_num_keys_text_command
exec_curr_num_keys_text_command:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  mr r13, r6
  ; Convert the text command ID to the dungeon stage ID.
  ; The text command ID ranges from 0x4B-0x4F, for DRC, FW, TotG, ET, and WT.
  ; The the dungeon stage IDs for those same 5 dungeons range from 3-7.
  ; So just subtract 0x48 to get the right stage ID.
  addi r6, r30, -0x48
  
  lis r9, gameInfo_ptr@ha
  lwz r9, gameInfo_ptr@l(r9)
  addi r9,r9,0x7bc ; stage ID of the current stage
  lbz r10, 0 (r9)
  cmpw r10, r6 ; Check if we're currently in the right dungeon for this key
  beq exec_curr_num_keys_text_command_in_correct_dungeon
  
exec_curr_num_keys_text_command_not_in_correct_dungeon:
  ; Read the current number of small keys from that dungeon's stage info.
  lis r9, gameInfo_ptr@ha
  lwz r9, gameInfo_ptr@l(r9)
  addi r9,r9,0x3a0
  mulli r6, r6, 0x24 ; Use stage ID of the dungeon as the index, each entry in the list is 0x24 bytes long
  add r9, r9, r6
  lbz r6, 0x20 (r9) ; Current number of keys for the correct dungeon
  mr r30, r9 ; Remember the correct stage info pointer for later when we check the big key
  b exec_curr_num_keys_text_command_after_reading_num_keys
  
exec_curr_num_keys_text_command_in_correct_dungeon:
  ; Read the current number of small keys from the currently loaded dungeon info.
  lis r9, gameInfo_ptr@ha
  lwz r9, gameInfo_ptr@l(r9)
  addi r9,r9,0x798
  lbz r6, 0x20 (r9) ; Current number of keys for the current dungeon
  mr r30, r9 ; Remember the correct stage info pointer for later when we check the big key
  
exec_curr_num_keys_text_command_after_reading_num_keys:
  li r7, 0
  li r8, 1
  bl FUN_025fcb90
  
  ; Check whether the player has the big key or not.
  lbz r6, 0x21 (r30) ; Bitfield of dungeon-specific flags in the appropriate stage info
  rlwinm. r6, r6, 0, 29, 29 ; Extract the has big key bit
  beq exec_curr_num_keys_text_command_after_appending_big_key_text
  
; Do a silly hack to overwrite extra space we add to the message (appending properly is much more complex)
exec_curr_num_keys_text_command_has_big_key:
  lis r7, key_text_command_has_big_key_text@ha
  addi r7, r7, key_text_command_has_big_key_text@l
	addi r13, r13, 0x8
  li r8, 5

  copy_char_begin:
    lhz r9, 0(r7)
    sth r9, 0(r13)
    addi r13, r13, 2
    addi r7, r7, 2
    addi r8, r8, -0x1
    cmpwi r8, 0
    ble exec_curr_num_keys_text_command_after_appending_big_key_text
    b copy_char_begin
  
exec_curr_num_keys_text_command_after_appending_big_key_text:
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

key_text_command_has_big_key_text:
  .string "\0 \0+\0B\0i\0g"

.align 2



; Make cannons die in 1 hit from the boomerang instead of 2.
.org 0x02330c24
	subi r12, r8, 0x2



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
 ; This is super hacky and probably a terrible idea but it saves a ton of time randomizing (compression is slow)
 ; in sead::SZSDecompressor::decomp
 .org 0x0275ed6c
  b load_uncompressed_szs
  lwz r29, 0xC(sp)
  lwz r0, 0x1C(sp)
  lwz r30, 0x10(sp)
  mtlr r0
  lwz r31, 0x14(sp)
  addi sp, sp, 0x18
  blr
.org @NextFreeSpace
.global load_uncompressed_szs
load_uncompressed_szs:
  lis r9,0x5341
  addi r9,r9,0x5243
  cmplw r3, r9 ; Check if file is SARC (should be)
  bne return_err_1
  lwz r5, 0x8(r30)
  cmplw r29, r5 ; Check if output buffer is large enough
  blt return_err_2
  mr r3, r31
  mr r4, r30
  li r6, 0
  .byte 0x49, 0x8C, 0xAE, 0x09 ; OSBlockMove, not sure the proper way to do the rpl call, manually done with a relocation (these bytes are a random absolute branch so Cemu loads it properly)
  mr r3, r5 ; Return size
  b 0x0275ed70 ; Branch back

.global return_err_1
return_err_1:
  li r3, -0x1
  b 0x0275ed88 ; Return -1 if it isn't SARC

.global return_err_2
return_err_2:
  li r3, -0x2
  b 0x0275edac ; Return -1 if it isn't SARC

 ; in sead::SZSDecompressor::tryDecompFromDevice
.org 0x0275eba8
  b get_decompressed_szs_size
.org @NextFreeSpace
.global get_decompressed_szs_size
get_decompressed_szs_size:
  lwz r4, 0(r3)
  lis r5, 0x5961
  addi r5, r5, 0x7a30
  cmplw r4, r5
  beq read_yaz0_size
  lis r5, 0x5341
  addi r5, r5, 0x5243
  cmplw r4, r5
  beq read_sarc_size
  li r3, -1
  b 0x0275ebac
  
read_yaz0_size:
  lwz r3, 0x4(r3)
  b 0x0275ebac
read_sarc_size:
  lwz r3, 0x8(r3)
  b 0x0275ebac

.org 0x0275f180
  b get_alignment
.org @NextFreeSpace
.global get_alignment
get_alignment:
  lwz r4, 0(r3)
  lis r5, 0x5961
  addi r5, r5, 0x7a30
  cmplw r4, r5
  beq read_yaz0_align
  li r3, 0
  b 0x0275ebac
  
read_yaz0_align:
  lwz r3, 0x8(r3)
  b 0x0275ebac

 ; in sead::SZSDecompressor::readHeader_
.org 0x0275edfc
  b copy_sarc_to_output
.org @NextFreeSpace
.global copy_sarc_to_output
copy_sarc_to_output:
  lwz r11, 0(r4)
  lis r7, 0x5341
  addi r7, r7, 0x5243
  cmplw r11, r7
  bne not_sarc ; not SARC
  lwz r3, 0x0(r12) ; load dst, src already in r4
  lwz r5, 0x8(r4) ; load size
  li r6, 0
  mflr r0
  .byte 0x49, 0x8C, 0xAE, 0x09 ; OSBlockMove, not sure the proper way to do the rpl call, manually done with a relocation (these bytes are a random absolute branch so Cemu loads it properly)
  li r3, -0x2
  mtlr r0
  blr ; return back to streamDecomp
not_sarc:
  b 0x0275ee44

 ; in sead::SZSDecompressor::tryDecompFromDevice
.org 0x0275f508
  b check_copied_dst
.org @NextFreeSpace
.global check_copied_dst
check_copied_dst:
  bge continue_normally
  cmpwi r3, -0x2
  beq copied_success
  b 0x0275f484 ; return as error
continue_normally:
  b 0x0275f50c ; return as normal
copied_success:
  b 0x0275f528 ; return back as success

.close
