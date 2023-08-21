; In itemGetExecute
.org 0x0217DEE0 ; above 0x1f and below 0x38
	b 0x0217E18C
.org 0x0217DF2C ; above 0x37 and below 0x46, not 0x38, not 0x3B, not 0x3D, not 0x45
	b 0x0217E18C
.org 0x0217DF84 ; above 0x4b, not 0x82, not 0x83
	b 0x0217E18C
.org 0x0217DE64 ; above 0x15, less than 0x1E
	bgt 0x0217E18C

; In mode_wait
.org 0x02182DA8 ; above 0x37, below 0x45, not 0x38, not 0x3B, not 0x3D
	blt 0x02182DD8
.org 0x02182DB8 ; above 0x4B
	bl itemActionForArrow
.org 0x02182D70 ; above 0x14, below 0x38, not 0x15, not 0x1E, not 0x1F
	cmpwi r0, 0x20
	mr r3, r26
	blt 0x02182D84 ; itemActionForRupee
	bl itemActionForArrow ; >= 0x20
	b 0x02182E5C

; Rewrite getYOffset and dance around the existing relocations
; We want items <= 0x1F to use 0.0 (except 0x1A), anything above to use 23.0, except 0x45-0x4B use 0.0, 0x38 uses 20.0 and 0x3D uses 10.0
; This would need too many instructions to fit nicely (I think), so we ignore 0x1A and 0x3D because the randomizer never places them
.org 0x0217F1B4
	lbz r3, 0x74E(r3) ; Change this to use r3 (subi didn't like r0)
	cmplwi r3, 0x38
	beq 0x0217F1E8 ; Load 20.0
	subi r30, r3, 0x45
	cmplwi r30, 0x6 ; 0x4B - 0x45 = 0x6, unsigned compare
	ble 0x0217F1D0 ; Jump to loading 0.0 if we are in the range [0x45, 0x4B]
	cmplwi r3, 0x1F ; If we aren't, check if we are <= 0x1F
.org 0x0217F1D8 ; After loading 0.0, overwrite its blr
	ble 0x0217F1FC ; If we are less than 0x1F, or still have the CR set from the subtract -> compare, return from another blr in the function
	; Otherwise we spill over to loading 23.0 and return

; The slightly too long version, also needs replaced relocations:
; .org 0x0217F1B8
; 	lis r12, float_symbol_twenty@ha
; 
; 	cmpwi r0, 0x45
; 	blt check_low_ids
; 	cmpwi r0, 0x4B
; 	ble load_zero
; 
; check_low_ids:
; 	cmpwi r0, 0x1F
; 	ble load_zero
; 	cmpwi r0, 0x38
; 	beq load_twenty
; 	cmpwi r0, 0x3D
; 	beq load_ten
; 
; 	lfs f1, float_twenty_three@l(r12)
; 	blr
; load_zero:
; 	lfs f1, float_zero@l(r12)
; 	blr
; load_ten:
; 	lfs f1, float_ten@l(r12)
; 	blr
; load_twenty:
; 	lfs f1, float_twenty@l(r12)
; 	blr
