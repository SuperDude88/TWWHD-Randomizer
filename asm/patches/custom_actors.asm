.org @NextFreeSpace
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

  .byte 'D' ; Actor name
  .byte 'n'
  .byte 'g'
  .byte 'S'
  .byte 'w'
  .byte 0
  .byte 0
  .byte 0
  .short 0x01E5 ; Actor ID
  .byte 0xFF ; Subtype
  .byte 0 ; GBA Name

.org @NextFreeSpace
.global custom_profile_list
custom_profile_list:
  .long g_profile_SwitchOperator
  .long g_profile_DUNGEON_FLAG_SW
  .long 0

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
  cmplwi r10, 0x329 + 2 ; Num entries in vanilla list + custom list
  bge read_custom_l_objectName_loop_for_dStage_searchName_end_loop
  b continue_normal_search_loop

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
  li r0, 2 ; Num entries in our custom list
  mtctr r0
  b 0x025C10FC ; Return to the start of the vanilla loop

has_actor_with_same_proc_name:
  b 0x025C1144
read_custom_l_objectName_loop_for_dStage_getName_end_loop:
  b 0x025C1148 ; Return to after the end of the loop



; Check if we need to load this actor's profile from our custom list
; In fpcPf_Get
.org 0x025E10E4
  b check_load_custom_actor_profile

.org @NextFreeSpace
.global check_load_custom_actor_profile
check_load_custom_actor_profile:
  cmplwi r3, 0x1E4
  blt load_normal_actor_profile
  subi r3, r3, 0x1E4
  lis r0, custom_profile_list@ha
  addic r0, r0, custom_profile_list@l
  slwi r12, r3, 0x2 ; Get offset in custom list
  b 0x025E10EC ; Load profile pointer + return
load_normal_actor_profile:
  slwi r12, r3, 0x2 ; Replace the line we overwrote to jump here
  b 0x025E10E8 ; Load profile pointer + return
