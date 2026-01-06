; Initial hook in function ALLdie_execute
.org 0x02048430
  b check_stage_name

.org @NextFreeSpace
.global required_bosses
required_bosses:
  .space 6, 0xFF ; Allocate space for dungeon scene indexes
  .byte 0xFF
  .align 2 ; Align to the next 4 bytes

.global staircase_to_puppet_ganon_stage_name
staircase_to_puppet_ganon_stage_name:
  .string "GanonL"
  .align 2 ; Align to the next 4 bytes

.global check_stage_name
check_stage_name:
; If the enemy pointer is not null, jump to the end of the original function
  cmpwi r3, 0 
  bne jump_back_to_all_die_end
; Check if we're currently on the staircase before Puppet Ganon stage
  bl FUN_025200d4
  addi r3, r3, 0x5133
  lis r4, staircase_to_puppet_ganon_stage_name@ha
  addi r4, r4, staircase_to_puppet_ganon_stage_name@l
  subi r4, r4, 1
strcmp_start:
  lbzu r5, 0x1(r3)
  lbzu r6, 0x1(r4)
  cmplw r5, r6
  bne strncmp_end
  cmplwi r5, 0
  bne strcmp_start
strncmp_end:
  subf. r3, r5, r6

; If we aren't on the stage, jump back to the original function where mState and mTimer get set
  cmpwi r3, 0
  bne jump_back_to_all_die_success

; If we are on the stage, then check for each required boss being defeated.
  bl check_required_bosses_defeated
  cmpwi r3, 1
  beq jump_back_to_all_die_success

; These addresses is too far away for a conditional branch instruction
jump_back_to_all_die_end:
  b 0x02048448
jump_back_to_all_die_success:
  b 0x02048438


; This function checks to see if all required bosses have been beaten. Returns 1 if all defeated and 0 if not all defeated
.global check_required_bosses_defeated
check_required_bosses_defeated:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  stw r31, 0xC (sp)

  lis r31, required_bosses@ha
  addi r31, r31, required_bosses@l
  lbz r3, 0 (r31)
  b check_required_bosses_check_continue_loop

check_required_bosses_begin_loop:
  li r4, 3 ; bit for boss being defeated
  bl generic_is_dungeon_bit
  cmpwi r3, 0 ; If the bit for the boss isn't set, jump to the end with 0 stored in r3
  beq end_check_required_bosses_defeated
  lbzu r3, 1(r31)
check_required_bosses_check_continue_loop:
  cmplwi r3, 0xFF ; check for terminator
  bne+ check_required_bosses_begin_loop
  li r3, 1

end_check_required_bosses_defeated:
  lwz r31, 0xC (sp)
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr