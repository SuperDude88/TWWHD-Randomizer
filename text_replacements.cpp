#include "text_replacements.hpp"
#include "server/command/Log.hpp"
#include "server/filetypes/util/msbtMacros.hpp"
#include "server/utility/stringUtil.hpp"
#include "server/utility/text.hpp"

TextReplacements generate_text_replacements(World& world)
{
  using namespace Text;
  // Get all relevant items
  auto& auction5           = world.locationEntries["Windfall Island - Auction 5 Rupee"].currentItem;
  auto& auction40          = world.locationEntries["Windfall Island - Auction 40 Rupee"].currentItem;
  auto& auction60          = world.locationEntries["Windfall Island - Auction 60 Rupee"].currentItem;
  auto& auction80          = world.locationEntries["Windfall Island - Auction 80 Rupee"].currentItem;
  auto& auction100         = world.locationEntries["Windfall Island - Auction 100 Rupee"].currentItem;
  auto& splooshFirstPrize  = world.locationEntries["Windfall Island - Battle Squid First Prize"].currentItem;
  auto& splooshSecondPrize = world.locationEntries["Windfall Island - Battle Squid Second Prize"].currentItem;
  auto& savageFloor30      = world.locationEntries["Outset Island - Savage Labyrinth Floor 30"].currentItem;
  auto& savageFloor50      = world.locationEntries["Outset Island - Savage Labyrinth Floor 50"].currentItem;
  auto& beedle20           = world.locationEntries["Great Sea - Beedle Shop 20 Rupee Item"].currentItem;
  auto& beedle500          = world.locationEntries["Rock Spire Isle - Beedle 500 Rupee Item"].currentItem;
  auto& beedle950          = world.locationEntries["Rock Spire Isle - Beedle 950 Rupee Item"].currentItem;
  auto& beedle900          = world.locationEntries["Rock Spire Isle - Beedle 900 Rupee Item"].currentItem;

  LOG_TO_DEBUG("Calculating text replacement articles/pronouns");
  // Calculate articles for some replacements
  std::set<char16_t> vowels = {
    u'a',
    u'e',
    u'i',
    u'o',
    u'u',
    u'à',
    u'â',
    u'æ',
    u'è',
    u'é',
    u'ê',
    u'ë',
    u'ô',
    u'œ',
    u'î',
    u'ï',
    u'ù',
    u'û',
    u'ü',
  };

  // When beedle has these items, the text should say "These <item> are" instead of
  // "This <item> is"
  std::set<std::u16string> beedleTheseDemonstrativeItems = {u"Bombs", u"Power Bracelets"};

  std::u16string beedle500EnglishPronoun   = beedleTheseDemonstrativeItems.contains(beedle500.getUTF16Name("English", Text::Type::STANDARD, Text::Color::RAW)) ? u"These " : u"This ";
  std::u16string beedle500EnglishPlurality = beedleTheseDemonstrativeItems.contains(beedle500.getUTF16Name("English", Text::Type::STANDARD, Text::Color::RAW)) ? u" are " : u" is ";
  std::u16string beedle950EnglishPronoun   = beedleTheseDemonstrativeItems.contains(beedle950.getUTF16Name("English", Text::Type::STANDARD, Text::Color::RAW)) ? u"These " : u"This ";
  std::u16string beedle950EnglishPlurality = beedleTheseDemonstrativeItems.contains(beedle950.getUTF16Name("English", Text::Type::STANDARD, Text::Color::RAW)) ? u" are " : u" is ";
  std::u16string beedle900EnglishPronoun   = beedleTheseDemonstrativeItems.contains(beedle900.getUTF16Name("English", Text::Type::STANDARD, Text::Color::RAW)) ? u"These " : u"This ";
  std::u16string beedle900EnglishPlurality = beedleTheseDemonstrativeItems.contains(beedle900.getUTF16Name("English", Text::Type::STANDARD, Text::Color::RAW)) ? u" are " : u" is ";

  // If the French Pretty Name starts with a vowel, use "d'", otherwise "de "
  std::u16string beedle500FrenchArticle = vowels.contains(beedle500.getUTF16Name("French", Text::Type::PRETTY)[0]) ? u"d'" : u"de ";
  std::u16string beedle950FrenchArticle = vowels.contains(beedle950.getUTF16Name("French", Text::Type::PRETTY)[0]) ? u"d'" : u"de ";
  std::u16string beedle900FrenchArticle = vowels.contains(beedle900.getUTF16Name("French", Text::Type::PRETTY)[0]) ? u"d'" : u"de ";

  // Format for text replacements:
  // Message Label,
  //   English: english translation,
  //   Spanish: spanish translation,
  //   French:  french translation,

  // Swordless text should only be updated if the player is swordless
  bool swordless = world.getSettings().sword_mode == SwordMode::NoSword;

  return {
     // Key Bag
     {"00403",
     {{"English", u"Key Bag\0"s},
      {"Spanish", u"Saco de Llaves\0"s},
      {"French",  u"Sac de Clés\0"s}}},
     {"00603",
     {{"English", u"A handy bag for holding your keys!\nHere's how many you've got with you:\n"s +
                  u"DRC: \x000E\x0007\x004B\x0000         "s +
                  u"FW: \x000E\x0007\x004C\x0000         "s +
                  u"TotG: \x000E\x0007\x004D\x0000     \n"s +
                  u"ET: \x000E\x0007\x004E\x0000           "s +
                  u"WT: \x000E\x0007\x004F\x0000     \0"s},
      {"Spanish", u"Un saco práctico para guardar tus llaves!!\nEstas son las que llevas hasta ahora:\n"s +
                  u"CdD: \x000E\x0007\x004B\x0000         "s +
                  u"BP: \x000E\x0007\x004C\x0000        "s +
                  u"TdlD: \x000E\x0007\x004D\x0000     \n"s +
                  u"TdlT: \x000E\x0007\x004E\x0000        "s +
                  u"TdV: \x000E\x0007\x004F\x0000   \0"s},
      {"French",  u"Un sac bien pratique qui contient vos clés!\nVoici combien vous en avez avec vous:\n"s +
                  u"CdD: \x000E\x0007\x004B\x0000         "s +
                  u"BD: \x000E\x0007\x004C\x0000        "s +
                  u"TdD: \x000E\x0007\x004D\x0000     \n"s +
                  u"TdlT: \x000E\x0007\x004E\x0000        "s +
                  u"TdV: \x000E\x0007\x004F\x0000   \0"s}}},

     // Tingle Statues
     {"00503",
     {{"English", u"Tingle Statues\0"s},
      {"Spanish", u"Estatuas de Tingle\0"s},
      {"French",  u"Statues Tingle\0"s}}},
     {"00703",
     {{"English", u"Golden statues of a mysterious dashing\n figure. They can be traded to " + TEXT_COLOR_RED + u"Ankle" + TEXT_COLOR_DEFAULT + u" on " + TEXT_COLOR_RED + u"\nTingle Island" + TEXT_COLOR_DEFAULT + u" for a reward!\0"s},
      {"Spanish", u"Estatuas doradas de un personaje misterioso\ny muy elegante. Puede ser intercambiado con\n" + TEXT_COLOR_RED + u"Angle" + TEXT_COLOR_DEFAULT + u" en la " + TEXT_COLOR_RED+ u"Isla de Tingle" + TEXT_COLOR_DEFAULT + u" por una recompenza.\0"s},
      {"French",  u"Des statuettes en or d'un mystérieux\npersonnage très pimpant. Elles\npeuvent être échangées avec " + TEXT_COLOR_RED + u"Dingle" + TEXT_COLOR_DEFAULT + u" sur" + TEXT_COLOR_RED + u"\nl'Ile de Tingle" + TEXT_COLOR_DEFAULT + u" contre une récompense.\0"s}}},

     // Flyer telling players which items the auction has
     {"00804",
     {{"English", TEXT_COLOR_DEFAULT + u"Notice: Windfall Auction Tonight!" + TEXT_COLOR_DEFAULT + u"\nBidding starts at dusk. All comers welcome!" + TEXT_COLOR_DEFAULT + u"\n\n\n" +
                  word_wrap_string(u"Participate for the chance to win " + auction5.getUTF16Name("English", Text::Type::PRETTY) + u", " +
                  auction40.getUTF16Name("English", Text::Type::PRETTY) + u", " + auction60.getUTF16Name("English", Text::Type::PRETTY) + u", " +
                  auction80.getUTF16Name("English", Text::Type::PRETTY) + u", and " + auction100.getUTF16Name("English", Text::Type::PRETTY) + u'!', 43) + u'\0'},
      {"Spanish", u""},
      {"French",  TEXT_COLOR_DEFAULT + u"Avis: Enchères de Mercantîle." + TEXT_COLOR_DEFAULT + u"\nLes enchères débuteront à la nuit tombée." + TEXT_COLOR_DEFAULT + u"\nVous y êtes cordialement invités." + TEXT_COLOR_DEFAULT + u"\n\n" +
                  word_wrap_string(u"Participez pour une chance de gagner " + auction5.getUTF16Name("French", Text::Type::PRETTY) + u", " +
                  auction40.getUTF16Name("French", Text::Type::PRETTY) + u", " + auction60.getUTF16Name("French", Text::Type::PRETTY) + u", " +
                  auction80.getUTF16Name("French", Text::Type::PRETTY) + u", et " + auction100.getUTF16Name("French", Text::Type::PRETTY) + u'!', 43) + u'\0'}}},

     // Swordless Text
     {"01128",
     {{"English", !swordless ? u"" : (CAPITAL + REPLACE(ReplaceTags::PLAYER_NAME) + u", you may not have the\nMaster Sword, but do not be afraid!\n\n\nThe hammer of the dead is all you\nneed to crush your foe...\n\n\n" +
                  u"Even as his ball of fell magic bears down\non you, you can " + TEXT_COLOR_RED + u"knock it back\nwith an empty bottle" + TEXT_COLOR_DEFAULT + u"!\n\n...I am sure you will have a shot at victory!" + TEXT_END)},
      {"Spanish", u""},
      {"French",  !swordless ? u"" : (CAPITAL + REPLACE(ReplaceTags::PLAYER_NAME) + u", tu ne détiens peut-être\npas Excalibur, mais n'ait pas peur!\n\n\nLe marteau de la mort est tout ce dont\ntu as besoin afin de vaincre ton ennemi...\n\n\n" +
                  u"EMême lorsque son orbe de magie noire\ns'abat sur toi, tu peux " + TEXT_COLOR_RED + u"la renvoyer\navec un flacon" + TEXT_COLOR_DEFAULT + u"!\n\n... Je suis sûr que tu as une chance de victoire!" + TEXT_END)}}},
     {"01590",
     {{"English", !swordless ? u"" : (CAPITAL + REPLACE(ReplaceTags::PLAYER_NAME) + u"! Do not run! Trust in the\npower of the Skull Hammer!" + TEXT_END)},
      {"Spanish", u""},
      {"French",  !swordless ? u"" : (CAPITAL + REPLACE(ReplaceTags::PLAYER_NAME) + u"!Ne t'enfuis pas! Crois en\nla puissance de la Masse!" + TEXT_END)}}},

     // Savage Labyrinth Hints
     {"00837",
     {{"English", u"\n" + TEXT_SIZE(150) + TEXT_COLOR_RED + u"The Savage Labyrinth" + TEXT_COLOR_DEFAULT + TEXT_SIZE(100) + u"\n\n\n" +
                  word_wrap_string(u"Deep in the never-ending darkness, the way to " + savageFloor30.getUTF16Name("English", Text::Type::CRYPTIC) + u" and " + savageFloor50.getUTF16Name("English", Text::Type::CRYPTIC) + u" await.", 39) + TEXT_END},
      {"Spanish", u""},
      {"French",  u"\n" + TEXT_SIZE(150) + TEXT_COLOR_RED + u"La Crypte Magique" + TEXT_COLOR_DEFAULT + TEXT_SIZE(100) + u"\n\n\n" +
                  word_wrap_string(u"Au plus profond des ténèbres sans fin, le chemin vers " + savageFloor30.getUTF16Name("French", Text::Type::CRYPTIC) + u" et " + savageFloor50.getUTF16Name("French", Text::Type::CRYPTIC) + u" attendent.", 39) + TEXT_END}}},

     // Aryll Pirate Ship Text
     {"03008",
     {{"English", u"'Hoy! Big Brother!\nWanna play a game? It's fun, trust me!\n\n\n" + word_wrap_string(u"Just" + TEXT_COLOR_RED + u" step on this button" + TEXT_COLOR_DEFAULT + u", and try to swing across the ropes to reach that door over there before time's up!", 44) + u'\0'},
      {"Spanish", u""},
      {"French",  u"Grand Frère!\nDis, tu veux jouer à un jeu? Ce sera amusant, je te le promets!\n\n\n" + word_wrap_string(TEXT_COLOR_RED + u"Appuye sur ce bouton" + TEXT_COLOR_DEFAULT + u", et essaye d'atteindre la la pièce du fond avant que le temps ne soit écoulé!", 44) + u'\0'}}},

     // Beedle Rock Spire Mail Text
     {"03325",
     {{"English", u"  I'm sorry to disturb you with this\n  unsolicited letter. If the following does\n  not interest you, please throw the letter\n  away without a second thought.\n\n  BUT THIS IS YOUR BIG CHANCE!!!\n\n\n" +
                    pad_str_4_lines(word_wrap_string(u"Do you have need of " + beedle500.getUTF16Name("English", Text::Type::PRETTY) + u", " + beedle950.getUTF16Name("English", Text::Type::PRETTY) + u", or " +
                    beedle900.getUTF16Name("English", Text::Type::PRETTY) + u"? We have them at special bargain prices.", 39)) + u"\n  BUT WE HAVE ONLY ONE OF EACH!\n\n\n\n  If you're interested, go to the Shop Ship\n  near " + TEXT_COLOR_RED +
                  u"Rock Spire Island" + TEXT_COLOR_DEFAULT + u". First come,\n  first served! I can't wait to serve you!\n\n  To those who took the time to read this\n  letter...please accept my humble thanks.\n     Asst. Manager, Rock Spire Shop Ship" + TEXT_END},
      {"Spanish", u""},
      {"French",  u"  Cher Client,\n  Excusez-nous pour ce courrier\n  inattendu. N'hésitez pas à le\n  détruire s'il vous importune.\n\n  Voici une chance unique, pour vous seul!\n\n\n" +
                    pad_str_4_lines(word_wrap_string(u"Besoin " + beedle500FrenchArticle + beedle500.getUTF16Name("French", Text::Type::PRETTY) + u", " + beedle950FrenchArticle + beedle950.getUTF16Name("French", Text::Type::PRETTY) + u", " +
                    beedle900FrenchArticle + beedle900.getUTF16Name("French", Text::Type::PRETTY) + u"? Nous vous en proposons à des prix exceptionnels!", 39)) + u"\n  Mais attention : nous n'avons qu'un\n  seul article de chaque sorte!\n\n" +
                  u"  Si vous êtes intéressé, n'hésitez pas à\n  venir acheter ces articles dans notre\n  nouvelle boutique, à l'" + TEXT_COLOR_RED + u"île de la Rocaille" + TEXT_COLOR_DEFAULT + u".\n\n" +
                  u"  Nous serons très heureux de vous\n  accueillir.\n\n\n  Nous tenons à vous offrir ce petit cadeau\n  pour vous remercier d'avoir lu ce courrier.\n  Fidèlement vôtre!\n    Boutique Terry de l'Ile de la Rocaille" + TEXT_END}}},

     // Korl Text
     {"03443",
     {{"English", CAPITAL + REPLACE(ReplaceTags::PLAYER_NAME) + u", the sea is all yours.\nMake sure to explore every corner\nin search of items to help you. Remember\nthat your quest is to defeat Ganondorf.\0"},
      {"Spanish", u""},
      {"French",  CAPITAL + REPLACE(ReplaceTags::PLAYER_NAME) + u", l'océan est tout à toi.\nExplore tous ses recoins à la recherche\nd'objets qui t'aideront dans ta quête.\nTa mission est de vaincre Ganondorf!\0"}}},

     // Beedle Shop 20 Rupee Item
     {"03906",
     {{"English", TEXT_COLOR_RED + beedle20.getUTF16Name("English") + u"  20 Rupees" + TEXT_COLOR_DEFAULT + u'\0'},
      {"Spanish", u""},
      {"French",  TEXT_COLOR_RED + beedle20.getUTF16Name("French")  + u"  20 Rubis" + TEXT_COLOR_DEFAULT + u'\0'}}},
     {"03909",
     {{"English", beedle20.getUTF16Name("English") + u"  20 Rupees\nWill you buy it?\n" + TWO_CHOICES + u"I'll buy it\nNo thanks\0"s},
      {"Spanish", u""},
      {"French",  beedle20.getUTF16Name("French")  + u"  20 Rubis\nAcheter?\n" + TWO_CHOICES + u"Oui\nSans façon\0"s}}},

     // Auction 40 Rupee Item
     {"07440",
     {{"English", TEXT_COLOR_RED + auction40.getUTF16Name("English") + TEXT_COLOR_DEFAULT + u'\0'},
      {"Spanish", u""},
      {"French",  u""}}},

     // Auction 5 Rupee Item
     {"07441",
     {{"English", TEXT_COLOR_RED + auction5.getUTF16Name("English") + TEXT_COLOR_DEFAULT + u'\0'},
      {"Spanish", u""},
      {"French",  u""}}},

     // Auction 60 Rupee Item
     {"07442",
     {{"English", TEXT_COLOR_RED + auction60.getUTF16Name("English") + TEXT_COLOR_DEFAULT + u'\0'},
      {"Spanish", u""},
      {"French",  u""}}},

     // Auction 80 Rupee Item
     {"07443",
     {{"English", TEXT_COLOR_RED + auction80.getUTF16Name("English") + TEXT_COLOR_DEFAULT + u'\0'},
      {"Spanish", u""},
      {"French",  u""}}},

     // Auction 100 Rupee Item
     {"07444",
     {{"English", TEXT_COLOR_RED + auction100.getUTF16Name("English") + TEXT_COLOR_DEFAULT + u'\0'},
      {"Spanish", u""},
      {"French",  u""}}},

     // Battlesquid First Prize
     {"07520",
     {{"English", SOUND(0x8E) + u"Hoorayyy! Yayyy! Yayyy!\nOh, thank you, Mr. Sailer!\n\n\n" + word_wrap_string(u"Please take this " + splooshFirstPrize.getUTF16Name("English") +
                                u" as a sign of our gratitude. You are soooooo GREAT!", 43) + u'\0'},
      {"Spanish", u""},
      {"French",  SOUND(0x8E) + u"Oui, oui!\nMerci l'ami!\n\n\n" + word_wrap_string(u"Nous tenons à te faire un cadeau pour te remercier de nous avoir protégés. Voici " +
                                splooshFirstPrize.getUTF16Name("French", Text::Type::PRETTY) + u", prends-le!", 43) + u'\0'}}},

     // Battlesquid Second Prize
     {"07521",
     {{"English", SOUND(0x8E) + u"Hoorayyy! Yayyy! Yayyy!\nOh, thank you so much, Mr. Sailer!\n\n\n" + word_wrap_string(u"This is our thanks to you! It's been passed down on our island for many years, so don't tell the island elder, OK? Here... " +
                                TEXT_COLOR_RED + IMAGE(ImageTags::HEART) + TEXT_COLOR_DEFAULT + u"Please accept this " + splooshSecondPrize.getUTF16Name("English") + u'!', 43) + u'\0'},
      {"Spanish", u""},
      {"French",  SOUND(0x8E) + u"Oui, oui!\nMerci l'ami!\n\n\nNous tenons à te faire un cadeau pour\nte remercier de nous avoir protégés.\n\n\n" + word_wrap_string(u"Voici " +
                                splooshSecondPrize.getUTF16Name("English", Text::Type::PRETTY) + u" qui se transmet depuis fort longtemps sur cette île. Prends-là et surtout ne le dis pas aux anciens!", 43) + u'\0'}}},

     // Rock Spire Shop 500 Rupee Item
     {"12106",
     {{"English", word_wrap_string(TEXT_COLOR_RED + beedle500.getUTF16Name("English") + u"  500 Rupees" + TEXT_COLOR_DEFAULT, 43) + u"\nThis is my last one." + TEXT_END},
      {"Spanish", u""},
      {"French",  word_wrap_string(TEXT_COLOR_RED + beedle500.getUTF16Name("French")  + u".  500 Rubis." + TEXT_COLOR_DEFAULT, 43) + u"\nIl ne me reste que celui-ci!" + TEXT_END}}},
     {"12109",
     {{"English", word_wrap_string(beedle500EnglishPronoun + TEXT_COLOR_RED + beedle500.getUTF16Name("English") + TEXT_COLOR_DEFAULT + beedle500EnglishPlurality + u"a mere " + TEXT_COLOR_RED + u"500 Rupees" + TEXT_COLOR_DEFAULT, 43) + u"\nBuy it! Buy it! Buy buy buy!\n" + TWO_CHOICES + u"I'll buy it\nNo thanks" + TEXT_END},
      {"Spanish", u""},
      {"French",  word_wrap_string(beedle500.getUTF16Name("French") + u" seulement " + TEXT_COLOR_RED + u"500 Rubis" + TEXT_COLOR_DEFAULT, 43) + u".\nAcheter!\n" + TWO_CHOICES + u"Oui\nSans façon" + TEXT_END}}},

     // Rock Spire Shop 950 Rupee Item
     {"12107",
     {{"English", word_wrap_string(TEXT_COLOR_RED + beedle950.getUTF16Name("English") + u"  950 Rupees" + TEXT_COLOR_DEFAULT, 43) + u"\nThis is my last one of these, too." + TEXT_END},
      {"Spanish", u""},
      {"French",  word_wrap_string(TEXT_COLOR_RED + beedle950.getUTF16Name("French") + u".  950 Rubis." + TEXT_COLOR_DEFAULT, 43) + u"\nLà encore, il ne m'en reste juste un." + TEXT_END}}},
     {"12110",
     {{"English", word_wrap_string(beedle950EnglishPronoun + TEXT_COLOR_RED + beedle950.getUTF16Name("English") + TEXT_COLOR_DEFAULT + beedle950EnglishPlurality + u"only " + TEXT_COLOR_RED + u"950 Rupees" + TEXT_COLOR_DEFAULT, 43) + u"\nBuy it! Buy it! Buy buy buy!\n" + TWO_CHOICES + u"I'll buy it\nNo thanks" + TEXT_END},
      {"Spanish", u""},
      {"French",  word_wrap_string(beedle950.getUTF16Name("French") + u" seulement " + TEXT_COLOR_RED + u"950 Rubis" + TEXT_COLOR_DEFAULT, 43) + u".\nAcheter!\n" + TWO_CHOICES + u"Oui\nSans façon" + TEXT_END}}},

     // Rock Spire Shop 900 Rupee Item
     {"12108",
     {{"English", word_wrap_string(TEXT_COLOR_RED + beedle900.getUTF16Name("English") + u"  900 Rupees" + TEXT_COLOR_DEFAULT, 43) + u"\nThe price may be high, but it'll pay\noff handsomely in the end!" + TEXT_END},
      {"Spanish", u""},
      {"French",  word_wrap_string(TEXT_COLOR_RED + beedle900.getUTF16Name("French")  + u".  900 Rubis." + TEXT_COLOR_DEFAULT, 43) + u"\nC'est " + beedle900.getUTF16Name("French") + u", n'est-ce pas...\nDonc quel que soit son prix,\nvous parviendrez à l'amortir..." + TEXT_END}}},
     {"12111",
     {{"English", word_wrap_string(beedle900EnglishPronoun + TEXT_COLOR_RED + beedle900.getUTF16Name("English") + TEXT_COLOR_DEFAULT + beedle900EnglishPlurality + u"just " + TEXT_COLOR_RED + u"500 Rupees" + TEXT_COLOR_DEFAULT, 43) + u"\nBuy it! Buy it! Buy buy buy!\n" + TWO_CHOICES + u"I'll buy it\nNo thanks" + TEXT_END},
      {"Spanish", u""},
      {"French",  word_wrap_string(beedle900.getUTF16Name("French") + u" seulement " + TEXT_COLOR_RED + u"900 Rubis" + TEXT_COLOR_DEFAULT, 43) + u".\nAcheter!\n" + TWO_CHOICES + u"Oui\nSans façon" + TEXT_END}}},
 };
}
