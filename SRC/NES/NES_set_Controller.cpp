
// data base to set the expand controller automaticaly

ex_controller_type = EX_NONE;
//tf_bank = 0;

// Doremikko Keyboard ---------------------------------------------------------
if(fds_id() == 0xa4445245) // Doremikko
{
	SetExControllerType(EX_DOREMIKKO_KEYBOARD);
}
else
	switch(crc){
		// Arkanoid Paddle ------------------------------------------------------------
	case 0x35893b67:	// Arkanoid (J)
	case 0x6267fbd1:	// Arkanoid 2 (J)
		SetExControllerType(EX_ARKANOID_PADDLE);
		break;
		// Crazy Climber Controller ---------------------------------------------------
	case 0xc68363f6:	// Crazy Climber (J)
		SetExControllerType(EX_CRAZY_CLIMBER);
		break;
		// Datach Barcode Battler -----------------------------------------------------
	case 0x983d8175:	// Datach - Battle Rush - Build Up Robot Tournament (J)
	case 0x894efdbc:	// Datach - Crayon Shin Chan - Ora to Poi Poi (J)
	case 0x19e81461:	// Datach - Dragon Ball Z - Gekitou Tenkaichi Budou Kai (J)
	case 0xbe06853f:	// Datach - J League Super Top Players (J)
	case 0x0be0a328:	// Datach - SD Gundam - Gundam Wars (J)
	case 0x5b457641:	// Datach - Ultraman Club - Supokon Fight! (J)
	case 0xf51a7f46:	// Datach - Yuu Yuu Hakusho - Bakutou Ankoku Bujutsu Kai (J)
		SetExControllerType(EX_DATACH_BARCODE_BATTLER);
		break;
		// Exciting Boxing Controller -------------------------------------------------
	case 0x786148b6:	// Exciting Boxing (J)
		SetExControllerType(EX_EXCITING_BOXING);
		break;
		// Family Basic Keyboard with Data Recorder -----------------------------------
		//	case :				// Family BASIC (Ver 1.0)
	case 0xf9def527:	// Family BASIC (Ver 2.0)
	case 0xde34526e:	// Family BASIC (Ver 2.1a)
	case 0xf050b611:	// Family BASIC (Ver 3)
	case 0x3aaeed3f:	// Family BASIC (Ver 3) (Alt)
		//                       )   // Play Box Basic
		SetExControllerType(EX_FAMILY_KEYBOARD);
		break;
		// Family Trainer -------------------------------------------------------------
	case 0x8c8fa83b:	// Family Trainer - Athletic World (J)
	case 0x7e704a14:	// Family Trainer - Jogging Race (J)
	case 0x2330a5d3:	// Family Trainer - Rairai Kyonshiizu (J)
		SetExControllerType(EX_FAMILY_TRAINER_A);
		break;
	case 0xf8da2506:	// Family Trainer - Aerobics Studio (J)
	case 0xca26a0f1:	// Family Trainer - Dai Undoukai (J)
	case 0x28068b8c:	// Family Trainer - Fuuun Takeshi Jou 2 (J)
	case 0x10bb8f9a:	// Family Trainer - Manhattan Police (J)
	case 0xad3df455:	// Family Trainer - Meiro Dai Sakusen (J)
	case 0x8a5b72c0:	// Family Trainer - Running Stadium (J)
	case 0x59794f2d:	// Family Trainer - Totsugeki Fuuun Takeshi Jou (J)
		SetExControllerType(EX_FAMILY_TRAINER_B);
		break;
		// Hyper Shot -----------------------------------------------------------------
	case 0xff6621ce:	// Hyper Olympic (J)
	case 0xdb9418e8:	// Hyper Olympic (Tonosama Ban) (J)
	case 0xac98cd70:	// Hyper Sports (J)
		SetExControllerType(EX_HYPER_SHOT);
		break;
		// Mahjong Controller ---------------------------------------------------------
	case 0x9fae4d46:	// Ide Yousuke Meijin no Jissen Mahjong (J)
	case 0x7b44fb2a:	// Ide Yousuke Meijin no Jissen Mahjong 2 (J)
		SetExControllerType(EX_MAHJONG);
		break;
		// Oeka Kids Tablet -----------------------------------------------------------
	case 0xc3c0811d:	// Oeka Kids - Anpanman no Hiragana Daisuki (J)
	case 0x9d048ea4:	// Oeka Kids - Anpanman to Oekaki Shiyou!! (J)
		SetExControllerType(EX_OEKAKIDS_TABLET);
		break;
		// Optical Gun (Zapper) -------------------------------------------------------
	case 0xfbfc6a6c:	// Adventures of Bayou Billy, The (E)
	case 0xcb275051:	// Adventures of Bayou Billy, The (U)
	case 0xfb69c131:	// Baby Boomer (Unl) (U)
	case 0xf2641ad0:	// Barker Bill's Trick Shooting (U)
	case 0xbc1dce96:	// Chiller (Unl) (U)
	case 0x90ca616d:	// Duck Hunt (JUE)
	case 0x59e3343f:	// Freedom Force (U)
	case 0x242a270c:	// Gotcha! (U)
	case 0x7b5bd2de:	// Gumshoe (UE)
	case 0x255b129c:	// Gun Sight (J)
	case 0x8963ae6e:	// Hogan's Alley (JU)
	case 0x51d2112f:	// Laser Invasion (U)
	case 0x0a866c94:	// Lone Ranger, The (U)
	case 0xe4c04eea:	// Mad City (J)
	case 0x9eef47aa:	// Mechanized Attack (U)
	case 0xc2db7551:	// Shooting Range (U)
	case 0x163e86c0:	// To The Earth (U)
	case 0x389960db:	// Wild Gunman (JUE)
		SetExControllerType(EX_OPTICAL_GUN);
		break;
		// Pokkun Moguraa Controller --------------------------------------------------
	case 0x3993b4eb:	// Pokkun Moguraa (J)
		SetExControllerType(EX_POKKUN_MOGURAA);
		break;
		// Power Pad ------------------------------------------------------------------
	case 0xbc5f6c94:	// Athletic World (U)
		SetExControllerType(EX_POWER_PAD_A);
		break;
	case 0xd836a90b:	// Dance Aerobics (U)
	case 0x96c4ce38:	// Short Order - Eggsplode (U)
	case 0x987dcda3:	// Street Cop (U)
		SetExControllerType(EX_POWER_PAD_B);
		break;
		// Space Shadow Gun (Hyper Shot) ----------------------------------------------
	case 0x0cd00488:	// Space Shadow (J)
		SetExControllerType(EX_SPACE_SHADOW_GUN);
		break;
		// Top Rider Controller -------------------------------------------------------
	case 0x20d22251:	// Top Rider (J)
		//ex_controller_type = EX_TOP_RIDER;
		break;
		// Turbo File -----------------------------------------------------------------
	case 0xe792de94:	// Best Play - Pro Yakyuu (New) (J)
	case 0xf79d684a:	// Best Play - Pro Yakyuu (Old) (J)
		SetExControllerType(EX_TURBO_FILE);
		ex_controller->SetTurboFile_Bank(0);
		break;
	case 0xc2ef3422:	// Best Play - Pro Yakyuu 2 (J)
		SetExControllerType(EX_TURBO_FILE);
		ex_controller->SetTurboFile_Bank(1);
		break;
	case 0x974e8840:	// Best Play - Pro Yakyuu '90 (J)
		SetExControllerType(EX_TURBO_FILE);
		ex_controller->SetTurboFile_Bank(2);
		break;
	case 0xb8747abf:	// Best Play - Pro Yakyuu Special (J)
		SetExControllerType(EX_TURBO_FILE);
		ex_controller->SetTurboFile_Bank(3);
		break;
	case 0x9fa1c11f:	// Castle Excellent (J)
		SetExControllerType(EX_TURBO_FILE);
		ex_controller->SetTurboFile_Bank(4);
		break;
	case 0x0b0d4d1b:	// Derby Stallion - Zenkoku Ban (J)
		SetExControllerType(EX_TURBO_FILE);
		ex_controller->SetTurboFile_Bank(5);
		break;
	case 0x728c3d98:	// Downtown - Nekketsu Monogatari (J)
		SetExControllerType(EX_TURBO_FILE);
		ex_controller->SetTurboFile_Bank(6);
		break;
	case 0xd68a6f33:	// Dungeon Kid (J)
		SetExControllerType(EX_TURBO_FILE);
		ex_controller->SetTurboFile_Bank(7);
		break;
	case 0x3a51eb04:	// Fleet Commander (J)
		SetExControllerType(EX_TURBO_FILE);
		ex_controller->SetTurboFile_Bank(8);
		break;
	case 0x7c46998b:	// Haja no Fuuin (J)
		SetExControllerType(EX_TURBO_FILE);
		ex_controller->SetTurboFile_Bank(9);
		break;
	case 0x7e5d2f1a:	// Itadaki Street - Watashi no Mise ni Yottette (J)
		SetExControllerType(EX_TURBO_FILE);
		ex_controller->SetTurboFile_Bank(10);
		break;
	case 0xcee5857b:	// Ninjara Hoi! (J)
		SetExControllerType(EX_TURBO_FILE);
		ex_controller->SetTurboFile_Bank(11);
		break;
	case 0x50ec5e8b:	// Wizardry - Legacy of Llylgamyn (J)
	case 0x343e9146:	// Wizardry - Proving Grounds of the Mad Overlord (J)
	case 0x33d07e45:	// Wizardry - The Knight of Diamonds (J)
		SetExControllerType(EX_TURBO_FILE);
		ex_controller->SetTurboFile_Bank(12);
		break;
		// VS Unisystem Zapper --------------------------------------------------------
	case 0xed588f00:	// VS Duck Hunt
	case 0x17ae56be:	// VS Freedom Force.nes
	case 0xff5135a3:	// VS Hogan's Alley
		ex_controller_type = EX_VS_ZAPPER;
		break;
	}

