.org 0x028f87f4

.global hurricane_spin_item_resource_arc_name
hurricane_spin_item_resource_arc_name:
.string "Vscroll"
.align 2

; save init goes here, has bugs though so not yet

.global convert_progressive_item_id
convert_progressive_item_id:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

cmpwi r3, 0x38
beq convert_progressive_sword_id
cmpwi r3, 0x39
beq convert_progressive_sword_id
cmpwi r3, 0x3A
beq convert_progressive_sword_id
cmpwi r3, 0x3D
beq convert_progressive_sword_id
cmpwi r3, 0x3E
beq convert_progressive_sword_id

cmpwi r3, 0x3B
beq convert_progressive_shield_id
cmpwi r3, 0x3C
beq convert_progressive_shield_id

cmpwi r3, 0x27
beq convert_progressive_bow_id
cmpwi r3, 0x35
beq convert_progressive_bow_id
cmpwi r3, 0x36
beq convert_progressive_bow_id

cmpwi r3, 0xAB
beq convert_progressive_wallet_id
cmpwi r3, 0xAC
beq convert_progressive_wallet_id

cmpwi r3, 0xAD
beq convert_progressive_bomb_bag_id
cmpwi r3, 0xAE
beq convert_progressive_bomb_bag_id

cmpwi r3, 0xAF
beq convert_progressive_quiver_id
cmpwi r3, 0xB0
beq convert_progressive_quiver_id

cmpwi r3, 0x23
beq convert_progressive_picto_box_id
cmpwi r3, 0x26
beq convert_progressive_picto_box_id

b convert_progressive_item_id_func_end


convert_progressive_sword_id:
lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0xd4
lbz r4, 0 (r3) ; Bitfield of swords you own
cmpwi r4, 0
beq convert_progressive_sword_id_to_normal_sword
cmpwi r4, 1
beq convert_progressive_sword_id_to_powerless_master_sword
cmpwi r4, 3
beq convert_progressive_sword_id_to_half_power_master_sword
cmpwi r4, 7
beq convert_progressive_sword_id_to_full_power_master_sword
li r3, 0x38
b convert_progressive_item_id_func_end

convert_progressive_sword_id_to_normal_sword:
li r3, 0x38
b convert_progressive_item_id_func_end
convert_progressive_sword_id_to_powerless_master_sword:
li r3, 0x39
b convert_progressive_item_id_func_end
convert_progressive_sword_id_to_half_power_master_sword:
li r3, 0x3A
b convert_progressive_item_id_func_end
convert_progressive_sword_id_to_full_power_master_sword:
li r3, 0x3E
b convert_progressive_item_id_func_end


convert_progressive_shield_id:
lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0xd5 ;Check if this is right location for shield bitfield
lbz r4, 0 (r3)
cmpwi r4, 0
beq convert_progressive_shield_id_to_heros_shield
cmpwi r4, 1
beq convert_progressive_shield_id_to_mirror_shield
li r3, 0x3B
b convert_progressive_item_id_func_end

convert_progressive_shield_id_to_heros_shield:
li r3, 0x3B
b convert_progressive_item_id_func_end
convert_progressive_shield_id_to_mirror_shield:
li r3, 0x3C
b convert_progressive_item_id_func_end


convert_progressive_bow_id:
lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x7d
lbz r4, 0 (r3) ; Bitfield of arrow types you own
cmpwi r4, 0
beq convert_progressive_bow_id_to_heros_bow
cmpwi r4, 1
beq convert_progressive_bow_id_to_fire_and_ice_arrows
cmpwi r4, 3
beq convert_progressive_bow_id_to_light_arrows
li r3, 0x27 ; Invalid bow state; this shouldn't happen so just return the base bow ID
b convert_progressive_item_id_func_end

convert_progressive_bow_id_to_heros_bow:
li r3, 0x27
b convert_progressive_item_id_func_end
convert_progressive_bow_id_to_fire_and_ice_arrows:
li r3, 0x35
b convert_progressive_item_id_func_end
convert_progressive_bow_id_to_light_arrows:
li r3, 0x36
b convert_progressive_item_id_func_end


convert_progressive_wallet_id:
lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x32
lbz r4, 0 (r3) ; Which wallet you have
cmpwi r4, 0
beq convert_progressive_wallet_id_to_1000_rupee_wallet
cmpwi r4, 1
beq convert_progressive_wallet_id_to_5000_rupee_wallet
li r3, 0xAB ; Invalid wallet state; this shouldn't happen so just return the base wallet ID
b convert_progressive_item_id_func_end

convert_progressive_wallet_id_to_1000_rupee_wallet:
li r3, 0xAB
b convert_progressive_item_id_func_end
convert_progressive_wallet_id_to_5000_rupee_wallet:
li r3, 0xAC
b convert_progressive_item_id_func_end


convert_progressive_bomb_bag_id:
lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x8a
lbz r4, 6 (r3) ; Max number of bombs the player can currently hold
cmpwi r4, 30
beq convert_progressive_bomb_bag_id_to_60_bomb_bomb_bag
cmpwi r4, 60
beq convert_progressive_bomb_bag_id_to_99_bomb_bomb_bag
li r3, 0xAD ; Invalid bomb bag state; this shouldn't happen so just return the base bomb bag ID
b convert_progressive_item_id_func_end

convert_progressive_bomb_bag_id_to_60_bomb_bomb_bag:
li r3, 0xAD
b convert_progressive_item_id_func_end
convert_progressive_bomb_bag_id_to_99_bomb_bomb_bag:
li r3, 0xAE
b convert_progressive_item_id_func_end


convert_progressive_quiver_id:
lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x89
lbz r4, 6 (r3) ; Max number of arrows the player can currently hold
cmpwi r4, 30
beq convert_progressive_quiver_id_to_60_arrow_quiver
cmpwi r4, 60
beq convert_progressive_quiver_id_to_99_arrow_quiver
li r3, 0xAF ; Invalid quiver state; this shouldn't happen so just return the base quiver ID
b convert_progressive_item_id_func_end

convert_progressive_quiver_id_to_60_arrow_quiver:
li r3, 0xAF
b convert_progressive_item_id_func_end
convert_progressive_quiver_id_to_99_arrow_quiver:
li r3, 0xB0
b convert_progressive_item_id_func_end


convert_progressive_picto_box_id:
lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x76
lbz r4, 0 (r3) ; Bitfield of picto boxes you own
cmpwi r4, 0
beq convert_progressive_picto_box_id_to_normal_picto_box
cmpwi r4, 1
beq convert_progressive_picto_box_id_to_deluxe_picto_box
li r3, 0x23 ; Invalid bomb bag state; this shouldn't happen so just return the base bomb bag ID
b convert_progressive_item_id_func_end

convert_progressive_picto_box_id_to_normal_picto_box:
li r3, 0x23
b convert_progressive_item_id_func_end
convert_progressive_picto_box_id_to_deluxe_picto_box:
li r3, 0x26
b convert_progressive_item_id_func_end


convert_progressive_item_id_func_end:
lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global convert_progressive_item_id_for_createDemoItem
convert_progressive_item_id_for_createDemoItem:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  mr r3, r31
  bl convert_progressive_item_id
  mr r31, r3
  
  li r3,0x101
  
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.global convert_progressive_item_id_for_daItem_create
convert_progressive_item_id_for_daItem_create:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  lbz r3, 0xB3 (r31)
  bl convert_progressive_item_id
  stb r3, 0xB3 (r31)
  
  mr r0, r3
  
  lwz r3, 0x14 (sp)
  mtlr r3
  addi sp, sp, 0x10
  blr

.global convert_progressive_item_id_for_dProcGetItem_init_1
convert_progressive_item_id_for_dProcGetItem_init_1:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  lwz r3,0x428(r30)
  bl convert_progressive_item_id
  
  mr r31, r3
  
  lwz r3, 0x14 (sp)
  mtlr r3
  addi sp, sp, 0x10
  blr

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
.global convert_progressive_item_id_for_shop_item
convert_progressive_item_id_for_shop_item:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  lbz r3,0xb3(r30)

  bl convert_progressive_item_id
  mr r0, r3
  
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

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

.global progressive_sword_item_func
progressive_sword_item_func:
; Function start stuff
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0xd4
lbz r4, 0 (r3) ; Bitfield of swords you own
cmpwi r4, 0
beq get_normal_sword
cmpwi r4, 1
beq get_powerless_master_sword
cmpwi r4, 3
beq get_half_power_master_sword
cmpwi r4, 7
beq get_full_power_master_sword
b sword_func_end

get_normal_sword:
bl item_func_sword
b sword_func_end

get_powerless_master_sword:
bl item_func_master_sword
b sword_func_end

get_half_power_master_sword:
bl item_func_lv3_sword
b sword_func_end

get_full_power_master_sword:
bl item_func_master_sword_ex

sword_func_end:
; Function end stuff
lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global progressive_shield_item_func
progressive_shield_item_func:
; Function start stuff
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0xd5 ;Check if this is right location for shield bitfield
lbz r4, 0 (r3) ; Bitfield of shields you own
cmpwi r4, 0
beq get_heros_shield
cmpwi r4, 1
beq get_mirror_shield
b shield_func_end

get_heros_shield:
bl item_func_shield
b shield_func_end

get_mirror_shield:
bl item_func_mirror_shield

shield_func_end:
; Function end stuff
lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global progressive_bow_func
progressive_bow_func:
; Function start stuff
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x7d
lbz r4, 0 (r3) ; Bitfield of arrow types you own
cmpwi r4, 0
beq get_heros_bow
cmpwi r4, 1
beq get_fire_and_ice_arrows
cmpwi r4, 3
beq get_light_arrows
b bow_func_end

get_heros_bow:
bl item_func_bow
b bow_func_end

get_fire_and_ice_arrows:
bl item_func_magic_arrow
b bow_func_end

get_light_arrows:
bl item_func_light_arrow

bow_func_end:
; Function end stuff
lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global progressive_wallet_item_func
progressive_wallet_item_func:

lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x32
lbz r4, 0 (r3) ; Which wallet you have
cmpwi r4, 0
beq get_1000_rupee_wallet
cmpwi r4, 1
beq get_5000_rupee_wallet
b wallet_func_end

get_1000_rupee_wallet:
li r4, 1
stb r4, 0 (r3) ; Which wallet you have
b wallet_func_end

get_5000_rupee_wallet:
li r4, 2
stb r4, 0 (r3) ; Which wallet you have

wallet_func_end:
blr

.global progressive_bomb_bag_item_func
progressive_bomb_bag_item_func:

lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x8a
lbz r4, 6 (r3) ; Max number of bombs the player can currently hold
cmpwi r4, 30
beq get_60_bomb_bomb_bag
cmpwi r4, 60
beq get_99_bomb_bomb_bag
b bomb_bag_func_end

get_60_bomb_bomb_bag:
li r4, 60
stb r4, 0 (r3) ; Current num bombs
stb r4, 6 (r3) ; Max num bombs
b bomb_bag_func_end

get_99_bomb_bomb_bag:
li r4, 99
stb r4, 0 (r3) ; Current num bombs
stb r4, 6 (r3) ; Max num bombs

bomb_bag_func_end:
blr

.global progressive_quiver_item_func
progressive_quiver_item_func:

lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x89
lbz r4, 6 (r3) ; Max number of arrows the player can currently hold
cmpwi r4, 30
beq get_60_arrow_quiver
cmpwi r4, 60
beq get_99_arrow_quiver
b quiver_func_end

get_60_arrow_quiver:
li r4, 60
stb r4, 0 (r3) ; Current num arrows
stb r4, 6 (r3) ; Max num arrows
b quiver_func_end

get_99_arrow_quiver:
li r4, 99
stb r4, 0 (r3) ; Current num arrows
stb r4, 6 (r3) ; Max num arrows

quiver_func_end:
blr

.global progressive_picto_box_item_func
progressive_picto_box_item_func:
; Function start stuff
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x76
lbz r4, 0 (r3) ; Bitfield of picto boxes you own
cmpwi r4, 0
beq get_normal_picto_box
cmpwi r4, 1
beq get_deluxe_picto_box
b picto_box_func_end

get_normal_picto_box:
bl item_func_camera
b picto_box_func_end

get_deluxe_picto_box:
bl item_func_camera2

picto_box_func_end:
; Function end stuff
lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global hurricane_spin_item_func
hurricane_spin_item_func:
; Function start stuff
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

lis r3,0x1020
lwz r3,-0x7b24(r3)
li r4,0x6901
addi r3,r3,0x644
bl onEventBit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global custom_createItem
custom_createItem:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

; If the item ID is FF, no item will be spawned.
; In order to avoid issues we need to return -1.
; Also don't bother even trying to spawn the item - it wouldn't do anything.
cmpwi r4, 0xFF
beq custom_createItem_invalid_item_id

; Create the item by calling createItem, which will load the item's model if necessary.
mr r9, r5
mr r5, r8
mr r8, r6
mr r6, r9
mr r10, r7
li r7, 3 ; Don't fade out
li r9, 5 ; Item action, how it behaves. 5 causes it to make a ding sound so that it's more obvious.
bl fopAcM_createItem

; Return the actor's unique ID that createItem returned.
b custom_createItem_func_end

custom_createItem_invalid_item_id:
li r3, -1 ; Return -1 to indicate no actor was created.

custom_createItem_func_end:
lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global generic_on_dungeon_bit
generic_on_dungeon_bit:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

mr r5, r3 ; Argument r3 to this func is the stage ID of the dungeon to add this item for
mr r6, r4 ; Argument r4 to this func is the bit index to set

lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x7bc ; stage ID of the current stage
lbz r4, 0 (r3)
cmpw r4, r5 ; Check if we're currently in the right dungeon for this key
beq generic_on_dungeon_bit_in_correct_dungeon

lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x3a0
mulli r4, r5, 0x24 ; Use stage ID of the dungeon as the index, each entry in the list is 0x24 bytes long
add r3, r3, r4
b generic_on_dungeon_bit_func_end

generic_on_dungeon_bit_in_correct_dungeon:
; In the correct dungeon for this dungeon bit.
lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x798

generic_on_dungeon_bit_func_end:
; Now call onDungeonBit with argument r3 being the stage info that was determined above.
mr r4, r6 ; Argument r4 is the bit index
bl onDungeonItem

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global generic_small_key_item_get_func
generic_small_key_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

mr r5, r3 ; Argument r3 is the stage ID of the dungeon to add this item for
lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x7bc ; stage ID of the current stage
lbz r4, 0 (r3)
cmpw r4, r5 ; Check if we're currently in the right dungeon for this key
bne generic_small_key_item_get_func_not_in_correct_dungeon

bl FUN_025200d4
lwzu r12,0x5150(r3)
lwz r12,0x15c(r12)
mtctr r12
bctrl
lbz r0,9(r3)
rlwinm. r0, r0, 0, 31, 31
beq generic_small_key_item_get_func_in_non_dungeon_room_of_correct_dungeon
b generic_small_key_item_get_func_in_correct_dungeon

generic_small_key_item_get_func_not_in_correct_dungeon:
; Not in the correct dungeon for this small key.
; We need to bypass the normal small key adding method.
; Instead we add directly to the small key count for the correct dungeon's stage info.
lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x3a0
mulli r4, r5, 0x24 ; Use stage ID of the dungeon as the index, each entry in the list is 0x24 bytes long
add r3, r3, r4
lbz r4, 0x20 (r3) ; Current number of keys for the correct dungeon
addi r4, r4, 1
stb r4, 0x20 (r3) ; Current number of keys for the correct dungeon
b generic_small_key_item_get_func_end

generic_small_key_item_get_func_in_non_dungeon_room_of_correct_dungeon:
lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x798
lbz r4, 0x20 (r3) ; Current number of keys for the current dungeon
addi r4, r4, 1
stb r4, 0x20 (r3) ; Current number of keys for the current dungeon
b generic_small_key_item_get_func_end

generic_small_key_item_get_func_in_correct_dungeon:
; In the correct dungeon for this small key.
; Simply call the normal small key func, as it will work correctly in this case.
bl item_func_small_key

generic_small_key_item_get_func_end:
lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global drc_small_key_item_get_func
drc_small_key_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 3 ; DRC stage ID
bl generic_small_key_item_get_func

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global fw_small_key_item_get_func
fw_small_key_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 4 ; FW stage ID
bl generic_small_key_item_get_func

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global totg_small_key_item_get_func
totg_small_key_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 5 ; TotG stage ID
bl generic_small_key_item_get_func

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global et_small_key_item_get_func
et_small_key_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 6 ; ET stage ID
bl generic_small_key_item_get_func

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global wt_small_key_item_get_func
wt_small_key_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 7 ; WT stage ID
bl generic_small_key_item_get_func

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr




.global drc_big_key_item_get_func
drc_big_key_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 3 ; DRC stage ID
li r4, 2 ; Big key bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global fw_big_key_item_get_func
fw_big_key_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 4 ; FW stage ID
li r4, 2 ; Big key bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global totg_big_key_item_get_func
totg_big_key_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 5 ; TotG stage ID
li r4, 2 ; Big key bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global et_big_key_item_get_func
et_big_key_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 6 ; ET stage ID
li r4, 2 ; Big key bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global wt_big_key_item_get_func
wt_big_key_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 7 ; WT stage ID
li r4, 2 ; Big key bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr




.global drc_dungeon_map_item_get_func
drc_dungeon_map_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 3 ; DRC stage ID
li r4, 0 ; Dungeon map bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global fw_dungeon_map_item_get_func
fw_dungeon_map_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 4 ; FW stage ID
li r4, 0 ; Dungeon map bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global totg_dungeon_map_item_get_func
totg_dungeon_map_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 5 ; TotG stage ID
li r4, 0 ; Dungeon map bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global ff_dungeon_map_item_get_func
ff_dungeon_map_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 2 ; FF stage ID
li r4, 0 ; Dungeon map bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global et_dungeon_map_item_get_func
et_dungeon_map_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 6 ; ET stage ID
li r4, 0 ; Dungeon map bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global wt_dungeon_map_item_get_func
wt_dungeon_map_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 7 ; WT stage ID
li r4, 0 ; Dungeon map bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr




.global drc_compass_item_get_func
drc_compass_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 3 ; DRC stage ID
li r4, 1 ; Compass bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global fw_compass_item_get_func
fw_compass_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 4 ; FW stage ID
li r4, 1 ; Compass bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global totg_compass_item_get_func
totg_compass_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 5 ; TotG stage ID
li r4, 1 ; Compass bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global ff_compass_item_get_func
ff_compass_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 2 ; FF stage ID
li r4, 1 ; Compass bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global et_compass_item_get_func
et_compass_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 6 ; ET stage ID
li r4, 1 ; Compass bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global wt_compass_item_get_func
wt_compass_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

li r3, 7 ; WT stage ID
li r4, 1 ; Compass bit index
bl generic_on_dungeon_bit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global dragon_tingle_statue_item_get_func
dragon_tingle_statue_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x644
li r4, 0x6A04 ; Unused event bit we use for Dragon Tingle Statue
bl onEventBit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global forbidden_tingle_statue_item_get_func
forbidden_tingle_statue_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x644
li r4, 0x6A08 ; Unused event bit we use for Forbidden Tingle Statue
bl onEventBit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global goddess_tingle_statue_item_get_func
goddess_tingle_statue_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x644
li r4, 0x6A10 ; Unused event bit we use for Goddess Tingle Statue
bl onEventBit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global earth_tingle_statue_item_get_func
earth_tingle_statue_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x644
li r4, 0x6A20 ; Unused event bit we use for Earth Tingle Statue
bl onEventBit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr


.global wind_tingle_statue_item_get_func
wind_tingle_statue_item_get_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x644
li r4, 0x6A40 ; Unused event bit we use for Wind Tingle Statue
bl onEventBit

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global check_tingle_statue_owned
check_tingle_statue_owned:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

cmpwi r4, 0xF ; The opened flag index (argument r4) for tingle statue chests should always be 0xF.
bne check_tingle_statue_owned_invalid

; The stage ID (argument r3) determines which dungeon it's checking.
cmpwi r3, 3
beq check_dragon_tingle_statue_owned
cmpwi r3, 4
beq check_forbidden_tingle_statue_owned
cmpwi r3, 5
beq check_goddess_tingle_statue_owned
cmpwi r3, 6
beq check_earth_tingle_statue_owned
cmpwi r3, 7
beq check_wind_tingle_statue_owned
b check_tingle_statue_owned_invalid

check_dragon_tingle_statue_owned:
li r4, 0x6A04 ; Unused event bit
b check_tingle_statue_owned_event_bit

check_forbidden_tingle_statue_owned:
li r4, 0x6A08 ; Unused event bit
b check_tingle_statue_owned_event_bit

check_goddess_tingle_statue_owned:
li r4, 0x6A10 ; Unused event bit
b check_tingle_statue_owned_event_bit

check_earth_tingle_statue_owned:
li r4, 0x6A20 ; Unused event bit
b check_tingle_statue_owned_event_bit

check_wind_tingle_statue_owned:
li r4, 0x6A40 ; Unused event bit

check_tingle_statue_owned_event_bit:
lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x644
bl isEventBit
b check_tingle_statue_owned_end

check_tingle_statue_owned_invalid:
; If the function call was somehow invalid, return false.
li r3, 0

check_tingle_statue_owned_end:
lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global custom_isTbox_for_unloaded_stage_save_info
custom_isTbox_for_unloaded_stage_save_info:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

mulli r0, r3, 0x24 ; Use stage info ID as the index, each entry in the list is 0x24 bytes long
lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x3a0
add r3, r3, r0

bl isTbox

lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

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

.global jank_orca_counter_failsafe
jank_orca_counter_failsafe:
  cmpwi r12, 0x38 ; Hero's Sword
  bne not_heros_sword
  beq 0x0225af1c ; Use Hero's Sword icon for the counter (icon 1)
  not_heros_sword:
  cmpwi r12, 0xFF ; No sword
  bne master_sword
  b 0x0225afb4 ; Skip past the code to create the counter entirely
  master_sword:
  b 0x0225af80 ; Use Master Sword icon for the counter (icon 2)

 .global ultra_sketch_beedle_patch ; REPLACE ble INSTRUCTION AT 0X02215864
 ultra_sketch_beedle_patch:
   cmplwi r0, 0x4b
   bne beedle_not_blue_chu_jelly
   li r31, 0xf75
   b 0x02215d80

   beedle_not_blue_chu_jelly:
   blt less_than_val
   b 0x02215868

   less_than_val:
   b 0x02215898

.global stalfos_kill_lower_body_when_upper_body_light_arrowed
stalfos_kill_lower_body_when_upper_body_light_arrowed:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  bl fopAcIt_Judge ; Get upper body entity
  cmplwi r3, 0
  beq stalfos_kill_lower_body_when_upper_body_light_arrowed_end
  
  lbz r0, 0x1FA4 (r3) ; Counter for how many frames the upper body has been dying to light arrows
  cmpwi r0, 0
  beq stalfos_kill_lower_body_when_upper_body_light_arrowed_end ; The upper body hasn't been hit with light arrows, so don't kill the lower body either
  
  lbz r0, 0x1FA4 (r31) ; Counter for how many frames the lower body has been dying to light arrows
  cmpwi r0, 0
  bne stalfos_kill_lower_body_when_upper_body_light_arrowed_end ; The lower body is already dying to light arrows, so don't reset its counter
  
  li r0, 1
  stb r0, 0x1FA4 (r31) ; Start the lower body's counter for dying to light arrows at 1
  
  stalfos_kill_lower_body_when_upper_body_light_arrowed_end:
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

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
  lwz r12, 0x8 (sp) ; Read Jalhalla entity pointer (original code used sp+8 but this function's stack offset is +0x10)
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
  lwz r9,0x8(sp) ; Read Jalhalla entity pointer again
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

;Seems fine
.global magtail_respawn_when_head_light_arrowed
magtail_respawn_when_head_light_arrowed:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  bl GetTgHitObj ; Replace the function call we overwrote to call this custom function
  
  ; Then we need to reproduce a few lines of code from the original function.
  addi r9,r31,0x1990
  stw r3, 0xc8 (sp) ; Write the TgHitObj (original code used sp+0x30 but this function's stack offset is +0x10)
  stw r9, 0xdc (sp) ; Write something from the Magtail entity (original code used sp+0x44 but this function's stack offset is +0x10)
  lwz r5, 0x10 (r3) ; Read the bitfield of damage types done by the actor that just damaged this Magtail
  rlwinm. r4,r5,0x0,0xb,0xb ; Check the Light Arrows damage type
  
  ; Then if the Light Arrows bit was set, we store true to magtail_entity+0x1CBC to signify that the Magtail should respawn.
  ; (We can't use the original branch on the Light Arrows bit because it's inside the REL.)
  ; Check how beq works and find addresses for the rest of this patch
  beq magtail_respawn_when_head_light_arrowed_end
  li r0, 1
  stb r0, 0x1dd8 (r31)
  
  magtail_respawn_when_head_light_arrowed_end:
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

;Seems fine now
.global phantom_ganon_check_link_within_y_diff
phantom_ganon_check_link_within_y_diff:
  lfs f10, 0x24 (sp) ; Read the Y difference between Link and Phantom Ganon
  lfs f1, -0x1018(r9) ; load from 0x1000efe8
  
  ; If the Link is 1000.0 units or more higher than PG, do not trigger the fight.
  ; (Does not account for negative difference. Still extends infinitely downwards.)
  fcmpo cr0, f10, f1
  bge phantom_ganon_check_link_within_y_diff_outside_range
  
  ; Otherwise, go on to check the X and Z difference as usual.
  fmuls f9,f11,f11 ; Replace the line of code we overwrote to jump here
  b 0x02139b80

phantom_ganon_check_link_within_y_diff_outside_range:
  b 0x02139e4c

.global set_shop_item_in_bait_bag_slot_sold_out
set_shop_item_in_bait_bag_slot_sold_out:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  ; First call the regular SoldOutItem function with the given arguments since we overwrote a call to that in order to call this custom function.
  bl SoldOutItem
  
  ; Set event bit 6902 (bit 02 of byte 803C5295).
  ; This bit was unused in the base game, but we repurpose it to keep track of whether you've purchased whatever item is in the Bait Bag slot of Beedle's shop.
  lis r3,0x1020
  lwz r3,-0x7b24(r3)
  addi r3,r3,0x644
  li r4, 0x6902 ; Unused event bit
  bl onEventBit
  
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr
.global check_shop_item_in_bait_bag_slot_sold_out
check_shop_item_in_bait_bag_slot_sold_out:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  ; Check event bit 6902 (bit 02 of byte 803C5295), which was originally unused but we use it to keep track of whether the item in the Bait Bag slot has been purchased or not.
  lis r3,0x1020
  lwz r3,-0x7b24(r3)
  addi r3,r3,0x644
  li r4, 0x6902 ; Unused event bit
  bl isEventBit
  
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

;some HD optimizations removed the extra space that was used in SD
.global withered_trees_custom_createitem
  withered_trees_custom_createitem:
  or. r30,r3,r3
  cmpwi r30,-1
  bne item_created
  b 0x02347cd4
  item_created:
  b 0x02347c40 ;MAKE SURE TO PATCH THE lwz TO mr

.global create_item_for_withered_trees_without_setting_speeds
create_item_for_withered_trees_without_setting_speeds:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  bl custom_createItem
  
  ; We need to set our custom flag at withered_tree_entity+0x212 to 1 to prevent withered_tree_item_try_give_momentum from setting the speeds for this item actor.
  li r5, 1
  stb r5, 0x32e (r3)
  
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

  ;MAKE SURE TO PATCH cmpwi at 0x02347ac0 and following lwz
  ;MAKE SURE TO nop other instruction that uses the returned object pointer


.global withered_trees_custom_init
withered_trees_custom_init:
  li r11,0xffff
  stw r11,0x76C(r30)
  li r11, 0x1
  b 0x02346d8c

.global withered_tree_item_try_give_momentum  ;This needs to change completely since search_heart_part is gone, need to find new loop point to check
withered_tree_item_try_give_momentum:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  ; First replace the function call we overwrote to call this custom function.
  bl fopAcM_SearchByID
  
  cmpwi r3, 0
  beq withered_tree_item_try_give_momentum_end ; Item actor has already been picked up
  
  lwz r4, 0x18 (sp) ; Read the item actor pointer (original code used sp+8 but this function's stack offset is +0x10)
  cmpwi r4, 0
  beq withered_tree_item_try_give_momentum_end ; Item actor was just created a few frames ago and hasn't actually been properly spawned yet
  
  ; Now that we have the item actor pointer in r4, we need to check if the actor was just created this frame or not.
  ; To do that we store a custom flag to an unused byte in the withered tree actor struct.
  lbz r5, 0x32e (r3) ; (Bytes +0x32e and +0x32f were originally just padding)
  cmpwi r5, 0
  bne withered_tree_item_try_give_momentum_end ; Already set the flag, so this isn't the first frame it spawned on.
  
  ; Since this is the first frame since the item actor was properly created, we can set its momentum.
  lis r10, withered_tree_item_speeds@ha
  addi r10, r10, withered_tree_item_speeds@l
  lfs f0, 0 (r10) ; Read forward velocity
  stfs f0, 0x370 (r4)
  lfs f0, 4 (r10) ; Read the Y velocity
  stfs f0, 0x340 (r4)
  lfs f0, 8 (r10) ; Read gravity
  stfs f0, 0x374 (r4)
  
  ; Also set bit 0x40 in some bitfield for the item actor.
  ; Apparently this bit is for allowing the actor to still move while events are going on. It doesn't seem to really matter in this specific case, but set it just to be completely safe.
  lwz r5, 0x2e0 (r4)
  ori r5, r5, 0x40
  stw r5, 0x2e0 (r4)
  
  ; Now store the custom flag meaning that we've already set the item actor's momentum so we don't do it again.
  li r5, 1
  stb r5, 0x32e (r3)

withered_tree_item_try_give_momentum_end:
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.global withered_tree_item_speeds
withered_tree_item_speeds:
  .float 1.75 ; Initial forward velocity
  .float 30 ; Initial Y velocity
  .float -2.1 ; Gravity (Y acceleration)

.global check_ganons_tower_chest_opened
check_ganons_tower_chest_opened:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  li r3, 8 ; Stage ID for Ganon's Tower.
  li r4, 0 ; Chest open flag for the Light Arrows chest. Just 0 since this is the only chest in the whole dungeon.
  bl dComIfGs_isStageTbox
  
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.global check_phantom_ganons_sword_should_disappear
check_phantom_ganons_sword_should_disappear:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  ; First replace the event flag check we overwrote to call this custom function.
  bl isEventBit
  
  ; If the player hasn't destroyed the door with Phantom Ganon's sword yet, we don't need to do anything different so just return.
  cmpwi r3, 0
  beq check_phantom_ganons_sword_should_disappear_end
  
  ; If the player has destroyed the door, check if the current stage is the Phantom Ganon maze, where Phantom Ganon 2 is fought.
  bl FUN_025200d4
  addi r3, r3, 0x5133
  lis r4, phantom_ganon_maze_stage_name@ha
  addi r4, r4, phantom_ganon_maze_stage_name@l
strcmp_start:
    lbzu r5, 0x1(r3)
    lbzu r6, 0x1(r4)
    cmplw r5, r6
    bne check_phantom_ganons_sword_should_disappear_end
    cmplwi r5, 0
    bne strcmp_start

  ; If the stage is the maze, strcmp will return 0, so we return that to tell Phantom Ganon's sword that it should not disappear.
  ; If the stage is anything else, strcmp will not return 0, so Phantom Ganon's sword should disappear.
check_phantom_ganons_sword_should_disappear_end:
  subf. r3, r5, r6
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.global phantom_ganon_maze_stage_name
phantom_ganon_maze_stage_name:
  .string "GanonJ"
  .align 2 ; Align to the next 4 bytes

.global create_item_and_set_event_bit_for_townsperson
create_item_and_set_event_bit_for_townsperson:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  stw r31, 0xC (sp)
  mr r31, r4 ; Preserve argument r4, which has both the item ID and the event bit to set.
  
  clrlwi r4,r4,24 ; Get the lowest byte (0x000000FF), which has the item ID
  bl fopAcM_createItemForPresentDemo
  
  rlwinm. r4,r31,16,16,31 ; Get the upper halfword (0xFFFF0000), which has the event bit to set
  beq create_item_and_set_event_bit_for_townsperson_end ; If the event bit specified is 0000, skip to the end of the function instead
  mr r31, r3 ; Preserve the return value from createItemForPresentDemo so we can still return that
  lis r3,0x1020
  lwz r3,-0x7b24(r3)
  addi r3,r3,0x644
  bl onEventBit ; Otherwise, set that event bit
  mr r3, r31
  
create_item_and_set_event_bit_for_townsperson_end:
  lwz r31, 0xC (sp)
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

;THIS MIGHT NOT OVERWRITE A WORKING LINE, MAKE SURE THIS RUNS AT THE RIGHT POINT
.global lenzo_set_deluxe_picto_box_event_bit
lenzo_set_deluxe_picto_box_event_bit:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  ; First replace the line we overwrote to call this custom function.
  stb r10,0xb39(r31)
  
  ; Next set an originally-unused event bit to keep track of whether the player got the item that was the Deluxe Picto Box in vanilla.
  lis r3,0x1020
  lwz r3,-0x7b24(r3)
  addi r3,r3,0x644
  li r4, 0x6920
  bl onEventBit
  
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.global zunari_give_item_and_set_magic_armor_event_bit
zunari_give_item_and_set_magic_armor_event_bit:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  stw r31, 0xC (sp)
  mr r31, r4 ; Preserve argument r4, which has the item ID
  
  bl fopAcM_createItemForPresentDemo
  
  lis r4, zunari_magic_armor_slot_item_id@ha
  addi r4, r4, zunari_magic_armor_slot_item_id@l
  lbz r4, 0 (r4) ; Load what item ID is in the Magic Armor slot. This value is updated by the randomizer when it randomizes that item.
  
  cmpw r31, r4 ; Check if the item ID given is the same one from the Magic Armor slot.
  bne zunari_give_item_and_set_magic_armor_event_bit_end ; If it's not the item in the Magic Armor slot, skip to the end of the function
  mr r31, r3 ; Preserve the return value from createItemForPresentDemo so we can still return that
  lis r3,0x1020
  lwz r3,-0x7b24(r3)
  addi r3,r3,0x644
  li r4, 0x6940 ; Unused event bit that we use to keep track of whether Zunari has given the Magic Armor item
  bl onEventBit
  mr r3, r31
  
zunari_give_item_and_set_magic_armor_event_bit_end:
  lwz r31, 0xC (sp)
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.global zunari_magic_armor_slot_item_id
zunari_magic_armor_slot_item_id:
  .byte 0x2A ; Default item ID is Magic Armor. This value is updated by the randomizer when this item is randomized.
  .align 2 ; Align to the next 4 bytes
.close

.global salvage_corp_give_item_and_set_event_bit
salvage_corp_give_item_and_set_event_bit:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  stw r31, 0xC (sp)
  
  bl fopAcM_createItemForPresentDemo
  
  mr r31, r3 ; Preserve the return value from createItemForPresentDemo so we can still return that
  lis r3,0x1020
  lwz r3,-0x7b24(r3)
  addi r3,r3,0x644
  li r4, 0x6980 ; Unused event bit that we use to keep track of whether the Salvage Corp has given you their item yet or not
  bl onEventBit
  mr r3, r31
  
  lwz r31, 0xC (sp)
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.global maggie_give_item_and_set_event_bit
maggie_give_item_and_set_event_bit:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  stw r31, 0xC (sp)
  
  bl fopAcM_createItemForPresentDemo
  
  mr r31, r3 ; Preserve the return value from createItemForPresentDemo so we can still return that
  lis r3,0x1020
  lwz r3,-0x7b24(r3)
  addi r3,r3,0x644
  li r4, 0x6A01 ; Unused event bit
  bl onEventBit
  mr r3, r31
  
  lwz r31, 0xC (sp)
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.global rito_cafe_postman_start_event_and_set_event_bit
rito_cafe_postman_start_event_and_set_event_bit:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  stw r31, 0xC (sp)
  mr r31, r3 ; Preserve argument r3, which has the Rito postman entity
  
  bl fopAcM_orderOtherEventId
  
  lha r31, 0x9da(r31) ; Load the index of this Rito postman from the Rito postman entity
  cmpwi r31, 0 ; 0 is the one in the Windfall cafe. If it's not that one, we don't want to set the event bit.
  bne rito_cafe_postman_start_event_and_set_event_bit_end
  
  mr r31, r3 ; Preserve the return value from orderOtherEventId so we can still return that (not sure if necessary, but just to be safe)
  lis r3,0x1020
  lwz r3,-0x7b24(r3)
  addi r3,r3,0x644
  li r4, 0x6A02 ; Unused event bit
  bl onEventBit
  mr r3, r31
  
rito_cafe_postman_start_event_and_set_event_bit_end:
  lwz r31, 0xC (sp)
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.global check_play_special_item_get_music
check_play_special_item_get_music:
  
  ; We overwrite the line that branched to the "normal" item get music, it does not need to be replaced since we always branch back to the same area

  ; Check if the item ID (in r0) matches any of the items with special music.
  cmplwi r4, 0x69 ; Nayru's Pearl
  beq play_pearl_item_get_music
  cmplwi r4, 0x6A ; Din's Pearl
  beq play_pearl_item_get_music
  cmplwi r4, 0x6B ; Farore's Pearl
  beq play_pearl_item_get_music
  cmplwi r4, 0x6D ; Wind's Requiem
  beq play_song_get_music
  cmplwi r4, 0x6E ; Ballad of Gales
  beq play_song_get_music
  cmplwi r4, 0x6F ; Command Melody
  beq play_song_get_music
  cmplwi r4, 0x70 ; Earth God's Lyric
  beq play_song_get_music
  cmplwi r4, 0x71 ; Wind God's Aria
  beq play_song_get_music
  cmplwi r4, 0x72 ; Song of Passing
  beq play_song_get_music
  b 0x023dbdb0

play_pearl_item_get_music:
  lis r3, 0x8000004F@ha ; BGM ID for the pearl item get music
  addi r3, r3, 0x8000004F@l
  b 0x023dbdb0 ; Jump to the code that plays the normal item get music

play_song_get_music:
  lis r3, 0x80000027@ha ; BGM ID for the song get music
  addi r3, r3, 0x80000027@l
  b 0x023dbdb0 ; Jump to the code that plays the normal item get music

.global doc_bandam_check_new_potion_and_give_free_item
doc_bandam_check_new_potion_and_give_free_item:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  lwz r0, 0x998 (r30) ; Read the current message ID Doc Bandam is on (r28 has the Doc Bandam entity)
  cmpwi r0, 7627 ; This message ID means he just made a brand new potion for the first time
  ; Any other message ID means he's either giving you or selling you a type he already made before.
  ; So do not give a randomized item in those cases.
  bne doc_bandam_give_item
  
  ; If we're on a newly made potion we need to change the item ID in r4 to be the randomized item
  cmpwi r4, 0x52 ; Green Potion item ID
  beq doc_bandam_set_randomized_green_potion_item_id
  cmpwi r4, 0x53 ; Blue Potion item ID
  beq doc_bandam_set_randomized_blue_potion_item_id
  ; If it's not either of those something unexpected happened, so just give whatever item ID it was originally supposed to give
  b doc_bandam_give_item
  
  doc_bandam_set_randomized_green_potion_item_id:
  lis r4, doc_bandam_green_potion_slot_item_id@ha
  addi r4, r4, doc_bandam_green_potion_slot_item_id@l
  lbz r4, 0 (r4) ; Load what item ID is in the this slot. This value is updated by the randomizer when it randomizes that item.
  b doc_bandam_give_item
  
  doc_bandam_set_randomized_blue_potion_item_id:
  lis r4, doc_bandam_blue_potion_slot_item_id@ha
  addi r4, r4, doc_bandam_blue_potion_slot_item_id@l
  lbz r4, 0 (r4) ; Load what item ID is in the this slot. This value is updated by the randomizer when it randomizes that item.
  
  doc_bandam_give_item:
  bl fopAcM_createItemForPresentDemo
  
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.global doc_bandam_green_potion_slot_item_id
doc_bandam_green_potion_slot_item_id:
  .byte 0x52 ; Default item ID is Green Potion. This value is updated by the randomizer when this item is randomized.
.global doc_bandam_blue_potion_slot_item_id
doc_bandam_blue_potion_slot_item_id:
  .byte 0x53 ; Default item ID is Blue Potion. This value is updated by the randomizer when this item is randomized.
  .align 2 ; Align to the next 4 bytes

.global reset_makar_position_to_start_of_dungeon
reset_makar_position_to_start_of_dungeon:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  bl FUN_025200d4
  lha r8,0x513c(r3)
  
  ; Search through the list of places Makar can spawn from to see which one corresponds to the player's last spawn ID, if any.
  lis r9, makar_possible_wt_spawn_positions@ha
  addi r9, r9, makar_possible_wt_spawn_positions@l
  li r0, 5 ; 5 possible spawn points
  mtctr r0
  makar_spawn_point_search_loop_start:
  lbz r0, 0 (r9) ; Spawn ID
  cmpw r0, r8
  beq reset_makar_found_matching_spawn_point ; Found the array element corresponding to this spawn ID
  addi r9, r9, 0x10 ; Increase pointer to point to next element
  bdnz makar_spawn_point_search_loop_start ; Loop
  
  ; The player came from a spawn that doesn't correspond to any of the elements in the array, don't change Makar's position.
  b after_resetting_makar_position
  
  reset_makar_found_matching_spawn_point:
  lwz r8, 4 (r9) ; X pos
  stw r8, 0 (r5)
  lwz r8, 8 (r9) ; Y pos
  stw r8, 4 (r5)
  lwz r8, 0xC (r9) ; Z pos
  stw r8, 8 (r5)
  
  lha r8, 2 (r9) ; Rotation
  sth r8, 0xC (r5)
  mr r6, r8 ; Argument r6 to setRestartOption needs to be the rotation
  mr r25, r8 ; Also modify the local variable rotation in Makar's code (for when he calls set__19dSv_player_priest)
  
  lbz r8, 1 (r9) ; Room index
  stb r8, 0xE (r5)
  mr r7, r8 ; Argument r7 to setRestartOption needs to be the room index
  mr r26, r8 ; Also modify the local variable room index in Makar's code (for when he calls set__19dSv_player_priest)
  
  after_resetting_makar_position:
  
  bl setRestartOption ; Replace the function call we overwrote to call this custom function
  
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.global makar_possible_wt_spawn_positions
makar_possible_wt_spawn_positions:
  ; Spawn ID, room index, rotation, X pos, Y pos, Z pos
  ; WT entrance
  .byte 15, 0xF
  .short 0x94A0
  .float -3651.02, 1557.67, 13235.2
  ; First WT warp pot
  .byte 22, 0
  .short 0x4000
  .float -4196.33, 754.518, 7448.5
  ; Second WT warp pot
  .byte 23, 2
  .short 0xB000
  .float 668.107, 1550, 2298.75
  ; Third WT warp pot
  .byte 24, 12
  .short 0xC000
  .float 14203.1, -5062.49, 8948.05
  ; Inter-dungeon warp pot in WT
  .byte 69, 3
  .short 0x4000
  .float -4146.65, 1100, 47.88

.global reset_medli_position_to_start_of_dungeon
reset_medli_position_to_start_of_dungeon:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  bl FUN_025200d4
  lha r8,0x513c(r3)
  
  ; Search through the list of places Medli can spawn from to see which one corresponds to the player's last spawn ID, if any.
  lis r9, medli_possible_et_spawn_positions@ha
  addi r9, r9, medli_possible_et_spawn_positions@l
  li r0, 5 ; 5 possible spawn points
  mtctr r0
  medli_spawn_point_search_loop_start:
  lbz r0, 0 (r9) ; Spawn ID
  cmpw r0, r8
  beq reset_medli_found_matching_spawn_point ; Found the array element corresponding to this spawn ID
  addi r9, r9, 0x10 ; Increase pointer to point to next element
  bdnz medli_spawn_point_search_loop_start ; Loop
  
  ; The player came from a spawn that doesn't correspond to any of the elements in the array, don't change Medli's position.
  b after_resetting_medli_position
  
  reset_medli_found_matching_spawn_point:
  lwz r8, 4 (r9) ; X pos
  stw r8, 0 (r5)
  lwz r8, 8 (r9) ; Y pos
  stw r8, 4 (r5)
  lwz r8, 0xC (r9) ; Z pos
  stw r8, 8 (r5)
  
  lha r8, 2 (r9) ; Rotation
  sth r8, 0xC (r5)
  mr r6, r8 ; Argument r6 to setRestartOption needs to be the rotation
  mr r26, r8 ; Also modify the local variable rotation in Medli's code (for when she calls set__19dSv_player_priest)      might need to be r25 instead
  
  lbz r8, 1 (r9) ; Room index
  stb r8, 0xE (r5)
  mr r7, r8 ; Argument r7 to setRestartOption needs to be the room index
  mr r28, r8 ; Also modify the local variable room index in Medli's code (for when she calls set__19dSv_player_priest)      might need to be r26 instead
  
  after_resetting_medli_position:
  
  bl setRestartOption ; Replace the function call we overwrote to call this custom function
  
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.global medli_possible_et_spawn_positions
medli_possible_et_spawn_positions:
  ; Spawn ID, room index, rotation, X pos, Y pos, Z pos
  ; ET entrance
  .byte 0, 0
  .short 0x8000
  .float -7215.21, -200, 5258.79
  ; First ET warp pot
  .byte 22, 2
  .short 0xE000
  .float -2013.11, 200, -1262.97
  ; Second ET warp pot
  .byte 23, 6
  .short 0x8000
  .float 4750, 350, -2251.06
  ; Third ET warp pot
  .byte 24, 15
  .short 0xE000
  .float -2371.38, -2000, 8471.54
  ; Inter-dungeon warp pot in ET
  .byte 69, 1
  .short 0
  .float -8010, 1000, -1508.94

.global check_hyrule_warp_unlocked
check_hyrule_warp_unlocked:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  lis r3,0x1020
  lwz r3,-0x7b24(r3)
  addi r3,r3,0xd4
  
  bl getTriforceNum
  cmpwi r3, 8
  bge hyrule_warp_unlocked
  
  hyrule_warp_not_unlocked:
  li r3, 0
  b hyrule_warp_end
  
  hyrule_warp_unlocked:
  li r3, 1
  
  hyrule_warp_end:
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.global give_pearl_and_raise_totg_if_necessary
give_pearl_and_raise_totg_if_necessary:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  stw r31, 0xC (sp)
  mr r31, r4 ; Preserve argument r4, which has the pearl index to give
  
  bl onSymbol ; Replace the call we overwrote to jump here, which gives the player a specific pearl
  
  lis r3,0x1020
  lwz r3,-0x7b24(r3)
  addi r3,r3,0x644
  
  ; Check the pearl index to know which event flag to set for the pearl being placed
  cmpwi r31, 0
  beq place_nayrus_pearl
  cmpwi r31, 1
  beq place_dins_pearl
  cmpwi r31, 2
  beq place_farores_pearl
  b check_should_raise_totg
  
  place_nayrus_pearl:
  li r4, 0x1410 ; Placed Nayru's Pearl
  bl onEventBit
  b check_should_raise_totg
  
  place_dins_pearl:
  li r4, 0x1480 ; Placed Din's Pearl
  bl onEventBit
  b check_should_raise_totg
  
  place_farores_pearl:
  li r4, 0x1440 ; Placed Farore's Pearl
  bl onEventBit
  
  check_should_raise_totg:
  lis r3,0x1020
  lwz r3,-0x7b24(r3)
  addi r3,r3,0xdf
  lbz r4, 0 (r5)
  cmpwi r4, 7
  bne after_raising_totg ; Only raise TotG if the player owns all 3 pearls
  
  li r4, 0x1E40 ; TOWER_OF_THE_GODS_RAISED
  bl onEventBit
  li r4, 0x2E80 ; PEARL_TOWER_CUTSCENE
  bl onEventBit
  after_raising_totg:
  
  lwz r31, 0xC (sp)
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

;HD is weird so do this differently
;JUMP TO THIS AT 0x025b2030
.global give_temporary_sword_during_ganondorf_fight_in_swordless
give_temporary_sword_during_ganondorf_fight_in_swordless:
  
  bl FUN_025200d4
  addi r3, r3, 0x5133
  lis r4, 0x1005336c@ha
  addi r4, r4, 0x1005336c@l
strcmp_2_start:
  lbzu r5, 0x1(r3)
  lbzu r6, 0x1(r4)
  cmplw r5, r6
  bne after_strcmp_2
  cmplwi r5, 0
  bne strcmp_2_start

after_strcmp_2:
  subf. r3, r5, r6
  bne give_temporary_sword_during_ganondorf_fight_in_swordless_end

  lis r30,0x1020
  lwz r30,-0x7b24(r30)
  lbz r0, 0x2E (r30) ; Read the player's currently equipped sword ID
  cmpwi r0, 0xFF
  ; If the player has any sword equipped, don't replace it with the Hero's Sword
  bne give_temporary_sword_during_ganondorf_fight_in_swordless_end
  
  li r0, 0x38
  stb r0, 0x2E (r30) ; Set the player's currently equipped sword ID to the regular Hero's Sword
  
give_temporary_sword_during_ganondorf_fight_in_swordless_end:
  bl FUN_025200d4
  b 0x025b2034 ; Return

.global give_temporary_sword_in_orcas_house_in_swordless
give_temporary_sword_in_orcas_house_in_swordless:
  bl FUN_025200d4
  addi r3, r3, 0x5133
  lis r4, 0x10003d28@ha ; Pointer to the string "Ojhous", the stage for Orca's house
  addi r4, r4, 0x10003d28@l
  strcmp_3_start:
  lbzu r5, 0x1(r3)
  lbzu r6, 0x1(r4)
  cmplw r5, r6
  bne after_strcmp_3
  cmplwi r5, 0
  bne strcmp_3_start

after_strcmp_3:
  subf r3, r5, r6
  cmpwi r3, 0
  ; If the player did not just enter Orca's house, skip giving a temporary sword
  bne give_temporary_sword_in_orcas_house_in_swordless_end
  
  lis r3,0x1020
  lwz r3,-0x7b24(r3)
  lbz r0, 0x2E (r3) ; Read the player's currently equipped sword ID
  cmpwi r0, 0xFF
  ; If the player has any sword equipped, don't replace it with the Hero's Sword
  bne give_temporary_sword_in_orcas_house_in_swordless_end

  li r0, 0x38
  stb r0, 0x2E (r3) ; Set the player's currently equipped sword ID to the regular Hero's Sword
  
  mr r5, r0
  b 0x025b26fc
  
give_temporary_sword_in_orcas_house_in_swordless_end:
  lwz r4,-0x7b24(r27) ; Replace the line we overwrote to branch here
  b 0x025b26f8 ; Return

.global remove_temporary_sword_when_loading_stage_in_swordless
remove_temporary_sword_when_loading_stage_in_swordless:
  lbz r0, 0xd4 (r4) ; Read the player's owned swords bitfield
  cmpwi r0, 0
  ; If the player owns any sword, don't remove their equipped sword since it's not temporary
  bne remove_temporary_sword_when_loading_stage_in_swordless_end
  
  li r0, 0xFF
  stb r0, 0x2E (r4) ; Set the player's currently equipped sword ID to no sword
  
remove_temporary_sword_when_loading_stage_in_swordless_end:
  lbz r5,0x68(r4) ; Replace the line we overwrote to jump here
  b 0x025b26fc ; Return
