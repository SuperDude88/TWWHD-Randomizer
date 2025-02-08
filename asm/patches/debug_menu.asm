; copy FindPaneByName functionality from 026caad4
; cram it into 026ca60c
; figure out why we are getting cut off at 42 characters

.org 0x026ca548
	b update_debug_menu_text
.org @NextFreeSpace
.global update_debug_menu_text
update_debug_menu_text:
    lwz r3, 0x44(r3) ; this->field_0x44
    lwz r3, 0x4(r3) ; this->field_0x44->field_0x4
    lwz r3, 0xC(r3) ; this->field_0x44->field_0x4->pParts
    lwz r4, 0x8(r3) ; this->field_0x44->field_0x4->pParts->vtbl
    lwz r0, 0x5C(r4) ; this->field_0x44->field_0x4->pParts->vtbl->FindPaneByName
    mtctr r0
    ; r3 already has the right value
    lis r4, custom_debug_pane_name@ha
    addi r4, r4, custom_debug_pane_name@l
    li r5, 1
    bctrl
    ; r3 already has the right value
    lis r4, debug_menu_placeholder_bufferedsafestring@ha
    addi r4, r4, debug_menu_placeholder_bufferedsafestring@l
    li r5, 0
    bl SetTextPaneString
    lbz r10,0x62(r31) ; replace the line we overwrote to jump here
    b 0x026ca54c

; typically the game allocates this on the stack, depending on how giant the string is we could also use heap
; first 4 bytes are pointer to string, then vtbl pointer, then 4 bytes for buffer size (sead::BufferedSafeString)
.global debug_menu_placeholder_bufferedsafestring
debug_menu_placeholder_bufferedsafestring:
  .long debug_menu_placeholder_text ; MUST be a utf16 string
  .long wsafestring_vtbl_10101a6c
  .int 5 ; length (characters) of the string's buffer
.global custom_debug_pane_name
custom_debug_pane_name:
  .string "T_Menu_00"
    .align 2
