.org 0x0217dee8 ; between 0x1f and  0x38, not 34, 
	b 0x0217e18c
.org 0x0217df2c ; between 0x37 and 0x46, not 0x38, not 0x3B, not 0x3D, not 0x45
	b 0x0217E18C
.org 0x0217df84 ; above 0x4b, not 0x82, not 0x83
	b 0x0217E18C
.org 0x0217dec8 ; id 0x13
	b 0x0217E18C
.org 0x0217de64 ; any extraneous values
	bgt 0x0217E18C

.org 0x02182d84
	bl itemActionForArrow
.org 0x02182da8
	blt 0x02182dd8
.org 0x02182db8
	bl itemActionForArrow
.org 0x02182d40
	nop
.org 0x02180f08
	nop
.org 0x101e8ac6
	.short 0x8466

.org 0x02182e58
	b check_rupee_mode_wait
.org @NextFreeSpace
.global check_rupee_mode_wait
check_rupee_mode_wait:
	cmpwi r0, 0x20
	blt rupee_wait
	cmpwi r0, 0x44
	bgt rupee_wait
	bl itemActionForArrow
	b 0x02182e5c
  rupee_wait:
	bl itemActionForRupee
	b 0x02182e5c

.close
