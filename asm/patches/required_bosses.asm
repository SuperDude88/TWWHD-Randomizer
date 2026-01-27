; Initial hook in function daALLdie_Create
.org 0x02048608
  sth r12, 0x320(r31) ; swap sth and li instruction to not worry about registers
  b check_required_bosses

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

.global check_required_bosses
check_required_bosses:
  bl check_and_set_required_bosses_defeated
  li r3, 4 ; Instruction we overwrote to jump here
  b 0x02048610 ; jump back to daALLdie_Create

; This function checks to see if we're on the final staircase stage and if all required bosses have been beaten.
; If both of these conditions are true, then the bit for having defeated all bosses is set.
.global check_and_set_required_bosses_defeated
check_and_set_required_bosses_defeated:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  stw r31, 0xC (sp)

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

; If we aren't on the stage, return early
  cmpwi r3, 0
  bne end_check_and_set_required_bosses_defeated

; Begin checking defeated flags for required bosses
  lis r31, required_bosses@ha
  addi r31, r31, required_bosses@l
  lbz r3, 0 (r31)
  b check_required_bosses_check_continue_loop

check_required_bosses_begin_loop:
  li r4, 3 ; bit for boss being defeated
  bl generic_is_dungeon_bit
  cmpwi r3, 0 ; If the bit for the boss isn't set, jump to the end
  beq end_check_and_set_required_bosses_defeated
  lbzu r3, 1(r31)
check_required_bosses_check_continue_loop:
  cmplwi r3, 0xFF ; check for terminator
  bne+ check_required_bosses_begin_loop

; If all required bosses are defeated, set the associated switch flag (0x25)
  lis r3, 0x1020
  lwz r3, -0x7b24(r3)
  addi r3, r3, 0x20 ; SaveData->mCurInfo
  li r4, 0x25 ; flag
  li r5, 0 ; current room number (which is 0)
  bl onSwitch

end_check_and_set_required_bosses_defeated:
  lwz r31, 0xC (sp)
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr