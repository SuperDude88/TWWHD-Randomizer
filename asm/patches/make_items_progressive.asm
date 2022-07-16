; This patch modifies the game's code to make certain items progressive, so even if you get them out of order, they will always be upgraded, never downgraded.
; (Note that most of the modifications for this are in the make_items_progressive function of tweaks.py, not here.)

; Swap out the item ID of progressive items for item get events as well as for field items so that their model and item get text change depending on what the next progressive tier of that item you should get is.
.org 0x025d7dac
	bl convert_progressive_item_id_for_createDemoItem
.org @NextFreeSpace
.global convert_progressive_item_id_for_createDemoItem
convert_progressive_item_id_for_createDemoItem:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  mr r3, r31
  bl convert_progressive_item_id
  mr r31, r3
  
  clrlwi r4, r31, 24
  
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.org 0x021806fc
	bl convert_progressive_item_id_for_daItem_create
.org @NextFreeSpace
.global convert_progressive_item_id_for_daItem_create
convert_progressive_item_id_for_daItem_create:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  lbz r3, 0xB3 (r30)
  bl convert_progressive_item_id
  stb r3, 0xB3 (r30)
  
  mr r0, r3
  
  lwz r3, 0x14 (sp)
  mtlr r3
  addi sp, sp, 0x10
  blr

.org 0x024217d8
	bl convert_progressive_item_id_for_dProcGetItem_init_1
.org 0x02421868
	bl convert_progressive_item_id_for_dProcGetItem_init_1
.org @NextFreeSpace
.global convert_progressive_item_id_for_dProcGetItem_init_1
convert_progressive_item_id_for_dProcGetItem_init_1:
  mr r31, r0 ; save r0

  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  lwz r3,0x428(r30)
  bl convert_progressive_item_id
  
  mr r0, r31 ; restore r0
  mr r31, r3
  
  lwz r3, 0x14 (sp)
  mtlr r3
  addi sp, sp, 0x10
  blr

.org 0x0242188c
	bl convert_progressive_item_id_for_dProcGetItem_init_2
.org @NextFreeSpace
.global convert_progressive_item_id_for_dProcGetItem_init_2
convert_progressive_item_id_for_dProcGetItem_init_2:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  lbz r3,0x52a4(r3)
  bl convert_progressive_item_id
  
  mr r31, r3
  
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

 ; In SD they need more things because the RELs get relocated, things are linked statically in HD
.org 0x02483900
	bl convert_progressive_item_id_for_shop_item
.org @NextFreeSpace
.global convert_progressive_item_id_for_shop_item
convert_progressive_item_id_for_shop_item:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  lbz r3,0xb3(r30)

  bl convert_progressive_item_id
  mr r0, r3
  
  lwz r4, 0x14 (sp) ; use different registers to avoid overwriting r0
  mtlr r4
  addi sp, sp, 0x10
  blr


; Fix a big where buying a progressive item from the shop would not show the item get animation if it's the tier 2+ item.
 ; needs testing
.org 0x02215468
	bl custom_getSelectItemNo_progressive
.org 0x022155cc
	bl custom_getSelectItemNo_progressive
.org 0x02215634
	bl custom_getSelectItemNo_progressive
; Acts as a replacement to getSelectItemNo, but should only be called when the shopkeeper is checking if the item get animation should play or not, in order to have that properly show for progressive items past the first tier.
; If this was used all the time as a replacement for getSelectItemNo it would cause the shop to be buggy since it uses the item ID to know what slot it's on.
.org @NextFreeSpace
.global custom_getSelectItemNo_progressive
custom_getSelectItemNo_progressive:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  bl getSelectItemNo
  bl convert_progressive_item_id
  
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.close
