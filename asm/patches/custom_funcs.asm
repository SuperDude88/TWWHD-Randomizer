.org @NextFreeSpace

; save init goes here, has bugs though so not yet

.global num_triforce_shards_to_start_with
num_triforce_shards_to_start_with:
.byte 0 ; By default start with no Triforce Shards
.global should_start_with_heros_clothes
should_start_with_heros_clothes:
.byte 1 ; By default start with the Hero's Clothes
.global sword_mode
sword_mode:
.byte 0 ; By default Start with Sword
.global skip_rematch_bosses
skip_rematch_bosses:
.byte 1 ; By default skip them

.global starting_gear
starting_gear:
.space 47, 0xFF ; Allocate space for up to 47 additional items (when changing this also update the constant in tweaks.py)
.byte 0xFF
.align 1 ; Align to the next 2 bytes
.global starting_quarter_hearts
starting_quarter_hearts:
.short 12 ; By default start with 12 quarter hearts (3 heart containers)
.global starting_magic
starting_magic:
.byte 16 ; By default start with 16 units of magic (small magic meter)

.align 2 ; Align to the next 4 bytes



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

.global progressive_bow_item_func
progressive_bow_item_func:
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

.close