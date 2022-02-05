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
;	.org 0x02600634
;		b check_run_new_text_commands
; Check the ID of the upcoming text command to see if it's a custom one, and runs custom code for it if so.
;	.org @NextFreeSpace
;	.global check_run_new_text_commands
;	check_run_new_text_commands:
;		cmplwi r0,0x7
;		bne check_run_new_text_commands_check_failed
;	
;		lhz r0, 0x4(r6)
;		cmplwi r0, 0x4B
;		blt check_run_new_text_commands_check_failed
;		cmplwi r0, 0x4F
;		bgt check_run_new_text_commands_check_failed
;		b exec_curr_num_keys_text_command
;	
;	check_run_new_text_commands_check_failed:
; 		i r3, -0x1
;		b 0x02600638 ; jump back


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

.close
