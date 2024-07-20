.org 0x02176D58
	b hookshot_sight_failsafe_check

.org @NextFreeSpace
.global hookshot_sight_failsafe_check
hookshot_sight_failsafe_check:
  cmplwi r30, 0
  beq hookshot_sight_failsafe
  b hookshot_sight_return
  
  ; If r30 is null skip to the code that hides the hookshot sight.
  hookshot_sight_failsafe:
  li r0,0x0
  b 0x02176d68
  
  ; Otherwise we replace the line of code at 02176d58 we replaced to jump here, then jump back.
  hookshot_sight_return:
  lwz r10,0x2e0(r30)
  b 0x02176d5c



.org 0x0225Af14
	b orca_counter_failsafe

.org @NextFreeSpace
.global orca_counter_failsafe
orca_counter_failsafe:
  cmpwi r12, 0x38 ; Hero's Sword
  bne not_heros_sword
  b 0x0225af1c ; Use Hero's Sword icon for the counter (icon 1)
  not_heros_sword:
  cmpwi r12, 0xFF ; No sword
  bne master_sword
  b 0x0225afb4 ; Skip past the code to create the counter entirely
  master_sword:
  b 0x0225af80 ; Use Master Sword icon for the counter (icon 2)



.org 0x02215864
	b beedle_dont_buy_blue_chu

.org @NextFreeSpace
.global beedle_dont_buy_blue_chu
beedle_dont_buy_blue_chu:
  bne beedle_not_blue_chu_jelly
  li r31, 0xF75
  b 0x02215D80

  beedle_not_blue_chu_jelly:
  blt less_than_val
  b 0x02215868

  less_than_val:
  b 0x02215898



.org 0x0249421c
	bl stalfos_kill_lower_body_when_upper_body_light_arrowed

.org @NextFreeSpace
.global stalfos_kill_lower_body_when_upper_body_light_arrowed
stalfos_kill_lower_body_when_upper_body_light_arrowed:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  bl fopAcIt_Judge ; Get upper body entity
  cmplwi r3, 0
  beq stalfos_kill_lower_body_when_upper_body_light_arrowed_end
  
  lbz r0, 0x21F6 (r3) ; Counter for how many frames the upper body has been dying to light arrows
  cmpwi r0, 0
  beq stalfos_kill_lower_body_when_upper_body_light_arrowed_end ; The upper body hasn't been hit with light arrows, so don't kill the lower body either
  
  lbz r0, 0x21F6 (r31) ; Counter for how many frames the lower body has been dying to light arrows
  cmpwi r0, 0
  bne stalfos_kill_lower_body_when_upper_body_light_arrowed_end ; The lower body is already dying to light arrows, so don't reset its counter
  
  li r0, 1
  stb r0, 0x21F6 (r31) ; Start the lower body's counter for dying to light arrows at 1
  
  stalfos_kill_lower_body_when_upper_body_light_arrowed_end:
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr



.org 0x0244706C
	bl miniblin_set_death_switch_when_light_arrowed

.org @NextFreeSpace
.global miniblin_set_death_switch_when_light_arrowed
miniblin_set_death_switch_when_light_arrowed:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  bl dCcD_Sph_Set ; Replace the function call we overwrote to call this custom function
  
  lbz r0, 0x3d0 (r29) ; Read the behavior type param for the Miniblin
  cmpwi r0, 0 ; Behavior type 0 is a respawning Miniblin
  beq miniblin_set_death_switch_when_light_arrowed_end ; Respawning Miniblins should not set a switch when they die, so don't do anything
  
  ; Otherwise it's a single Miniblin, so it should set a switch when it dies.
  lbz r0, 0x3d4 (r29) ; Read the switch index param the non-respawning Miniblin should set on death
  stb r0, 0xaad (r29) ; Store it into the Miniblin's enemyice struct as the switch index it should set when it dies to Light Arrows.
  ; Note: The enemy_ice function does not set the switch specified here in the case that it's switch index 0, but the Miniblin itself would even for index 0. This doesn't matter in practice because no Miniblins placed in the game are supposed to set switch index 0 on death.
  
  miniblin_set_death_switch_when_light_arrowed_end:
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr



.org 0x0244A694
	b 0x0244A69C

.org 0x0244A6b8
	bl poe_fix_light_arrows_bug

.org @NextFreeSpace
.global poe_fix_light_arrows_bug
poe_fix_light_arrows_bug:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  lbz r0, 0x3a1 (r31) ; Read the Poe's current HP
  extsb. r0,r0
  ble poe_fix_light_arrows_bug_poe_is_dead ; Consider the Poe dead if its HP is <= 0
  
  lbz r0, 0x9a6 (r31) ; Read the Poe's dying to light arrows counter
  cmpwi r0, 0
  bgt poe_fix_light_arrows_bug_poe_is_dead ; Consider the Poe dead if it was hit with light arrows, even if its HP isn't 0 yet
  
  b poe_fix_light_arrows_bug_return_false ; Otherwise consider the Poe alive
  
  poe_fix_light_arrows_bug_poe_is_dead:
  bl fopAcM_SearchByID ; Replace the function call we overwrote to call this custom function
  
  ; Then we need to reproduce most of the rest of the original Big_pow_down_check function.
  ; The reason for this is a weird quirk Poes in the Jalhalla fight have where if they're killed in the last 4 frames before Jalhalla reforms, they will "unkill" themselves so they can join back up with Jalhalla.
  ; We need to unset the dying to light arrows counter in that case as well.
  
  cmpwi r3, 0
  beq poe_fix_light_arrows_bug_return_false
  lwz r12, 0x18 (sp)
  cmplwi r12, 0
  beq poe_fix_light_arrows_bug_return_false
  cmplwi r12, 0
  beq poe_fix_light_arrows_bug_return_false
  lha r11, 8 (r12)
  cmpwi r11, 0xD3 ; Check to be sure the supposed Jalhalla entity is actually an instance of bpw_class (might be different in HD)
  bne poe_fix_light_arrows_bug_return_false
  lha r9,0x562(r12)
  cmpwi r9,0x6f ; Check Jalhalla's state or something, 0x6F is for when the child Poes are running around
  bne poe_fix_light_arrows_bug_unkill_poe
  lha r0,0x56a(r12) ; Read number of frames left until Jalhalla reforms
  cmpwi r0,0x3 ; Poes killed within the last 4 frames before Jalhalla reforms shouldn't actually die
  ble poe_fix_light_arrows_bug_unkill_poe
  lbz r11,0x3a1(r12)
  subi r11,r11,0x1 ; Decrement Jalhalla's HP
  stb r11,0x3a1(r12)
  lwz r9,0x18(sp) ; Read Jalhalla entity pointer again
  lbz r10,0x3a1(r9) ; Check if Jalhalla's HP is zero, meaning this Poe that just died was the last one
  extsb. r10,r10
  bgt poe_fix_light_arrows_bug_not_the_last_poe
  li r0, 1
  stb r0,0x460(r31)
  poe_fix_light_arrows_bug_not_the_last_poe:
  li r0, 1
  li r3, 0
  stb r0,0x461(r31)
  b poe_fix_light_arrows_bug_return_false
  
  poe_fix_light_arrows_bug_unkill_poe:
  li r9,0x4
  stb r9,0x3a1(r31)
  
  ; These 2 lines are the new code:
  li r9, 0
  stb r9, 0x9a6 (r31) ; Set the Poe's dying to light arrows counter to zero to stop it from dying
  
  li r3, 1
  b poe_fix_light_arrows_bug_end
  
  poe_fix_light_arrows_bug_return_false:
  li r3, 0
  
  poe_fix_light_arrows_bug_end:
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.org 0x0244A6BC
	b 0x0244A754



.org 0x021D90D0
	bl magtail_respawn_when_head_light_arrowed

.org @NextFreeSpace
.global magtail_respawn_when_head_light_arrowed
magtail_respawn_when_head_light_arrowed:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  bl GetTgHitObj ; Replace the function call we overwrote to call this custom function
  
  ; Then we need to reproduce a few lines of code from the original function.
  addi r9,r31,0x1990
  stw r3, 0xd8 (sp) 
  stw r9, 0xec (sp) 
  lwz r5, 0x10 (r3) ; Read the bitfield of damage types done by the actor that just damaged this Magtail
  rlwinm. r4,r5,0x0,0xb,0xb ; Check the Light Arrows damage type
  
  ; Then if the Light Arrows bit was set, we store true to magtail_entity+0x1CBC to signify that the Magtail should respawn.
  ; (We can't use the original branch on the Light Arrows bit because it's inside the REL.)
  ; Check how beq works and find addresses for the rest of this patch
  beq magtail_respawn_when_head_light_arrowed_end
  li r0, 1
  stb r0, 0x1dd4 (r31)
  
  magtail_respawn_when_head_light_arrowed_end:
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.org 0x021D90D4
	nop
	nop
	nop
	nop
	nop



.org 0x02139B7C
	b phantom_ganon_check_link_within_y_diff

.org @NextFreeSpace
.global phantom_ganon_check_link_within_y_diff
phantom_ganon_check_link_within_y_diff:
  lfs f10, 0x24 (sp) ; Read the Y difference between Link and Phantom Ganon
  lfs f1, y_diff_float@l(r9) ; load from 0x1000efe8
  
  ; If the Link is 1000.0 units or more higher than PG, do not trigger the fight.
  ; (Does not account for negative difference. Still extends infinitely downwards.)
  fcmpo cr0, f10, f1
  bge phantom_ganon_check_link_within_y_diff_outside_range
  
  ; Otherwise, go on to check the X and Z difference as usual.
  fmuls f9,f11,f11 ; Replace the line of code we overwrote to jump here
  b 0x02139b80

phantom_ganon_check_link_within_y_diff_outside_range:
  b 0x02139e4c

.org 0x025b0aa0
  b stop_sub_bgm_when_unloading_stage

.org @NextFreeSpace
.global stop_sub_bgm_when_unloading_stage
stop_sub_bgm_when_unloading_stage:
  ; Stop the music
  bl subBgmStop_load_data

  bl FUN_025200d4 ; replace the line we overwrote to jump here

  b 0x025b0aa4 ; return
  

; Zero out the arrow actor's on-hit callback function when it enters the stopped state.
; This is to fix a vanilla crash that could happen if the arrow hit two different actors at the same time.
; The arrow actor keeps track of both the proc ID of the actor it hit and which joint index within that actor it hit.
; The joint index variable is only updating while the arrow is moving, while the proc ID is updated by the callback function.
; If the arrow hit something with more joints first (e.g. Big Octo) and then something with fewer joints (e.g. Big Octo eye), the joint index could wind up higher than the size of the joints array for the second actor.
; So when the actor tries to stop on that joint, it would wind up copying invalid joint data as matrix data.
; Invalid data can sometimes be NaN floats, and storing those as the arrow actor's position would cause an assertion error as positions are supposed to be valid numbers.
; Zeroing out the on-hit callback fixes the crash as the proc ID will no longer be desynced from the joint index.
.org 0x02054658
  b zero_out_arrow_on_hit_callback
.org @NextFreeSpace
.global zero_out_arrow_on_hit_callback
zero_out_arrow_on_hit_callback:
  ; Store 0 to the arrow actor's on-hit callback (atHit_CB).
  ; Specifically this is the dCcD_GObjAt.mpCallback field of the arrow's hitbox.
  li r0, 0
  stw r0, 0x510 (r30)
  
  ; Replace the line we overwrote to jump here (preparing to update the arrow's state to 2, stopped).
  li r0, 2
  
  ; Return
  b 0x0205465C



; Do not prevent the player from defending with the Skull Hammer when they don't own a shield.
; Originally, the game only checked if you own a shield to know if it should allow you to defend.
; Change it to allow defending if you own a shield, or are holding the Skull Hammer in your hands.
.org 0x023f0a1c ; In daPy_lk_c::checkNextActionFromButton(void)
  bl check_can_defend
  cmpwi r3, 0

.org 0x023fb970 ; In daPy_lk_c::setShieldGuard(void)
  bl check_can_defend
  cmpwi r3, 0

.org @NextFreeSpace
; r31 - pointer to the current daPy_lk_c Link player instance
.global check_can_defend
check_can_defend:
  lhz r3, 0x69B0(r31) ; What item the player is holding in their hand
  cmplwi r3, 0x33 # Skull Hammer
  beq check_can_defend_return_true ; Always allow defending if holding the Skull Hammer
  
  lis r3, gameInfo_ptr@ha
  lwz r3, gameInfo_ptr@l(r3)
  lbz r3, 0x2F(r3) ; Currently equipped shield ID
  cmplwi r3, 0xFF ; No shield equipped
  bne check_can_defend_return_true ; Also allow defending if you own a shield
  
  ; Otherwise, don't allow defending.
  check_can_defend_return_false:
  li r3, 0
  blr
  
  check_can_defend_return_true:
  li r3, 1
  blr



; If Zelda aims in the right spot, you can sometimes reflect her light arrow in phase 1 of the Ganondorf fight
; This still plays the camera animation that normally happens in phase 3
; If this happens at the end of phase 1, the game also tries to play the cutscene of knocking Zelda down, and she will be frozen briefly when she wakes up for phase 3
; Don't try to play this cutscene until his health is <=25 (75 -> phase 1, 50 -> phase 2, 25 -> phase 3)
.org 0x02053BB4 ; in daArrow_c::ShieldReflect
  b ganondorf_health_check
.org @NextFreeSpace
.global ganondorf_health_check
ganondorf_health_check:
  lfs f9, 0x314(r12) ; replace the line we overwrote to jump here
  lbz r10, 0x3A1(r12) ; load his current health
  cmpwi r10, 0x19
  bgt skip_reflect_cutscene
  b 0x02053BB8; continue with cutscene animation
skip_reflect_cutscene:
  b 0x02053C54 ; continue as if we weren't targeting Ganondorf (no cutscene)


; If Ganondorf gets hit by enough light arrows in phase 3, his health will go down to 0 and he will play a different hit sound
; This is normally used for the final hit on enemies so it sounds a bit strange (0x02519494 plays the sound)
.org 0x02519478 ; Runs when enemy is "dead" (based on health), in d_cc_uty::cc_at_check
  b check_if_ganondorf
.org @NextFreeSpace
.global check_if_ganondorf
check_if_ganondorf:
  lhz r3, 0x8(r27) ; Load mProcName (actor ID)
  cmplwi r3, 0xF6 ; Ganondorf actor ID
  beq force_normal_hit_sound
  lbz r3, 0x326(r27) ; Replace the line we woverwrote to jump here
  b 0x0251947C ; Continue with <= 0 health sound
force_normal_hit_sound:
  li r29, 0x20 ; Skip some checks that would realize health is 0, load this parameter directly instead
  b 0x02519700 ; Continue as if he isn't "dead"



; You can sometimes land directly in the Wind Temple warp after defeating Molgera
; This forces you to go all the way back through the dungeon to collect the item which is slow and annoying
; Add a check to prevent this
.org 0x02102ED8 ; where Molgera sets a target position for Link during the cutscene
   b check_if_inside_warp

.org @NextFreeSpace
.global molgera_warp_radius
molgera_warp_radius:
  .float 450.0
.global default_molgera_xz_pos
default_molgera_xz_pos:
  .float 0.0
  .float 777.0

.global check_if_inside_warp
check_if_inside_warp:
  stfs f11, 0x8(r25) ; replace instruction we overwrote to jump here

  ; Warp is centered at 0,0 (X and Z) so we don't need to do any subtraction
  lfs f1, 0x0(r25) ; target X position
  lfs f2, 0x8(r25) ; target Z position
  fmuls f1, f1, f1
  fmuls f2, f2, f2
  fadds f1, f1, f2 ; X^2 + Z^2
  bl sqrtf ; f1 is argument and return value for this
  lis r3, molgera_warp_radius@ha
  lfs f2, molgera_warp_radius@l(r3)
  fcmpo cr0, f1, f2
  bgt link_not_inside_warp

  ; overwrite the calculated X and Z positions with our own
  lis r3, default_molgera_xz_pos@ha
  addi r3, r3, default_molgera_xz_pos@l
  lfs f1, 0x0(r3)
  stfs f1, 0x0(r25) ; X position
  lfs f1, 0x4(r3)
  stfs f1, 0x8(r25) ; Z position

  ; Y position is handled later in the original function

link_not_inside_warp:
  b 0x02102EDC



; In vanilla, the barrier breaking cutscene will crash if you don't have 8 triforce shards because the layer is wrong
; This becomes an issue if decoupled entrance randomizer gives you access to Hyrule before you get all the shards
; Add the triforce requirement to the barrier itself to prevent the crash
.org 0x023238E4
  b check_barrier_triforce_requirement
.org @NextFreeSpace
.global check_barrier_triforce_requirement
check_barrier_triforce_requirement:
  lis r3, gameInfo_ptr@ha
  lwz r3,gameInfo_ptr@l(r3)
  addi r3, r3, 0xD4
  bl getTriforceNum

  cmpwi r3, 8
  blt barrier_does_not_have_triforce

  bl FUN_025200d4 ; replace the line we overwrote to jump here
  b 0x023238E8 ; continue checking the other conditions

barrier_does_not_have_triforce:
  b 0x0232393C ; continue without breaking the barrier



; In WWHD, going through a load trigger with a morth stuck to you causes a bug known as "morth glitch"
; Morth glitch makes it so that rolling no longer gets morths off of Link, and you need to bonk into a wall or spin attack the morths instead
; Going through more load triggers can lead to "advanced morth glitch," where morths do not grab onto Link at all

; The morth actor uses several global counters, including KS_ALL_COUNT (total number of morths) and KUTTUKU_ALL_COUNT (number of morths stuck to Link)
; When morths are created, the KS_ALL_COUNT variable is only incremented under certain circumstances, and it does not occur for all morths
; For all morths, however, the delete function still decrements the counter, causing KS_ALL_COUNT to become a progressively more negative number
; The delete function also checks if KS_ALL_COUNT is exactly equal to zero, and if it is, resets KUTTUKU_ALL_COUNT
; Because KS_ALL_COUNT is negative, KUTTUKU_ALL_COUNT is never reset to 0
; When a morth grabs onto Link, it checks if KUTTUKU_ALL_COUNT is zero, and sets some instance variable to true if it is
; This seems to allow Link to roll the morth off, but because KUTTUKU_ALL_COUNT isn't reset to 0, this flag stops getting set properly
; In SD, the morth's REL is reloaded after each loading trigger (which happens to reset the counter), but HD does not reload the code so the issue manifests
.org 0x021A9F04 ; in daKS_Delete
  b check_should_decrement_morth_counter
.org @NextFreeSpace
.global check_should_decrement_morth_counter
check_should_decrement_morth_counter:
  lbz r3, 0xD(r31) ; Get the create status of the morth
  cmpwi r3, 0x2 ; Status 0x2 is cPhs_NEXT_e (set for morths that are fully created, no errors)
  beq decrement_morth_counter

  ; Morth was not fully created, skip decrementing
  b 0x021A9F30

  ; Morth was fully created
decrement_morth_counter:
  subic. r11, r11, 0x1
  b 0x021A9F08



; The d-pad indicator in the top left assumes you get Wind Waker -> Grappling Hook -> Bombs in order and displays the icons based on that
; If you get the items out of order, it will also show the previous ones even if you do not have them (it's only visual but still wrong)

; Change how the item flags are set to fix this
; r0 is reset to be 0 at the start of the function
.org 0x026DD5C8 ; when you have bombs
  ori r0, r0, 0b100
  nop ; don't store flags/return yet
  nop
.org 0x026DD5E0 ; when you have the grappling hook
  ori r0, r0, 0b10
  nop ; don't store flags/return yet
  nop
.org 0x026DD5F4 ; when checking for the wind waker
  bne 0x026DD5FC ; always store the flags even if we don't have the wind waker
  ori r0, r0, 0b1

; Change how the item flags are read when the icons are appearing
.org 0x026DD704 ; when checking for the grappling hook
  andi. r4, r11, 0b10 ; mask out the flag for grapple (r4 is overwritten right after, just a throwaway while we set condition register)
.org 0x026DD70C ; when branching based on the grapple flag
  beq 0x026DD714
.org 0x026DD724 ; when checking for bombs
  andi. r4, r0, 0b100 ; mask out the flag for bombs (r4 is overwritten right after, just a throwaway while we set condition register)
.org 0x026DD72C ; when branching based on the bombs flag
  beq 0x026DD734

; Change the default/invalid state
.org 0x026DDA38
  li r11, 0b1000

; Change how the show/hide animations are played based on the flags
; This won't handle the case where we don't have wind waker, but that isnt applicable to rando yet anyways
; TODO: modify this to work without having the wind waker (may require modifying the layout animations)
; Old flags are in r0, current flags are in r12
.org 0x026DD8B8
  stw r11, 0x60(r31) ; replace a line we will overwrite
  xor r0, r0, r12 ; get the flags that changed
  andi. r11, r0, 0b10 ; check if the grapple flag changed
  beq dpad_check_bombs_flags ; flag not changed
  andi. r11, r12, 0b10 ; check if the current flags have grapple
  li r11, 0x1 ; animation to show grapple
  bne dpad_store_grapple_anim
  li r11, 0x3 ; animation to hide grapple
dpad_store_grapple_anim:
  stw r11, 0x5C(r31) ; store to the grapple animation

dpad_check_bombs_flags:
  andi. r11, r0, 0b100 ; check if the bombs flag changed
  beq dpad_play_item_animations ; flag not changed
  andi. r11, r12, 0b100 ; check if the current flags have bombs
  li r11, 0x2 ; animation to show bombs
  bne dpad_store_bombs_anim
  li r11, 0x4 ; animation to hide bombs
dpad_store_bombs_anim:
  stw r11, 0x60(r31) ; store to the bombs animation

dpad_play_item_animations:
  b 0x026DD938
