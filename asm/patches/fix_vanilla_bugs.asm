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
.global beedle_dont_buy_blue_chu ; REPLACE ble INSTRUCTION AT 0X02215864
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
	b 0x0244A6b8

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

.close
