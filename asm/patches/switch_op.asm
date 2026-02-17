; Doing this in ASM because properly setting up a C -> ASM patch script would take a while
; And I somehow convinced myself 400 lines of ASM would take less time

.org @NextFreeSpace
.global daSwOp_Create
daSwOp_Create:
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
    lwz r9, 0xB0(r3) ; Load parameters
    rlwinm r10, r9, 0, 0x1C, 0x1F ; params & 0x0000000F
	stb r10, 0x3AD(r3) ; mOperation
	rlwinm r10, r9, 0x1C, 0x1F, 0x1F ; (params & 0x00000010) >> 4
	stb r10, 0x3AE(r3) ; mContinuous
	srawi r10, r9, 0x8 ; params >> 8
	stb r10, 0x3AF(r3) ; mSwitchToSet
	srawi r10, r9, 0x10 ; params >> 0x10
	stb r10, 0x3B0(r3) ; mFirstSwitchToCheck
	srwi r9, r9, 0x18 ; params >> 0x18
	stb r9, 0x3B1(r3) ; mNumSwitchesToCheck
	lhz r5, 0x2F8(r3) ; X rotation 
	stb r5, 0x3B2(r3) ; mEVNTIndexToStart
	lhz r9, 0x2FC(r3) ; Z rotation
	stb r9, 0x3B6(r3) ; mTotalFramesToWait
	rlwinm r5, r5, 0, 0xFF ; X rotation & 0x00FF
	li r4, 0
	bl FUN_025200d4 ; get gameInfo pointer
	addi r3, r3, 0x52C4 ; (gameInfo->mPlay).mEventMgr
	bl getEventIdx
	sth r3, 0x3B4(r31) ; mEventIndexToStart
	lbz r9, 0x3B6(r31) ; load total frames to wait
	stb r9, 0x3B7(r31) ; mRemainingFramesToWait
	lbz r4, 0x3AF(r31) ; load switch to set
	cmpwi r4, 0xFF
	bne has_switch_to_set
switch_op_deactivated:
	li r9, 5 ; Deactivated state
	stb r9, 0x3AC(r31) ; mState
    b create_finish
has_switch_to_set:
    ; r4 still has the switch
    lbz r5, 0x326(r31) ; current room number
    lis r3, gameInfo_ptr@ha
    lwz r3, gameInfo_ptr@l(r3)
    addi r3, r3, 0x20
    bl isSwitch
    cmpwi r3, 0
    beq switch_not_set
    lbz r9, 0x3AE(r31) ; is continuous
    cmpwi r9, 0
    beq switch_op_deactivated
	li r9, 4 ; CheckConditionUnmet state
	stb r9, 0x3AC(r31) ; mState
    b create_finish
switch_not_set:
	li r9, 0 ; CheckConditionMet state
	stb r9, 0x3AC(r31) ; mState
create_finish:
    li r3, 4 ; cPhs_COMPLEATE_e
    lwz r0, 0x14(sp)
    mtlr r0
    lwz r31, 0xC(sp)
    addi sp, sp, 0x10
    blr

.global daSwOp_IsDelete
daSwOp_IsDelete:
    li r3, 0x1
    blr

.global daSwOp_Delete
daSwOp_Delete:
    li r3, 0x1
    blr

.global daSwOp_Draw
daSwOp_Draw:
    li r3, 0x1
    blr

.global daSwOp__eventEndCheck
daSwOp__eventEndCheck:
    stwu sp, -0x10(sp)
    mflr r0
    stw r0, 0x14(sp)
    stw r31, 0xC(sp)
    mr r31, r3
    lha r4, 0x3B4(r3) ; event index to start
	bl FUN_025200d4 ; get gameInfo pointer
	addi r3, r3, 0x52C4 ; (gameInfo->mPlay).mEventMgr
    bl endCheck
    cmpwi r3, 0
    beq return_event_end_check
    lbz r9, 0x3AE(r31) ; is continuous
    cmpwi r9, 0
    beq switch_not_continuous
	li r9, 4 ; CheckConditionUnmet state
	stb r9, 0x3AC(r31) ; mState
    b set_event_state_flag
switch_not_continuous:
    li r9, 0x5 ; Deactivated state
	stb r9, 0x3AC(r31) ; mState
set_event_state_flag:
	bl FUN_025200d4 ; get gameInfo pointer
	lhz r9, 0x52B8(r3) ; (gameInfo->mPlay).mEvtCtrl.mStateFlags
    ori r9, r9, 0x8 ; flags |= 8
    sth r9, 0x52B8(r3) ; store flags back
return_event_end_check:
    lwz r0, 0x14(sp)
    mtlr r0
    lwz r31, 0xC(sp)
    addi sp, sp, 0x10
    blr

.global daSwOp__isCondition
daSwOp__isCondition:
    stwu sp, -0x20(sp)
    mflr r0
    stw r0, 0x24(sp)
    stw r27, 0xC(sp)
    stw r28, 0x10(sp)
    stw r29, 0x14(sp)
    stw r30, 0x18(sp)
    stw r31, 0x1C(sp)
    mr r29, r3 ; this pointer
    lbz r31, 0x3B0(r3) ; switch to check
    li r30, 0 ; loop variable
    li r28, 0 ; numUnset
    li r27, 0 ; numSet
    b check_switches_loop_body
increment_num_unset:
    addi r28, r28, 1
increment_loop_counters:
    addi r31, r31, 1 ; current switch
    addi r30, r30, 1 ; increment loop
check_switches_loop_body:
    lbz r9, 0x3B1(r29) ; num switches to check
    cmpw r30, r9
    bge test_and_condition_met
    lbz r5, 0x326(r29) ; current room number
    rlwinm r4, r31, 0, 0xFF
    lis r3, gameInfo_ptr@ha
    lwz r3, gameInfo_ptr@l(r3)
    addi r3, r3, 0x20
    bl isSwitch
    cmpwi r3, 0
    beq increment_num_unset
    addi r27, r27, 1
    b increment_loop_counters
test_and_condition_met:
    li r3, 0 ; set return variable to false
	lbz r9, 0x3AD(r29) ; operation
    cmpwi r9, 0 ; AND op
    bne test_nand_condition_met
    cmpwi r28, 0
    beq set_condition_met
test_nand_condition_met:
    cmpwi r9, 1 ; NAND op
    bne test_or_condition_met
    cmpwi r28, 0
    bgt set_condition_met
test_or_condition_met:
    cmpwi r9, 2 ; OR op
    bne test_nor_condition_met
    cmpwi r27, 0
    bgt set_condition_met
test_nor_condition_met:
    cmpwi r9, 3 ; NOR op
    bne test_xor_condition_met
    cmpwi r27, 0
    beq set_condition_met
test_xor_condition_met:
    cmpwi r9, 4 ; XOR op
    bne test_xnor_condition_met
    cmpwi r27, 1
    beq set_condition_met
test_xnor_condition_met:
    cmpwi r9, 5 ; XNOR op
    bne return_is_condition
    cmpwi r27, 1
    bne set_condition_met
    b return_is_condition
set_condition_met:
    li r3, 1
return_is_condition:
    lwz r0, 0x24(sp)
    mtlr r0
    lwz r27, 0xC(sp)
    lwz r28, 0x10(sp)
    lwz r29, 0x14(sp)
    lwz r30, 0x18(sp)
    lwz r31, 0x1C(sp)
    addi sp, sp, 0x20
    blr

.global daSwOp__conditionMetCheck
daSwOp__conditionMetCheck:
    stwu sp, -0x10(sp)
    mflr r0
    stw r0, 0x14(sp)
    stw r31, 0xC(sp)
    mr r31, r3
    bl daSwOp__isCondition
    cmpwi r3, 0
    beq return_condition_met_check
    lbz r9, 0x3B6(r31)
    cmpwi r9, 0
    beq state_not_timer
    li r9, 1 ; Timer state
    stb r9, 0x3AC(r31)
    b return_condition_met_check
state_not_timer:
    lha r9, 0x3B4(r31) ; event index to start
    cmpwi r9, -0x1
    beq set_switch_directly
    li r9, 2 ; StartingEvent state
    stb r9, 0x3AC(r31)
    b return_condition_met_check
set_switch_directly:
    lbz r5, 0x326(r31) ; current room number
    lbz r4, 0x3AF(r31) ; switch to set
    lis r3, gameInfo_ptr@ha
    lwz r3, gameInfo_ptr@l(r3)
    addi r3, r3, 0x20
    bl onSwitch
    lbz r9, 0x3AE(r31) ; is continuous
    cmpwi r9, 0
    beq set_state_deactivated
    li r9, 4 ; CheckConditionUnmet state
    stb r9, 0x3AC(r31)
    b return_condition_met_check
set_state_deactivated:
    li r9, 4 ; Deactivated state
    stb r9, 0x3AC(r31)
return_condition_met_check:
    lwz r0, 0x14(sp)
    mtlr r0
    lwz r31, 0xC(sp)
    addi sp, sp, 0x10
    blr

.global daSwOp__timerCountdown
daSwOp__timerCountdown:
    stwu sp, -0x10(sp)
    mflr r0
    stw r0, 0x14(sp)
    stw r31, 0xC(sp)
    mr r31, r3
    lbz r9, 0x3AE(r3)
    cmpwi r9, 0
    beq check_remaining_frames
    bl daSwOp__isCondition
    cmpwi r3, 0
    bne check_remaining_frames ; branch if condition is met
	lbz r9, 0x3B6(r31) ; total frames to wait
	stb r9, 0x3B7(r31) ; reset remaining frames
    li r9, 0 ; CheckConditionMet state
	stb r9, 0x3AC(r31)
    b timer_countdown_return
check_remaining_frames:
    lbz r9, 0x3B7(r31) ; remaining frames
    cmplwi r9, 0
    bgt decrement_remaining_frames
    lha r9, 0x3B4(r31) ; event index to start
    cmpwi r9, -1
    beq set_timer_countdown_switch
    li r9, 2 ; StartingEvent state
	stb r9, 0x3AC(r31)
    b timer_countdown_return
set_timer_countdown_switch:
    lbz r5, 0x326(r31) ; current room number
    lbz r4, 0x3AF(r31) ; switch to set
    lis r3, gameInfo_ptr@ha
    lwz r3, gameInfo_ptr@l(r3)
    addi r3, r3, 0x20
    bl onSwitch
    lbz r9, 0x3AE(r31) ; is continuous
    cmpwi r9, 0
    beq timer_countdown_return
    li r9, 5 ; Deactivated state
	stb r9, 0x3AC(r31)
    b timer_countdown_return
decrement_remaining_frames:
    lbz r9, 0x3B7(r31) ; remaining frames
    addi r9, r9, -0x1 ; decrement
    stb r9, 0x3B7(r31)
timer_countdown_return:
    lwz r0, 0x14(sp)
    mtlr r0
    lwz r31, 0xC(sp)
    addi sp, sp, 0x10
    blr

.global daSwOp__eventStartCheck
daSwOp__eventStartCheck:
    stwu sp, -0x10(sp)
    mflr r0
    stw r0, 0x14(sp)
    stw r31, 0xC(sp)
    mr r31, r3
    lhz r9, 0xF8(r3) ; this->parent.mEvtInfo.mCommand
    cmpwi r9, 2 ; InDemo
    bne check_condition_if_continuous
    li r9, 3 ; DuringEvent state
	stb r9, 0x3AC(r3)
    lbz r5, 0x326(r3) ; current room number
    lbz r4, 0x3AF(r3) ; switch to set
    lis r3, gameInfo_ptr@ha
    lwz r3, gameInfo_ptr@l(r3)
    addi r3, r3, 0x20
    bl onSwitch
    b return_event_start_check
check_condition_if_continuous:
	lbz r9, 0x3AE(r3) ; is continuous
    cmpwi r9, 0
    beq other_other_id
    bl daSwOp__isCondition
    cmpwi r3, 0
    bne other_other_id
    li r9, 0 ; CheckConditionMet state
	stb r9, 0x3AC(r3)
    b return_event_start_check
other_other_id:
    li r8, 1
    li r7, 0
    li r6, 0
    ori r6, r6, 0xFFFF
	lbz r5, 0x3B2(r31) ; evnt index to start
	lhz r4, 0x3B4(r31) ; event index to start
    mr r3, r31
    bl fopAcM_orderOtherEventId
return_event_start_check:
    lwz r0, 0x14(sp)
    mtlr r0
    lwz r31, 0xC(sp)
    addi sp, sp, 0x10
    blr

.global daSwOp__conditionUnmetCheck
daSwOp__conditionUnmetCheck:
    stwu sp, -0x10(sp)
    mflr r0
    stw r0, 0x14(sp)
    stw r31, 0xC(sp)
    mr r31, r3
    bl daSwOp__isCondition
    cmpwi r3, 0
    bne return_condition_unmet_check
    li r9, 0 ; CheckConditionMet state
	stb r9, 0x3AC(r31)
    lbz r5, 0x326(r31) ; current room number
    lbz r4, 0x3AF(r31) ; switch to set
    lis r3, gameInfo_ptr@ha
    lwz r3, gameInfo_ptr@l(r3)
    addi r3, r3, 0x20
    bl offSwitch
return_condition_unmet_check:
    lwz r0, 0x14(sp)
    mtlr r0
    stw r31, 0xC(sp)
    addi sp, sp, 0x10
    blr

.global daSwOp_Execute
daSwOp_Execute:
	stwu sp, -0x8(sp)
	mflr r0
	stw r0, 0xC(sp)

    lbz r10, 0x3AC(r3)
    cmplwi r10, 4
    bgt return_execute
    lis r9, execute_switch_cases_start@ha
    addi r9, r9, execute_switch_cases_start@l
    rlwinm r10, r10, 2, 0, 0x1D
    add r9, r10, r9
    mtctr r9
    bctr
.global execute_switch_cases_start
execute_switch_cases_start:
    b switch_case_check_met
    b switch_case_timer
    b switch_case_starting_event
    b switch_case_during_event
    b switch_case_check_unmet
    b switch_case_deactivated
switch_case_check_met:
    bl daSwOp__conditionMetCheck
    b return_execute
switch_case_timer:
    bl daSwOp__timerCountdown
    b return_execute
switch_case_starting_event:
    bl daSwOp__eventStartCheck
    b return_execute
switch_case_during_event:
    bl daSwOp__eventEndCheck
    b return_execute
switch_case_check_unmet:
    bl daSwOp__conditionUnmetCheck
    b return_execute
switch_case_deactivated:
return_execute:
    li r3, 1
	lwz r0, 0xC(sp)
	mtlr r0
	addi sp, sp, 0x8
    blr

.global l_daSwOp_Method
l_daSwOp_Method:
    .long daSwOp_Create
    .long daSwOp_Delete
    .long daSwOp_Execute
    .long daSwOp_IsDelete
    .long daSwOp_Draw
    .long 0
    .long 0
    .long 0

.global g_profile_SwitchOperator
g_profile_SwitchOperator:
    .int -3 ; mLayerID
    .short 7 ; mListID
    .short -3 ; mListPrio
    .short 0x1E4 ; Actor ID
    .byte 0
    .byte 0
    .long g_fpcLf_Method
    .int 0x3B8 ; mSize
    .int 0
    .int 0
    .long g_fopAc_Method
    .short 0x9F ; mDrawPriority
    .byte 0
    .byte 0
    .long l_daSwOp_Method
    .int 0 ; mStatus
    .byte 0 ; mActorType
    .byte 0 ; mCullType
    .byte 0
    .byte 0
