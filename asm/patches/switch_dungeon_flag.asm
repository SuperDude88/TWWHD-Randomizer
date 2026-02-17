; Doing this in ASM because properly setting up a C -> ASM patch script would take a while
; And I somehow convinced myself (again) that adapting ASM would take less time
; Based on https://github.com/LagoLunatic/wwrando/blob/master/asm/actors/d_a_dungeon_flag_sw.cpp

.org @NextFreeSpace
.global daDungeonFlagSw_Create
daDungeonFlagSw_Create:
    stwu sp, -0x10(sp)
    mflr r0
    stw r0, 0x14(sp)
    stw r31, 0xC(sp)
    mr r31, r3
    lwz r9, 0x2E4(r31)
    rlwinm. r9, r9, 0, 0x1C, 0x1C
    bne custom_construct_code
    bl fopAc_ac_c_ct
    ; We *should* override our vtable here with a virtual destructor but I don't think we need to
    lwz r9, 0x2E4(r31)
    ori r9, r9, 0x8
    stw r9, 0x2E4(r31)
custom_construct_code:
    lwz r9, 0xB0(r31) ; Load parameters
    rlwinm r4, r9, 0, 0x18, 0x1F ; params & 0x000000FF
    lbz r5, 0x326(r31) ; current room number
    lis r3, gameInfo_ptr@ha
    lwz r3, gameInfo_ptr@l(r3)
    addi r3, r3, 0x20
    bl isSwitch
    cmpwi r3, 0
    beq switch_not_set
    li r3, 3 ; cPhs_STOP_e
    b create_finish
switch_not_set:
    li r3, 4 ; cPhs_COMPLEATE_e
create_finish:
    lwz r0, 0x14(sp)
    mtlr r0
    lwz r31, 0xC(sp)
    addi sp, sp, 0x10
    blr

.global daDungeonFlagSw_IsDelete
daDungeonFlagSw_IsDelete:
    li r3, 0x1
    blr

.global daDungeonFlagSw_Delete
daDungeonFlagSw_Delete:
    li r3, 0x1
    blr

.global daDungeonFlagSw_Draw
daDungeonFlagSw_Draw:
    li r3, 0x1
    blr

.global daDungeonFlagSw_checkConditionsMet
daDungeonFlagSw_checkConditionsMet:
    stwu sp, -0x10(sp)
    mflr r0
    stw r0, 0x14(sp)
    stw r31, 0xC(sp)
    mr r31, r3

    lwz r9, 0xB0(r3) ; Load parameters
    rlwinm r6, r9, 0, 0x08, 0x18 ; params & 0x00FFFF00
    srawi r6, r6, 0x08 ; r6 >> 0x08 (stage number bitset)
    rlwinm r7, r9, 0, 0x05, 0x07 ; params & 0x07000000
    srawi r7, r7, 0x18 ; r7 >> 0x18 (which dungeon condition to check)

    li r8, 0 ; Stage ID
dungeon_flag_check_loop_start:
    ; Test if we are checking this stage ID
    li r9, 1
    slw r9, r9, r8 ; 1 << r8
    and. r9, r9, r6 ; r9 & r6
    beq dungeon_flag_check_loop_inc

    lis r3, gameInfo_ptr@ha
    lwz r3, gameInfo_ptr@l(r3)
    addi r3, r3, 0x3A0 ; First entry in gameinfo->mSave.mMemory
    mulli r4, r8, 0x24 ; Stage ID * sizeof(dSv_memory_c)
    add r3, r3, r4 ; Index into gameinfo->mSave.mMemory
    mr r4, r7 ; Condition to check
    bl isDungeonItem
    cmpwi r3, 0
    beq conditions_not_met
dungeon_flag_check_loop_inc:
    addi r8, r8, 1 ; increment stage number
    cmpwi r8, 0x10 ; 0x10 possible stage IDs
    blt dungeon_flag_check_loop_start

    li r3, 1
    b checkConditionsMet_return
conditions_not_met:
    li r3, 0
checkConditionsMet_return:
    lwz r0, 0x14(sp)
    mtlr r0
    lwz r31, 0xC(sp)
    addi sp, sp, 0x10
    blr

.global daDungeonFlagSw_Execute
daDungeonFlagSw_Execute:
    stwu sp, -0x10(sp)
    mflr r0
    stw r0, 0x14(sp)
    stw r31, 0xC(sp)
    mr r31, r3

    bl daDungeonFlagSw_checkConditionsMet
    cmpwi r3, 0
    beq return_execute

    lwz r4, 0xB0(r31) ; Load parameters
    rlwinm r4, r4, 0, 0x18, 0x1F ; params & 0x000000FF
    lbz r5, 0x326(r31) ; current room number
    lis r3, gameInfo_ptr@ha
    lwz r3, gameInfo_ptr@l(r3)
    addi r3, r3, 0x20
    bl onSwitch
    mr r3, r31
    bl fopAcM_delete

return_execute:
    li r3, 1
    lwz r0, 0x14(sp)
    mtlr r0
    lwz r31, 0xC(sp)
    addi sp, sp, 0x10
    blr

.global l_daDungeonFlagSw_Method
l_daDungeonFlagSw_Method:
    .long daDungeonFlagSw_Create
    .long daDungeonFlagSw_Delete
    .long daDungeonFlagSw_Execute
    .long daDungeonFlagSw_IsDelete
    .long daDungeonFlagSw_Draw


.global g_profile_DUNGEON_FLAG_SW
g_profile_DUNGEON_FLAG_SW:
    .int -3 ; mLayerID
    .short 7 ; mListID
    .short -3 ; mListPrio
    .short 0x1E5 ; Actor ID
    .byte 0
    .byte 0
    .long g_fpcLf_Method
    .int 0x3AC ; mSize
    .int 0
    .int 0
    .long g_fopAc_Method
    .short 0x0134 ; mDrawPriority
    .byte 0
    .byte 0
    .long l_daDungeonFlagSw_Method
    .int 0x00044000 ; mStatus
    .byte 0 ; mActorType
    .byte 6 ; mCullType
    .byte 0
    .byte 0
