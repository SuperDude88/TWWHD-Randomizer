; Makes functions that play music just return instantly instead.
.org 0x0202204C ; JAIZelBasic::bgmStart
	blr
.org 0x0202879C ; JAIZelBasic::bgmNowBattle
	blr
.org 0x0201DE00 ; JAIZelBasic::subBgmStart
	blr
.org 0x02027F04 ; JAIZelBasic::bgmStreamPlay
	blr
