.org 0x028f87f4

.global hurricane_spin_item_resource_arc_name
hurricane_spin_item_resource_arc_name:
.string "Vscroll"
.align 2

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
lbz r4, 0 (r3) 
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
addi r3,r3,0xd5 
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
lbz r4, 0 (r3)
cmpwi r4, 0
beq convert_progressive_bow_id_to_heros_bow
cmpwi r4, 1
beq convert_progressive_bow_id_to_fire_and_ice_arrows
cmpwi r4, 3
beq convert_progressive_bow_id_to_light_arrows
li r3, 0x27 
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
lbz r4, 0 (r3) 
cmpwi r4, 0
beq convert_progressive_wallet_id_to_1000_rupee_wallet
cmpwi r4, 1
beq convert_progressive_wallet_id_to_5000_rupee_wallet
li r3, 0xAB 
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
lbz r4, 6 (r3) 
cmpwi r4, 30
beq convert_progressive_bomb_bag_id_to_60_bomb_bomb_bag
cmpwi r4, 60
beq convert_progressive_bomb_bag_id_to_99_bomb_bomb_bag
li r3, 0xAD 
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
lbz r4, 6 (r3) 
cmpwi r4, 30
beq convert_progressive_quiver_id_to_60_arrow_quiver
cmpwi r4, 60
beq convert_progressive_quiver_id_to_99_arrow_quiver
li r3, 0xAF 
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
lbz r4, 0 (r3) 
cmpwi r4, 0
beq convert_progressive_picto_box_id_to_normal_picto_box
cmpwi r4, 1
beq convert_progressive_picto_box_id_to_deluxe_picto_box
li r3, 0x23 
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
  
  rlwinm     r4,r31,0x0,0x18,0x1f
  
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
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0xd4
lbz r4, 0 (r3)
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
lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global progressive_shield_item_func
progressive_shield_item_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0xd5
lbz r4, 0 (r3)
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
lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global progressive_bow_func
progressive_bow_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x7d
lbz r4, 0 (r3)
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
lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global progressive_wallet_item_func
progressive_wallet_item_func:

lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x32
lbz r4, 0 (r3)
cmpwi r4, 0
beq get_1000_rupee_wallet
cmpwi r4, 1
beq get_5000_rupee_wallet
b wallet_func_end

get_1000_rupee_wallet:
li r4, 1
stb r4, 0 (r3)
b wallet_func_end

get_5000_rupee_wallet:
li r4, 2
stb r4, 0 (r3)

wallet_func_end:
blr

.global progressive_bomb_bag_item_func
progressive_bomb_bag_item_func:

lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x8a
lbz r4, 6 (r3)
cmpwi r4, 30
beq get_60_bomb_bomb_bag
cmpwi r4, 60
beq get_99_bomb_bomb_bag
b bomb_bag_func_end

get_60_bomb_bomb_bag:
li r4, 60
stb r4, 0 (r3)
stb r4, 6 (r3)
b bomb_bag_func_end

get_99_bomb_bomb_bag:
li r4, 99
stb r4, 0 (r3)
stb r4, 6 (r3)

bomb_bag_func_end:
blr

.global progressive_quiver_item_func
progressive_quiver_item_func:

lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x89
lbz r4, 6 (r3)
cmpwi r4, 30
beq get_60_arrow_quiver
cmpwi r4, 60
beq get_99_arrow_quiver
b quiver_func_end

get_60_arrow_quiver:
li r4, 60
stb r4, 0 (r3)
stb r4, 6 (r3)
b quiver_func_end

get_99_arrow_quiver:
li r4, 99
stb r4, 0 (r3)
stb r4, 6 (r3)

quiver_func_end:
blr

.global progressive_picto_box_item_func
progressive_picto_box_item_func:
stwu sp, -0x10 (sp)
mflr r0
stw r0, 0x14 (sp)

lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x76
lbz r4, 0 (r3)
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
lwz r0, 0x14 (sp)
mtlr r0
addi sp, sp, 0x10
blr

.global hurricane_spin_item_func
hurricane_spin_item_func:
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

cmpwi r4, 0xFF
beq custom_createItem_invalid_item_id

mr r9, r5
mr r5, r8
mr r8, r6
mr r6, r9
mr r10, r7
li r7, 3 
li r9, 5 
bl fopAcM_createItem

b custom_createItem_func_end

custom_createItem_invalid_item_id:
li r3, -1 

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

mr r5, r3 
mr r6, r4 

lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x7bc 
lbz r4, 0 (r3)
cmpw r4, r5 
beq generic_on_dungeon_bit_in_correct_dungeon

lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x3a0
mulli r4, r5, 0x24 
add r3, r3, r4
b generic_on_dungeon_bit_func_end

generic_on_dungeon_bit_in_correct_dungeon:
lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x798

generic_on_dungeon_bit_func_end:
mr r4, r6 
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

mr r5, r3 
lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x7bc 
lbz r4, 0 (r3)
cmpw r4, r5 
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

lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x3a0
mulli r4, r5, 0x24 
add r3, r3, r4
lbz r4, 0x20 (r3) 
addi r4, r4, 1
stb r4, 0x20 (r3) 
b generic_small_key_item_get_func_end

generic_small_key_item_get_func_in_non_dungeon_room_of_correct_dungeon:
lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x798
lbz r4, 0x20 (r3) 
addi r4, r4, 1
stb r4, 0x20(r3) 
b generic_small_key_item_get_func_end

generic_small_key_item_get_func_in_correct_dungeon:
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

li r3, 3 
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

li r3, 4 
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

li r3, 5 
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

li r3, 6 
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

li r3, 7 
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

li r3, 3 
li r4, 2 
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

li r3, 4 
li r4, 2 
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

li r3, 5 
li r4, 2 
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

li r3, 6 
li r4, 2 
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

li r3, 7 
li r4, 2 
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

li r3, 3 
li r4, 0 
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

li r3, 4 
li r4, 0 
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

li r3, 5 
li r4, 0 
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

li r3, 2 
li r4, 0 
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

li r3, 6 
li r4, 0 
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

li r3, 7 
li r4, 0 
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

li r3, 3 
li r4, 1 
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

li r3, 4 
li r4, 1 
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

li r3, 5 
li r4, 1 
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

li r3, 2 
li r4, 1 
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

li r3, 6 
li r4, 1 
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

li r3, 7 
li r4, 1 
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
li r4, 0x6A04 
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
li r4, 0x6A08 
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
li r4, 0x6A10 
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
li r4, 0x6A20 
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
li r4, 0x6A40 
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

cmpwi r4, 0xF 
bne check_tingle_statue_owned_invalid


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
li r4, 0x6A04 
b check_tingle_statue_owned_event_bit

check_forbidden_tingle_statue_owned:
li r4, 0x6A08 
b check_tingle_statue_owned_event_bit

check_goddess_tingle_statue_owned:
li r4, 0x6A10 
b check_tingle_statue_owned_event_bit

check_earth_tingle_statue_owned:
li r4, 0x6A20 
b check_tingle_statue_owned_event_bit

check_wind_tingle_statue_owned:
li r4, 0x6A40 

check_tingle_statue_owned_event_bit:
lis r3,0x1020
lwz r3,-0x7b24(r3)
addi r3,r3,0x644
bl isEventBit
b check_tingle_statue_owned_end

check_tingle_statue_owned_invalid:
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

mulli r0, r3, 0x24 
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
  
  hookshot_sight_failsafe:
  li r0,0x0
  li r31, 0x42
  
  hookshot_sight_return:
  lwz r10,0x2e0(r30)
  li r31, 0x42

.global jank_orca_counter_failsafe
jank_orca_counter_failsafe:
  cmpwi r12, 0x38
  bne not_heros_sword
  li r31, 0x42
  not_heros_sword:
  cmpwi r12, 0xFF
  bne master_sword
  li r31, 0x42
  master_sword:
  li r31, 0x42

 .global ultra_sketch_beedle_patch
 ultra_sketch_beedle_patch:
   cmplwi r0, 0x4b
   bne beedle_not_blue_chu_jelly
   li r31, 0xf75
   li r31, 0x42

   beedle_not_blue_chu_jelly:
   blt less_than_val
   li r31, 0x42

   less_than_val:
   li r31, 0x42


.global stalfos_kill_lower_body_when_upper_body_light_arrowed
stalfos_kill_lower_body_when_upper_body_light_arrowed:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  bl fopAcIt_Judge 
  cmplwi r3, 0
  beq stalfos_kill_lower_body_when_upper_body_light_arrowed_end
  
  lbz r0, 0x21f6 (r3) 
  cmpwi r0, 0
  beq stalfos_kill_lower_body_when_upper_body_light_arrowed_end 
  
  lbz r0, 0x21f6 (r31) 
  cmpwi r0, 0
  bne stalfos_kill_lower_body_when_upper_body_light_arrowed_end 
  
  li r0, 1
  stb r0, 0x21f6 (r31) 
  
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
  
  bl dCcD_Sph_Set 
  
  lbz r0, 0x3d0 (r30) 
  cmpwi r0, 0 
  beq miniblin_set_death_switch_when_light_arrowed_end 
  
  
  lbz r0, 0x3d4 (r30) 
  stb r0, 0xaad (r30) 
  
  
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
  
  lbz r0, 0x3a1 (r31) 
  extsb. r0,r0
  ble poe_fix_light_arrows_bug_poe_is_dead 
  
  lbz r0, 0x9a6 (r31) 
  cmpwi r0, 0
  bgt poe_fix_light_arrows_bug_poe_is_dead 
  
  b poe_fix_light_arrows_bug_return_false 
  
  poe_fix_light_arrows_bug_poe_is_dead:
  bl fopAcM_SearchByID 
  

  
  cmpwi r3, 0
  beq poe_fix_light_arrows_bug_return_false
  lwz r12, 0x18 (sp) 
  cmplwi r12, 0
  beq poe_fix_light_arrows_bug_return_false
  cmplwi r12, 0
  beq poe_fix_light_arrows_bug_return_false
  lha r11, 8 (r12)
  cmpwi r11, 0xD3 
  bne poe_fix_light_arrows_bug_return_false
  lha r9,0x562(r12)
  cmpwi r9,0x6f 
  bne poe_fix_light_arrows_bug_unkill_poe
  lha r0,0x56a(r12) 
  cmpwi r0,0x3 
  ble poe_fix_light_arrows_bug_unkill_poe
  lbz r11,0x3a1(r12)
  subi r11,r11,0x1 
  stb r11,0x3a1(r12)
  lwz r9,0x18(sp) 
  lbz r10,0x3a1(r9) 
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
  
  li r9, 0
  stb r9, 0x9a6 (r31)
  
  li r3, 1
  b poe_fix_light_arrows_bug_end
  
  poe_fix_light_arrows_bug_return_false:
  li r3, 0
  
  poe_fix_light_arrows_bug_end:
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr


.global magtail_respawn_when_head_light_arrowed
magtail_respawn_when_head_light_arrowed:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  bl GetTgHitObj 
  

  addi r9,r31,0x1990
  stw r3, 0xd8 (sp) 
  stw r9, 0xec (sp) 
  lwz r5, 0x10 (r3) 
  rlwinm. r4,r5,0x0,0xb,0xb 
  
  beq magtail_respawn_when_head_light_arrowed_end
  li r0, 1
  stb r0, 0x1dd4 (r31)
  
  magtail_respawn_when_head_light_arrowed_end:
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.global phantom_ganon_check_link_within_y_diff
phantom_ganon_check_link_within_y_diff:
  lfs f10, 0x24 (sp) 
  lfs f1, -0x1018(r9) 
  
  fcmpo cr0, f10, f1
  bge phantom_ganon_check_link_within_y_diff_outside_range
  
  fmuls f9,f11,f11 
  li r31, 0x42

phantom_ganon_check_link_within_y_diff_outside_range:
  li r31, 0x42

.global set_shop_item_in_bait_bag_slot_sold_out
set_shop_item_in_bait_bag_slot_sold_out:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  bl SoldOutItem
  
  lis r3,0x1020
  lwz r3,-0x7b24(r3)
  addi r3,r3,0x644
  li r4, 0x6902 
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
  
  lis r3,0x1020
  lwz r3,-0x7b24(r3)
  addi r3,r3,0x644
  li r4, 0x6902
  bl isEventBit
  
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr


.global withered_trees_custom_createitem
withered_trees_custom_createitem:
withered_trees_custom_createitem:
  or. r30,r3,r3
  cmpwi r30,-1
  bne item_created
  li r31, 0x42
  item_created:
  li r31, 0x42

.global create_item_for_withered_trees_without_setting_speeds
create_item_for_withered_trees_without_setting_speeds:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  bl custom_createItem
  
  li r5, 1
  stb r5, 0x32e (r3)
  stb r3, 0x76C (r31)
  
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.global withered_trees_custom_init
withered_trees_custom_init:
  li r11,0xffff
  stw r11,0x76C(r30)
  li r11, 0x1
  li r31,0x42

.global withered_tree_item_try_give_momentum
withered_tree_item_try_give_momentum:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)

  lwz r3, 0x76C (r30)
  cmplwi r3,0xffff
  beq withered_tree_item_try_give_momentum_end
  addi r4,sp,0x8
  fopAcM_SearchByID
  
  cmpwi r3, 0
  beq withered_tree_item_try_give_momentum_end 
  
  lwz r4, 0x8 (sp) 
  cmpwi r4, 0
  beq withered_tree_item_try_give_momentum_end
  
  lbz r5, 0x32e (r3) 
  cmpwi r5, 0
  bne withered_tree_item_try_give_momentum_end 
  

  lis r10, withered_tree_item_speeds@ha
  addi r10, r10, withered_tree_item_speeds@l
  lfs f0, 0 (r10) 
  stfs f0, 0x370 (r4)
  lfs f0, 4 (r10) 
  stfs f0, 0x340 (r4)
  lfs f0, 8 (r10) 
  stfs f0, 0x374 (r4)
  

  lwz r5, 0x2e0 (r4)
  ori r5, r5, 0x40
  stw r5, 0x2e0 (r4)
  lwz r5,0x4(r30)
  stw r5,0x2e0(r4)
  
  li r5, 1
  stb r5, 0x32e (r3)

withered_tree_item_try_give_momentum_end:
  mr r3,r30
  bl daObjFtree_talk_main
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.global withered_tree_item_speeds
withered_tree_item_speeds:
  .float 1.75 
  .float 30 
  .float -2.1 

.global check_ganons_tower_chest_opened
check_ganons_tower_chest_opened:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  li r3, 8 
  li r4, 0 
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
  

  bl isEventBit
  

  cmpwi r3, 0
  beq check_phantom_ganons_sword_should_disappear_end
  

  bl FUN_025200d4
  addi r3, r3, 0x5134
  lis r4, phantom_ganon_maze_stage_name@ha
  addi r4, r4, phantom_ganon_maze_stage_name@l
strcmp_start:
    lbzu r5, 0x1(r3)
    lbzu r6, 0x1(r4)
    cmplw r5, r6
    bne check_phantom_ganons_sword_should_disappear_end
    cmplwi r5, 0
    bne strcmp_start


check_phantom_ganons_sword_should_disappear_end:
  subf. r3, r5, r6
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.global phantom_ganon_maze_stage_name
phantom_ganon_maze_stage_name:
  .string "GanonJ"
  .align 2 

.global create_item_and_set_event_bit_for_townsperson
create_item_and_set_event_bit_for_townsperson:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  stw r31, 0xC (sp)
  mr r31, r4 
  
  clrlwi r4,r4,24 
  bl fopAcM_createItemForPresentDemo
  
  rlwinm. r4,r31,16,16,31 
  beq create_item_and_set_event_bit_for_townsperson_end 
  mr r31, r3 
  lis r3,0x1020
  lwz r3,-0x7b24(r3)
  addi r3,r3,0x644
  bl onEventBit 
  mr r3, r31
  
create_item_and_set_event_bit_for_townsperson_end:
  lwz r31, 0xC (sp)
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.global lenzo_set_deluxe_picto_box_event_bit
lenzo_set_deluxe_picto_box_event_bit:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  stb r10,0xb39(r31)
  
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
  
  bl fopAcM_createItemForPresentDemo
  
  lis r4, zunari_magic_armor_slot_item_id@ha
  addi r4, r4, zunari_magic_armor_slot_item_id@l
  lbz r4, 0 (r4)
  
  cmpw r31, r4 
  bne zunari_give_item_and_set_magic_armor_event_bit_end 
  mr r31, r3 
  lis r3,0x1020
  lwz r3,-0x7b24(r3)
  addi r3,r3,0x644
  li r4, 0x6940
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
  .byte 0x2A
  .align 2 

.global salvage_corp_give_item_and_set_event_bit
salvage_corp_give_item_and_set_event_bit:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  stw r31, 0xC (sp)
  
  bl fopAcM_createItemForPresentDemo
  
  mr r31, r3 
  lis r3,0x1020
  lwz r3,-0x7b24(r3)
  addi r3,r3,0x644
  li r4, 0x6980 
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
  
  mr r31, r3 
  lis r3,0x1020
  lwz r3,-0x7b24(r3)
  addi r3,r3,0x644
  li r4, 0x6A01
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
  mr r31, r3
  
  bl fopAcM_orderOtherEventId
  
  lha r31, 0x9da(r31)
  cmpwi r31, 0 
  bne rito_cafe_postman_start_event_and_set_event_bit_end
  
  mr r31, r3 
  lis r3,0x1020
  lwz r3,-0x7b24(r3)
  addi r3,r3,0x644
  li r4, 0x6A02 
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

  cmplwi r4, 0x69 
  beq play_pearl_item_get_music
  cmplwi r4, 0x6A 
  beq play_pearl_item_get_music
  cmplwi r4, 0x6B 
  beq play_pearl_item_get_music
  cmplwi r4, 0x6D 
  beq play_song_get_music
  cmplwi r4, 0x6E 
  beq play_song_get_music
  cmplwi r4, 0x6F 
  beq play_song_get_music
  cmplwi r4, 0x70 
  beq play_song_get_music
  cmplwi r4, 0x71 
  beq play_song_get_music
  cmplwi r4, 0x72 
  beq play_song_get_music
  li r31, 0x42

play_pearl_item_get_music:
  lis r3, 0x8000004F@ha 
  addi r3, r3, 0x8000004F@l
  li r31, 0x42

play_song_get_music:
  lis r3, 0x80000027@ha 
  addi r3, r3, 0x80000027@l
  li r31, 0x42

.global doc_bandam_check_new_potion_and_give_free_item
doc_bandam_check_new_potion_and_give_free_item:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  lwz r0, 0x998 (r30) 
  cmpwi r0, 7627 
 
  
  bne doc_bandam_give_item
  

  cmpwi r4, 0x52 
  beq doc_bandam_set_randomized_green_potion_item_id
  cmpwi r4, 0x53 
  beq doc_bandam_set_randomized_blue_potion_item_id
  
  b doc_bandam_give_item
  
  doc_bandam_set_randomized_green_potion_item_id:
  lis r4, doc_bandam_green_potion_slot_item_id@ha
  addi r4, r4, doc_bandam_green_potion_slot_item_id@l
  lbz r4, 0 (r4) 
  b doc_bandam_give_item
  
  doc_bandam_set_randomized_blue_potion_item_id:
  lis r4, doc_bandam_blue_potion_slot_item_id@ha
  addi r4, r4, doc_bandam_blue_potion_slot_item_id@l
  lbz r4, 0 (r4) 
  
  doc_bandam_give_item:
  bl fopAcM_createItemForPresentDemo
  
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.global doc_bandam_green_potion_slot_item_id
doc_bandam_green_potion_slot_item_id:
  .byte 0x52 
.global doc_bandam_blue_potion_slot_item_id
doc_bandam_blue_potion_slot_item_id:
  .byte 0x53 
  .align 2 

.global reset_makar_position_to_start_of_dungeon
reset_makar_position_to_start_of_dungeon:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  bl FUN_025200d4
  lha r8,0x513c(r3)
  addi r3,r10,0x1148
  
  
  lis r9, makar_possible_wt_spawn_positions@ha
  addi r9, r9, makar_possible_wt_spawn_positions@l
  li r0, 5 
  mtctr r0
  makar_spawn_point_search_loop_start:
  lbz r0, 0 (r9) 
  cmpw r0, r8
  beq reset_makar_found_matching_spawn_point 
  addi r9, r9, 0x10 
  bdnz makar_spawn_point_search_loop_start 
  
  b after_resetting_makar_position
  
  reset_makar_found_matching_spawn_point:
  lwz r8, 4 (r9) 
  stw r8, 0 (r5)
  lwz r8, 8 (r9) 
  stw r8, 4 (r5)
  lwz r8, 0xC (r9) 
  stw r8, 8 (r5)
  
  lha r8, 2 (r9) 
  sth r8, 0xC (r5)
  mr r6, r8 
  mr r25, r8 
  
  lbz r8, 1 (r9) 
  stb r8, 0xE (r5)
  mr r7, r8 
  mr r26, r8 
  
  after_resetting_makar_position:
  
  bl setRestartOption 
  
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.global makar_possible_wt_spawn_positions
makar_possible_wt_spawn_positions:

  .byte 15, 0xF
  .short 0x94A0
  .float -3651.02, 1557.67, 13235.2

  .byte 22, 0
  .short 0x4000
  .float -4196.33, 754.518, 7448.5

  .byte 23, 2
  .short 0xB000
  .float 668.107, 1550, 2298.75

  .byte 24, 12
  .short 0xC000
  .float 14203.1, -5062.49, 8948.05

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
  addi r3,r10,0x1148

  lis r9, medli_possible_et_spawn_positions@ha
  addi r9, r9, medli_possible_et_spawn_positions@l
  li r0, 5 
  mtctr r0
  medli_spawn_point_search_loop_start:
  lbz r0, 0 (r9) 
  cmpw r0, r8
  beq reset_medli_found_matching_spawn_point 
  addi r9, r9, 0x10 
  bdnz medli_spawn_point_search_loop_start 
  

  b after_resetting_medli_position
  
  reset_medli_found_matching_spawn_point:
  lwz r8, 4 (r9) 
  stw r8, 0 (r5)
  lwz r8, 8 (r9) 
  stw r8, 4 (r5)
  lwz r8, 0xC (r9) 
  stw r8, 8 (r5)
  
  lha r8, 2 (r9) 
  sth r8, 0xC (r5)
  mr r6, r8 
  mr r26, r8 
  
  lbz r8, 1 (r9)
  stb r8, 0xE (r5)
  mr r7, r8 
  mr r28, r8 
  
  after_resetting_medli_position:
  
  bl setRestartOption
  
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.global medli_possible_et_spawn_positions
medli_possible_et_spawn_positions:

  .byte 0, 0
  .short 0x8000
  .float -7215.21, -200, 5258.79

  .byte 22, 2
  .short 0xE000
  .float -2013.11, 200, -1262.97

  .byte 23, 6
  .short 0x8000
  .float 4750, 350, -2251.06

  .byte 24, 15
  .short 0xE000
  .float -2371.38, -2000, 8471.54

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
  mr r31, r4 
  
  bl onSymbol 
  
  lis r3,0x1020
  lwz r3,-0x7b24(r3)
  addi r3,r3,0x644
  

  cmpwi r31, 0
  beq place_nayrus_pearl
  cmpwi r31, 1
  beq place_dins_pearl
  cmpwi r31, 2
  beq place_farores_pearl
  b check_should_raise_totg
  
  place_nayrus_pearl:
  li r4, 0x1410 
  bl onEventBit
  b check_should_raise_totg
  
  place_dins_pearl:
  li r4, 0x1480
  bl onEventBit
  b check_should_raise_totg
  
  place_farores_pearl:
  li r4, 0x1440 
  bl onEventBit
  
  check_should_raise_totg:
  lis r3,0x1020
  lwz r3,-0x7b24(r3)
  addi r3,r3,0xdf
  lbz r4, 0 (r5)
  cmpwi r4, 7
  bne after_raising_totg 
  
  li r4, 0x1E40 
  bl onEventBit
  li r4, 0x2E80 
  bl onEventBit
  after_raising_totg:
  
  lwz r31, 0xC (sp)
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr


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
  lbz r0, 0x2E (r30)
  cmpwi r0, 0xFF
  bne give_temporary_sword_during_ganondorf_fight_in_swordless_end
  
  li r0, 0x38
  stb r0, 0x2E (r30)
  
give_temporary_sword_during_ganondorf_fight_in_swordless_end:
  bl FUN_025200d4
  li r31, 0x42

.global give_temporary_sword_in_orcas_house_in_swordless
give_temporary_sword_in_orcas_house_in_swordless:
  bl FUN_025200d4
  addi r3, r3, 0x5133
  lis r4, 0x10003d28@ha
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
  bne give_temporary_sword_in_orcas_house_in_swordless_end
  
  lis r3,0x1020
  lwz r3,-0x7b24(r3)
  lbz r0, 0x2E (r3)
  cmpwi r0, 0xFF
  bne give_temporary_sword_in_orcas_house_in_swordless_end

  li r0, 0x38
  stb r0, 0x2E (r3) 
  
  mr r5, r0
  li r31, 0x42
  
give_temporary_sword_in_orcas_house_in_swordless_end:
  lwz r4,-0x7b24(r27) 
  li r31, 0x42

.global remove_temporary_sword_when_loading_stage_in_swordless
remove_temporary_sword_when_loading_stage_in_swordless:
  lbz r0, 0xd4 (r4) 
  cmpwi r0, 0
  bne remove_temporary_sword_when_loading_stage_in_swordless_end
  
  li r0, 0xFF
  stb r0, 0x2E (r4)
  
remove_temporary_sword_when_loading_stage_in_swordless_end:
  lbz r5,0x68(r4) 
  li r31, 0x42
