#include "text_replacements.hpp"
#include <command/Log.hpp>
#include <filetypes/util/msbtMacros.hpp>
#include <utility/string.hpp>
#include <utility/text.hpp>

#define IS_PLURAL(item, language) world.itemTranslations[item.getGameItemId()][language].plurality == Plurality::PLURAL
#define IS_SINGULAR(item, language) world.itemTranslations[item.getGameItemId()][language].plurality == Plurality::SINGULAR
#define IS_MALE(item, language) world.itemTranslations[item.getGameItemId()][language].gender == Gender::MALE
#define IS_FEMALE(item, language) world.itemTranslations[item.getGameItemId()][language].gender == Gender::FEMALE

using namespace Text;

static std::u16string get_spanish_conjugation(Item& item)
{
    auto& world = *item.getWorld();
    if (IS_MALE(item, "Spanish") && IS_SINGULAR(item, "Spanish"))
    {
        return u"este ";
    }
    else if (IS_MALE(item, "Spanish") && IS_PLURAL(item, "Spanish"))
    {
        return u"estos ";
    }
    else if (IS_FEMALE(item, "Spanish") && IS_SINGULAR(item, "Spanish"))
    {
        return u"esta ";
    }
    else if (IS_FEMALE(item, "Spanish") && IS_PLURAL(item, "Spanish"))
    {
        return u"estas ";
    }
    return u"";
}

static std::u16string get_english_pronoun(Item& item)
{
    auto& world = *item.getWorld();
    return IS_PLURAL(item, "English") ? u"These " : u"This ";
}

static std::u16string get_english_plurality(Item& item)
{
    auto& world = *item.getWorld();
    return IS_PLURAL(item, "English") ? u" are " : u" is ";
}

TextReplacements generate_text_replacements(World& world)
{
  // Get all relevant items/locations
  auto& auction5           = world.locationTable["Windfall Island - Auction 5 Rupee"]->currentItem;
  auto& auction40          = world.locationTable["Windfall Island - Auction 40 Rupee"]->currentItem;
  auto& auction60          = world.locationTable["Windfall Island - Auction 60 Rupee"]->currentItem;
  auto& auction80          = world.locationTable["Windfall Island - Auction 80 Rupee"]->currentItem;
  auto& auction100         = world.locationTable["Windfall Island - Auction 100 Rupee"]->currentItem;
  auto& splooshFirstPrize  = world.locationTable["Windfall Island - Battle Squid First Prize"]->currentItem;
  auto& splooshSecondPrize = world.locationTable["Windfall Island - Battle Squid Second Prize"]->currentItem;
  auto& savageFloor30      = world.locationTable["Outset Island - Savage Labyrinth Floor 30"]->currentItem;
  auto& savageFloor50      = world.locationTable["Outset Island - Savage Labyrinth Floor 50"]->currentItem;
  auto& beedle20           = world.locationTable["Great Sea - Beedle Shop 20 Rupee Item"]->currentItem;
  auto& beedle500          = world.locationTable["Rock Spire Isle - Beedle 500 Rupee Item"]->currentItem;
  auto& beedle950          = world.locationTable["Rock Spire Isle - Beedle 950 Rupee Item"]->currentItem;
  auto& beedle900          = world.locationTable["Rock Spire Isle - Beedle 900 Rupee Item"]->currentItem;
  auto& octoFairyItem      = world.bigOctoFairyHintLocation->currentItem;
  auto& octoFairyRegion    = world.bigOctoFairyHintLocation->hintRegions.front();

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

  // If the French Pretty Name starts with a vowel, use "d'", otherwise "de "
  std::u16string beedle500FrenchArticle = vowels.contains(beedle500.getUTF16Name("French", Type::PRETTY, Color::NONE)[0]) ? u"d'" : u"de ";
  std::u16string beedle950FrenchArticle = vowels.contains(beedle950.getUTF16Name("French", Type::PRETTY, Color::NONE)[0]) ? u"d'" : u"de ";
  std::u16string beedle900FrenchArticle = vowels.contains(beedle900.getUTF16Name("French", Type::PRETTY, Color::NONE)[0]) ? u"d'" : u"de ";

  // If the beedle item is a plural in English, it should say "These <item> are" instead of "This <item> is"
  std::u16string beedle500EnglishPronoun   = get_english_pronoun(beedle500);
  std::u16string beedle500EnglishPlurality = get_english_plurality(beedle500);
  std::u16string beedle950EnglishPronoun   = get_english_pronoun(beedle950);
  std::u16string beedle950EnglishPlurality = get_english_plurality(beedle950);
  std::u16string beedle900EnglishPronoun   = get_english_pronoun(beedle900);
  std::u16string beedle900EnglishPlurality = get_english_plurality(beedle900);

  // If the last item in a list in spanish with an 'i' change the and conjunction to 'e' instead of 'y'
  auto auction100Spanish = auction60.getUTF16Name("Spanish", Type::PRETTY, Color::NONE);
  std::u16string spanishAuctionFlyerConjunction = auction100Spanish[0] == u'i' || auction100Spanish[0] == u'I' ? u"e " : u"y ";

  auto savageFloor50Spanish = savageFloor50.getUTF16Name("Spanish", Type::CRYPTIC, Color::NONE);
  std::u16string spanishSavageConjunction = savageFloor50Spanish[0] == u'i' || savageFloor50Spanish[0] == u'I' ? u" e " : u" y ";

  // French plurality for Big Octo Fairy Hint
  auto bigOctoFrenchPlurality = IS_SINGULAR(octoFairyItem, "French") ? u"t'aidera" : u"t'aideront";

  // Spanish conjugations for sploosh and beedle items
  std::u16string splooshFirstSpanishConjugation = get_spanish_conjugation(splooshFirstPrize);
  std::u16string splooshSecondSpanishConjugation = get_spanish_conjugation(splooshSecondPrize);
  std::u16string beedle500SpanishConjugation = get_spanish_conjugation(beedle500);
  std::u16string beedle950SpanishConjugation = get_spanish_conjugation(beedle950);
  std::u16string beedle900SpanishConjugation = get_spanish_conjugation(beedle900);

  std::u16string beedle950SpanishPlurality = IS_SINGULAR(beedle950, "Spanish") ? u" cuesta " : u" cuestan ";
  std::u16string beedle900SpanishPlurality = IS_SINGULAR(beedle900, "Spanish") ? u" cuesta " : u" cuestan ";

  // Hint Importance for Savage Labyrinth and Big Octo Fairy items
  auto& savageFloor30Loc = world.locationTable["Outset Island - Savage Labyrinth Floor 30"];
  auto& savageFloor50Loc = world.locationTable["Outset Island - Savage Labyrinth Floor 50"];

  auto savageFloor30EnglishImportance = savageFloor30Loc->generateImportanceText("English");
  auto savageFloor30SpanishImportance = savageFloor30Loc->generateImportanceText("Spanish");
  auto savageFloor30FrenchImportance = savageFloor30Loc->generateImportanceText("French");

  auto savageFloor50EnglishImportance = savageFloor50Loc->generateImportanceText("English");
  auto savageFloor50SpanishImportance = savageFloor50Loc->generateImportanceText("Spanish");
  auto savageFloor50FrenchImportance = savageFloor50Loc->generateImportanceText("French");

  auto bigOctoFairyEnglishImportance = world.bigOctoFairyHintLocation->generateImportanceText("English");
  auto bigOctoFairySpanishImportance = world.bigOctoFairyHintLocation->generateImportanceText("Spanish");
  auto bigOctoFairyFrenchImportance = world.bigOctoFairyHintLocation->generateImportanceText("French");

  // Format for text replacements:
  // Message Label,
  //   English: english translation,
  //   Spanish: spanish translation,
  //   French:  french translation,

  return {
     // FF Warp Text
     {"00076",
     {{"English", u"Warp to " + TEXT_COLOR_RED + u"Forsaken Fortress" + TEXT_COLOR_DEFAULT + u"?" + TEXT_END},
      {"Spanish", u"¿Quieres ir a este lugar?" + TEXT_END},
      {"French",  u"Prendre la Tornade vers " + TEXT_COLOR_RED + u"la Forteresse Maudite" + TEXT_COLOR_DEFAULT + u"?" + TEXT_END}}},

     // Key Bag
     {"00403",
     {{"English", u"Key Bag\0"s},
      {"Spanish", u"Saco de Llaves\0"s},
      {"French",  u"Sac de Clés\0"s}}},
     {"00603",
     {{"English", u"A handy bag for holding your keys!\nHere's how many you've got with you:\n"s +
                  u"DRC: \x000E\x0007\x004B\x0000"s + u"     FW: \x000E\x0007\x004C\x0000"s + u"     TotG: \x000E\x0007\x004D\x0000\n"s +
                  u"ET: \x000E\x0007\x004E\x0000"s + u"     WT: \x000E\x0007\x004F\x0000\0"s},
      {"Spanish", u"¡Un saco práctico para guardar tus llaves!!\nEstas son las que llevas hasta ahora:\n"s +
                  u"CdD: \x000E\x0007\x004B\x0000"s + u"     BP: \x000E\x0007\x004C\x0000"s + u"     TdlD: \x000E\x0007\x004D\x0000\n"s +
                  u"TdlT: \x000E\x0007\x004E\x0000"s + u"     TdV: \x000E\x0007\x004F\x0000\0"s},
      {"French",  u"Un sac bien pratique qui contient vos clés!\nVoici combien vous en avez avec vous:\n"s +
                  u"CdD: \x000E\x0007\x004B\x0000"s + u"     BD: \x000E\x0007\x004C\x0000"s + u"     TdD: \x000E\x0007\x004D\x0000\n"s +
                  u"TdlT: \x000E\x0007\x004E\x0000"s + u"     TdV: \x000E\x0007\x004F\x0000\0"s}}},

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
     {{"English", TEXT_COLOR_DEFAULT + u"Notice: Windfall Auction Tonight!" + TEXT_COLOR_DEFAULT + u"\nBidding starts at dusk. All comers welcome!\nParticipate for the chance to win" + TEXT_COLOR_DEFAULT + u"\n\n" +
                  auction5.getUTF16Name("English", Text::Type::PRETTY) + u",\n" +
                  auction40.getUTF16Name("English", Text::Type::PRETTY) + u",\n" + auction60.getUTF16Name("English", Text::Type::PRETTY) + u",\n\n\n" +
                  auction80.getUTF16Name("English", Text::Type::PRETTY) + u", and\n" + auction100.getUTF16Name("English", Text::Type::PRETTY) + u'!' + u'\0'},
      {"Spanish", TEXT_COLOR_RED + u"¡Aviso! Gran subasta de Taura" + TEXT_COLOR_DEFAULT + u"\nHorario: Tras la puesta del sol\n¡Esperamos su visita!\n¡Participa por una oportunidad de ganar" + TEXT_COLOR_DEFAULT + u"\n" +
                  auction5.getUTF16Name("Spanish", Text::Type::PRETTY) + u",\n" +
                  auction40.getUTF16Name("Spanish", Text::Type::PRETTY) + u",\n" + auction60.getUTF16Name("Spanish", Text::Type::PRETTY) + u",\n\n\n" +
                  auction80.getUTF16Name("Spanish", Text::Type::PRETTY) + u"," + spanishAuctionFlyerConjunction + u"\n" + auction100.getUTF16Name("Spanish", Text::Type::PRETTY) + u'!' + u'\0'},
      {"French",  TEXT_COLOR_DEFAULT + u"Avis: Enchères de Mercantîle." + TEXT_COLOR_DEFAULT + u"\nLes enchères débuteront à la nuit tombée." + TEXT_COLOR_DEFAULT + u"\nVous y êtes cordialement invités.\nParticipez pour une chance de gagner" + TEXT_COLOR_DEFAULT + u"\n" +
                  auction5.getUTF16Name("French", Text::Type::PRETTY) + u",\n" +
                  auction40.getUTF16Name("French", Text::Type::PRETTY) + u",\n" + auction60.getUTF16Name("French", Text::Type::PRETTY) + u",\n\n\n" +
                  auction80.getUTF16Name("French", Text::Type::PRETTY) + u", et\n" + auction100.getUTF16Name("French", Text::Type::PRETTY) + u'!' + u'\0'}}},

     // Aryll Save Text
     {"00849",
     {{"English", u"Oh! Did you get stuck in there, Big Brother?\0"s},
      {"Spanish", u"¡Ah! ¿Te atascaste, Hermanote?\0"s},
      {"French",  u"Oh! Es-tu coincé, Grand Frère ?\0"s}}},
     {"00850",
     {{"English", u"Don't worry, I'll open the door for you.\0"s},
      {"Spanish", u"No pasa nada. Abriré la puerta por ti.\0"s},
      {"French",  u"Ne t'inquiète pas, je vais te sortir de là.\0"s}}},

     // Swordless Text
     {"01128",
     {{"English", !world.getSettings().remove_swords ? u"" : (CAPITAL + REPLACE(ReplaceTags::PLAYER_NAME) + u", you may not have the\nMaster Sword, but do not be afraid!\n\n\nThe hammer of the dead is all you\nneed to crush your foe...\n\n\n" +
                  u"Even as his ball of fell magic bears down\non you, you can " + TEXT_COLOR_RED + u"knock it back\nwith an empty bottle" + TEXT_COLOR_DEFAULT + u"!\n\n...I am sure you will have a shot at victory!" + TEXT_END)},
      {"Spanish", !world.getSettings().remove_swords ? u"" : (u"¡" + CAPITAL + REPLACE(ReplaceTags::PLAYER_NAME) + u", puede que no\ntengas la espada maestra, pero no tengas miedo!\n\n\nEl martillo es todo lo que necesitas\npara aplastar a tus enemigos...\n\n\n" +
                  u"Incluso cuando su bola de magia desciende hacia ti,\n¡puedes " + TEXT_COLOR_RED + u"devolverla con una botella vacía" + TEXT_COLOR_DEFAULT + u"!\n\n...tengo la seguridad de que puedes ganar!" + TEXT_END)},
      {"French",  !world.getSettings().remove_swords ? u"" : (CAPITAL + REPLACE(ReplaceTags::PLAYER_NAME) + u", tu ne détiens peut-être\npas Excalibur, mais n'ait pas peur!\n\n\nLe marteau de la mort est tout ce dont\ntu as besoin afin de vaincre ton ennemi...\n\n\n" +
                  u"Même lorsque son orbe de magie noire\ns'abat sur toi, tu peux " + TEXT_COLOR_RED + u"la renvoyer\navec un flacon" + TEXT_COLOR_DEFAULT + u"!\n\n... Je suis sûr que tu as une chance de victoire!" + TEXT_END)}}},
     {"01590",
     {{"English", !world.getSettings().remove_swords ? u"" : (CAPITAL + REPLACE(ReplaceTags::PLAYER_NAME) + u"! Do not run! Trust in the\npower of the Skull Hammer!" + TEXT_END)},
      {"Spanish", !world.getSettings().remove_swords ? u"" : (u"¡" + CAPITAL + REPLACE(ReplaceTags::PLAYER_NAME) + u"¡No huyas! ¡Confía en el\npoder del Martillo!" + TEXT_END)},
      {"French",  !world.getSettings().remove_swords ? u"" : (CAPITAL + REPLACE(ReplaceTags::PLAYER_NAME) + u"!Ne t'enfuis pas! Crois en\nla puissance de la Masse!" + TEXT_END)}}},

     // KoRL Barren Dungeon Text
     {"01509",
     {{"English", CAPITAL + REPLACE(ReplaceTags::PLAYER_NAME) + u", this area is unrequired and\nthere are no valuable items in here. You can\nskip it in full confidence." + TEXT_END},
      {"Spanish", CAPITAL + REPLACE(ReplaceTags::PLAYER_NAME) + u", this area is unrequired and\nthere are no valuable items in here. You can\nskip it in full confidence." + TEXT_END},
      {"French",  CAPITAL + REPLACE(ReplaceTags::PLAYER_NAME) + u", this area is unrequired and\nthere are no valuable items in here. You can\nskip it in full confidence." + TEXT_END}}},

     // Savage Labyrinth Hints
     {"00837",
     {{"English", u"\n" + TEXT_SIZE(150) + TEXT_COLOR_RED + u"The Savage Labyrinth" + TEXT_COLOR_DEFAULT + TEXT_SIZE(100) + u"\n\n\n" +
                  word_wrap_string(u"Deep in the never-ending darkness, the way to " + savageFloor30.getUTF16Name("English", Text::Type::CRYPTIC) + savageFloor30EnglishImportance + u" and " + savageFloor50.getUTF16Name("English", Text::Type::CRYPTIC) + savageFloor50EnglishImportance + u" await.", 39) + TEXT_END},
      {"Spanish", u"\n" + TEXT_SIZE(150) + TEXT_COLOR_RED + u"Cripta de los Mounstros" + TEXT_COLOR_DEFAULT + TEXT_SIZE(100) + u"\n\n\n" +
                  word_wrap_string(u"En las profundidades de la interminable oscuridad, el camino hacia " + savageFloor30.getUTF16Name("Spanish", Text::Type::CRYPTIC) + savageFloor30SpanishImportance + spanishSavageConjunction + savageFloor50.getUTF16Name("Spanish", Text::Type::CRYPTIC) + savageFloor50SpanishImportance + u" esperan.", 39) + TEXT_END},
      {"French",  u"\n" + TEXT_SIZE(150) + TEXT_COLOR_RED + u"La Crypte Magique" + TEXT_COLOR_DEFAULT + TEXT_SIZE(100) + u"\n\n\n" +
                  word_wrap_string(u"Au plus profond des ténèbres sans fin, le chemin vers " + savageFloor30.getUTF16Name("French", Text::Type::CRYPTIC) + savageFloor30FrenchImportance + u" et " + savageFloor50.getUTF16Name("French", Text::Type::CRYPTIC) + savageFloor50FrenchImportance + u" attendent.", 39) + TEXT_END}}},

     // Aryll Pirate Ship Text
     {"03008",
     {{"English", u"'Hoy! Big Brother!\nWanna play a game? It's fun, trust me!\n\n\n" + word_wrap_string(u"Just" + TEXT_COLOR_RED + u" step on this button" + TEXT_COLOR_DEFAULT + u", and try to swing across the ropes to reach that door over there before time's up!", 44) + u'\0'},
      {"Spanish", CAPITAL + REPLACE(ReplaceTags::PLAYER_NAME) + u"!\n¿Quieres jugar? será divertido, ¡confía en mi!\n\n\n" + word_wrap_string(u"¡Solo tienes que " + TEXT_COLOR_RED + u"pisar este interruptor" + TEXT_COLOR_DEFAULT + u", y colgarte de las cuerdas hasta llegar a aquella puerta antes de que se acabe el tiempo!", 44) + u'\0'},
      {"French",  u"Grand Frère!\nDis, tu veux jouer à un jeu? Ce sera amusant, je te le promets!\n\n\n" + word_wrap_string(TEXT_COLOR_RED + u"Appuye sur ce bouton" + TEXT_COLOR_DEFAULT + u", et essaye d'atteindre la la pièce du fond avant que le temps ne soit écoulé!", 44) + u'\0'}}},

     // Beedle Rock Spire Mail Text
     {"03325",
     {{"English", u"  I'm sorry to disturb you with this\n  unsolicited letter. If the following does\n  not interest you, please throw the letter\n  away without a second thought.\n\n  BUT THIS IS YOUR BIG CHANCE!!!\n\n\n" +
                    pad_str_4_lines(word_wrap_string(u"Do you have need of " + beedle500.getUTF16Name("English", Text::Type::PRETTY) + u", " + beedle950.getUTF16Name("English", Text::Type::PRETTY) + u", or " +
                    beedle900.getUTF16Name("English", Text::Type::PRETTY) + u"? We have them at special bargain prices.", 39)) + u"\n  BUT WE HAVE ONLY ONE OF EACH!\n\n\n\n  If you're interested, go to the Shop Ship\n  near " + TEXT_COLOR_RED +
                  u"Rock Spire Island" + TEXT_COLOR_DEFAULT + u". First come,\n  first served! I can't wait to serve you!\n\n  To those who took the time to read this\n  letter...please accept my humble thanks.\n     Asst. Manager, Rock Spire Shop Ship" + TEXT_END},
      {"Spanish", u"  Le pedimos disculpas si esta\n  comunicación no es de su interés.\n  Si es así, por favor, ignore esta carta.\n\n\n¡Gran oferta exclusiva!\n\n\n  - " +
                    beedle500.getUTF16Name("Spanish") + u"\n  - " + beedle950.getUTF16Name("Spanish") + u"\n  - " + beedle900.getUTF16Name("Spanish") +
                  u"\n\n  Tres artículos de gran valor, a precios\n  de promoción.\n\n\n  * Solo disponemos de uno de cada tipo.\n\n\n\n  Lo esperamos en las proximidades\n  de la " + TEXT_COLOR_RED +
                  u"Isla de los Pilares" + TEXT_COLOR_DEFAULT + u".\n  ¡No deje escapar esta oportunidad!\n\n  Le agradecemos que se haya tomado\n  el tiempo de leer esta carta.\n     Jefe suplente\n     Sucursal Isla de los Pilares" + TEXT_END},
      {"French",  u"  Cher Client,\n  Excusez-nous pour ce courrier\n  inattendu. N'hésitez pas à le\n  détruire s'il vous importune.\n\n  Voici une chance unique, pour vous seul!\n\n\n" +
                    pad_str_4_lines(word_wrap_string(u"Besoin " + beedle500FrenchArticle + beedle500.getUTF16Name("French", Text::Type::PRETTY) + u", " + beedle950FrenchArticle + beedle950.getUTF16Name("French", Text::Type::PRETTY) + u", " +
                    beedle900FrenchArticle + beedle900.getUTF16Name("French", Text::Type::PRETTY) + u"? Nous vous en proposons à des prix exceptionnels!", 39)) + u"\n  Mais attention : nous n'avons qu'un\n  seul article de chaque sorte!\n\n" +
                  u"  Si vous êtes intéressé, n'hésitez pas à\n  venir acheter ces articles dans notre\n  nouvelle boutique, à l'" + TEXT_COLOR_RED + u"île de la Rocaille" + TEXT_COLOR_DEFAULT + u".\n\n" +
                  u"  Nous serons très heureux de vous\n  accueillir.\n\n\n  Nous tenons à vous offrir ce petit cadeau\n  pour vous remercier d'avoir lu ce courrier.\n  Fidèlement vôtre!\n    Boutique Terry de l'Ile de la Rocaille" + TEXT_END}}},

     // Korl Text
     {"03443",
     {{"English", CAPITAL + REPLACE(ReplaceTags::PLAYER_NAME) + u", the sea is all yours.\nMake sure to explore every corner\nin search of items to help you. Remember\nthat your " TEXT_COLOR_RED u"quest" TEXT_COLOR_DEFAULT u" is to " TEXT_COLOR_RED u"defeat Ganondorf" TEXT_COLOR_DEFAULT u".\0"s},
      {"Spanish", CAPITAL + REPLACE(ReplaceTags::PLAYER_NAME) + u", el océano es todo tuyo.\nAsegúrate de explorar cada rincón\npor objetos que te sean de ayuda. Recuerda,\ntu " TEXT_COLOR_RED u"misión" TEXT_COLOR_DEFAULT u" es " TEXT_COLOR_RED u"derrotar a Ganondorf" TEXT_COLOR_DEFAULT u".\0"s},
      {"French",  CAPITAL + REPLACE(ReplaceTags::PLAYER_NAME) + u", l'océan est tout à toi.\nExplore tous ses recoins à la recherche\nd'objets qui t'aideront dans ta quête.\nTa " TEXT_COLOR_RED u"mission" TEXT_COLOR_DEFAULT u" est de " TEXT_COLOR_RED u"vaincre Ganondorf" TEXT_COLOR_DEFAULT u"!\0"s}}},

     // Beedle Shop 20 Rupee Item
     {"03906",
     {{"English", TEXT_COLOR_RED + beedle20.getUTF16Name("English") + u"  20 Rupees" + TEXT_COLOR_DEFAULT + u'\0'},
      {"Spanish", TEXT_COLOR_RED + beedle20.getUTF16Name("Spanish") + u"  20 Rupias" + TEXT_COLOR_DEFAULT + u'\0'},
      {"French",  TEXT_COLOR_RED + beedle20.getUTF16Name("French")  + u"  20 Rubis" + TEXT_COLOR_DEFAULT + u'\0'}}},
     {"03909",
     {{"English", beedle20.getUTF16Name("English") + u"  20 Rupees\nWill you buy it?\n" + TWO_CHOICES + u"I'll buy it\nNo thanks\0"s},
      {"Spanish", beedle20.getUTF16Name("Spanish") + u"  20 Rupias\n¿Te lo llevas?\n" + TWO_CHOICES + u"Sí\nNo\0"s},
      {"French",  beedle20.getUTF16Name("French")  + u"  20 Rubis\nAcheter?\n" + TWO_CHOICES + u"Oui\nSans façon\0"s}}},

     // Beedle Spoil Sell Text
     {"03957",
     {{"English", u"I can't buy that! I only buy things that fit\nin a " + TEXT_COLOR_RED + u"Spoils Bag" + TEXT_COLOR_DEFAULT + u".\nAnd no Blue Chu Jelly, either!\n\n\nDon't you have anything else?" + TEXT_END},
      {"Spanish", u"No te puedo comprar eso...\nEstoy buscando objetos que\nquepan en una " + TEXT_COLOR_RED + u"bolsa de trofeos" + TEXT_COLOR_DEFAULT + u".\n¡Y tampoco Jugo de ChuChu Azul!\n\n\n¿No tienes nada, amiigo?" + TEXT_END},
      {"French",  u"Je ne peux pas vous acheter ça!\nJe ne peux acheter que les objets\nqui sont rangés dans un " + TEXT_COLOR_RED + u"sac à butin" + TEXT_COLOR_DEFAULT + u".\nEt pas de Gelée Chuchu Bleue non plus!\n\n\nVous n'avez rien d'autre à me proposer?" + TEXT_END}}},
     // Auction Transition Text
     {"07412",
     {{"English", REPLACE(ReplaceTags::AUCTION_ITEM_NAME) + u"!!!" + WAIT(0x1E) + SOUND(0x46) + TEXT_END},
      {"Spanish", u"¡¡" + REPLACE(ReplaceTags::AUCTION_ITEM_NAME) + u"!!" + WAIT(0x1E) + SOUND(0x46) + TEXT_END},
      {"French",  REPLACE(ReplaceTags::AUCTION_ITEM_NAME) + u"!!!" + WAIT(0x1E) + SOUND(0x46) + TEXT_END}}},

     // Auction 40 Rupee Item
     {"07440",
     {{"English", CAPITAL + auction40.getUTF16Name("English", Type::PRETTY) + u'\0'},
      {"Spanish", auction40.getUTF16Name("Spanish", Type::PRETTY) + u'\0'},
      {"French",  CAPITAL + auction40.getUTF16Name("French", Type::PRETTY) + u'\0'}}},

     // Auction 5 Rupee Item
     {"07441",
     {{"English", CAPITAL + auction5.getUTF16Name("English", Type::PRETTY) + u'\0'},
      {"Spanish", auction5.getUTF16Name("Spanish", Type::PRETTY) + u'\0'},
      {"French",  CAPITAL + auction5.getUTF16Name("French", Type::PRETTY) + u'\0'}}},

     // Auction 60 Rupee Item
     {"07442",
     {{"English", CAPITAL + auction60.getUTF16Name("English", Type::PRETTY) + u'\0'},
      {"Spanish", auction60.getUTF16Name("Spanish", Type::PRETTY) + u'\0'},
      {"French",  CAPITAL + auction60.getUTF16Name("French", Type::PRETTY) + u'\0'}}},

     // Auction 80 Rupee Item
     {"07443",
     {{"English", CAPITAL + auction80.getUTF16Name("English", Type::PRETTY) + u'\0'},
      {"Spanish", auction80.getUTF16Name("Spanish", Type::PRETTY) + u'\0'},
      {"French",  CAPITAL + auction80.getUTF16Name("French", Type::PRETTY) + u'\0'}}},

     // Auction 100 Rupee Item
     {"07444",
     {{"English", CAPITAL + auction100.getUTF16Name("English", Type::PRETTY) + u'\0'},
      {"Spanish", auction100.getUTF16Name("Spanish", Type::PRETTY) + u'\0'},
      {"French",  CAPITAL + auction100.getUTF16Name("French", Type::PRETTY) + u'\0'}}},

     // Battlesquid First Prize
     {"07520",
     {{"English", SOUND(0x8E) + u"Hoorayyy! Yayyy!\nOh, thank you, Mr. Sailer!\n\n\n" + word_wrap_string(u"Please take this " + splooshFirstPrize.getUTF16Name("English") +
                                u" as a sign of our gratitude. You are soooooo GREAT!", 43) + u'\0'},
      {"Spanish", SOUND(0x8E) + u"¡Viva, viva!\n¡Gracias, señor, gracias!\n\n\n" + word_wrap_string(u"Por favor, " + splooshFirstSpanishConjugation + splooshFirstPrize.getUTF16Name("Spanish") +
                                 u" como muestra de gratitud.", 43) + u'\0'},
      {"French",  SOUND(0x8E) + u"Oui, oui!\nMerci l'ami!\n\n\n" + word_wrap_string(u"Nous tenons à te faire un cadeau pour te remercier de nous avoir protégés. Voici " +
                                splooshFirstPrize.getUTF16Name("French", Text::Type::PRETTY) + u", prends-le!", 43) + u'\0'}}},

     // Battlesquid Second Prize
     {"07521",
     {{"English", SOUND(0x8E) + u"Hoorayyy! Yayyy!\nOh, thank you so much, Mr. Sailer!\n\n\n" + word_wrap_string(u"This is our thanks to you! It's been passed down on our island for many years, so don't tell the island elder, OK? Here... " +
                                TEXT_COLOR_RED + IMAGE(ImageTags::HEART) + TEXT_COLOR_DEFAULT + u"Please accept this " + splooshSecondPrize.getUTF16Name("English") + u'!', 43) + u'\0'},
      {"Spanish", SOUND(0x8E) + u"¡Viva, viva!\n¡Gracias, señor, gracias!\n\n\n" + word_wrap_string(u"Esta es nuestra muestra de gratitud. Ha sido pasado de generación en generación, así que no le digas a los ancianos de la isla, está bien? ¡Por favor!" +
                                 TEXT_COLOR_RED + IMAGE(ImageTags::HEART) + TEXT_COLOR_DEFAULT + u"¡Acepta " + splooshSecondSpanishConjugation + splooshSecondPrize.getUTF16Name("Spanish") + u'!', 43) + u'\0'},
      {"French",  SOUND(0x8E) + u"Oui, oui!\nMerci l'ami!\n\n\nNous tenons à te faire un cadeau pour\nte remercier de nous avoir protégés.\n\n\n" + word_wrap_string(u"Voici " +
                                splooshSecondPrize.getUTF16Name("English", Text::Type::PRETTY) + u" qui se transmet depuis fort longtemps sur cette île. Prends-là et surtout ne le dis pas aux anciens!", 43) + u'\0'}}},

     // Great Fairy Big Octo Text
     {"12015",
     {{"English", word_wrap_string(TEXT_COLOR_CYAN + u"At " + world.getUTF16HintRegion(octoFairyRegion, "English", Text::Type::PRETTY, Text::Color::RED) + TEXT_COLOR_CYAN + u", you will find an item.", 43) + u'\0'},
      {"Spanish", word_wrap_string(TEXT_COLOR_CYAN + u"Encontrarás un objeto en " + world.getUTF16HintRegion(octoFairyRegion, "Spanish", Text::Type::PRETTY, Text::Color::RED) + TEXT_COLOR_CYAN + u".", 43) + u'\0'},
      {"French",  word_wrap_string(TEXT_COLOR_CYAN + world.getUTF16HintRegion(octoFairyRegion, "French", Text::Type::PRETTY, Text::Color::RED) + TEXT_COLOR_CYAN + u" détient un objet.", 43) + u'\0'}}},
     {"12016",
     {{"English", word_wrap_string(TEXT_COLOR_CYAN + u"..." + octoFairyItem.getUTF16Name("English", Text::Type::PRETTY) + bigOctoFairyEnglishImportance + TEXT_COLOR_CYAN + u", which may help you on your quest.", 43) + u'\0'},
      {"Spanish", word_wrap_string(TEXT_COLOR_CYAN + u"..." + octoFairyItem.getUTF16Name("Spanish", Text::Type::PRETTY) + bigOctoFairySpanishImportance + TEXT_COLOR_CYAN + u", que te ayudará en tu aventura.", 43) + u'\0'},
      {"French",  word_wrap_string(TEXT_COLOR_CYAN + u"..." + octoFairyItem.getUTF16Name("French", Text::Type::PRETTY) + bigOctoFairyFrenchImportance + TEXT_COLOR_CYAN + u", qui " + bigOctoFrenchPlurality + u" dans ton aventure.", 43) + u'\0'}}},
     {"12017",
     {{"English", word_wrap_string(TEXT_COLOR_CYAN + u"When you find you have need of such an item, you must journey to that place.", 43) + u'\0'},
      {"Spanish", word_wrap_string(TEXT_COLOR_CYAN + u"Cuándo requieras el uso de tal objeto, dirígete a ese lugar.", 43) + u'\0'},
      {"French",  word_wrap_string(TEXT_COLOR_CYAN + u"Lorsque tu auras besoin de cet objet, tu devras te rendre en ce lieu..", 43) + u'\0'}}},

     // Rock Spire Shop 500 Rupee Item
     {"12106",
     {{"English", word_wrap_string(TEXT_COLOR_RED + beedle500.getUTF16Name("English") + u"  500 Rupees" + TEXT_COLOR_DEFAULT, 43) + u"\nThis is my last one." + TEXT_END},
      {"Spanish", word_wrap_string(TEXT_COLOR_RED + beedle500.getUTF16Name("Spanish") + u"  500 Rupias" + TEXT_COLOR_DEFAULT, 43) + u"\n¡Queda solo una!" + TEXT_END},
      {"French",  word_wrap_string(TEXT_COLOR_RED + beedle500.getUTF16Name("French")  + u".  500 Rubis." + TEXT_COLOR_DEFAULT, 43) + u"\nIl ne me reste que celui-ci!" + TEXT_END}}},
     {"12109",
     {{"English", word_wrap_string(beedle500EnglishPronoun + beedle500.getUTF16Name("English") + beedle500EnglishPlurality + u"a mere " + TEXT_COLOR_RED + u"500 Rupees" + TEXT_COLOR_DEFAULT, 43) + u"\nBuy it! Buy it! Buy buy buy!\n" + TWO_CHOICES + u"I'll buy it\nNo thanks" + TEXT_END},
      {"Spanish", word_wrap_string(CAPITAL + beedle500SpanishConjugation + beedle500.getUTF16Name("Spanish") + TEXT_COLOR_DEFAULT + u" - Solo " + TEXT_COLOR_RED + u"500 Rupias" + TEXT_COLOR_DEFAULT, 43) + u"\n¡Cómprala sin pensarlo dos veces!\n" + TWO_CHOICES + u"Lo quiero\nNo, gracias" + TEXT_END},
      {"French",  word_wrap_string(beedle500.getUTF16Name("French") + u" seulement " + TEXT_COLOR_RED + u"500 Rubis" + TEXT_COLOR_DEFAULT, 43) + u".\nAcheter!\n" + TWO_CHOICES + u"Oui\nSans façon" + TEXT_END}}},

     // Rock Spire Shop 950 Rupee Item
     {"12107",
     {{"English", word_wrap_string(TEXT_COLOR_RED + beedle950.getUTF16Name("English") + u"  950 Rupees" + TEXT_COLOR_DEFAULT, 43) + u"\nThis is my last one of these, too." + TEXT_END},
      {"Spanish", word_wrap_string(TEXT_COLOR_RED + beedle950.getUTF16Name("Spanish") + u"  950 Rupias" + TEXT_COLOR_DEFAULT, 43) + u"\nTambién es el único que me queda." + TEXT_END},
      {"French",  word_wrap_string(TEXT_COLOR_RED + beedle950.getUTF16Name("French") + u".  950 Rubis." + TEXT_COLOR_DEFAULT, 43) + u"\nLà encore, il ne m'en reste juste un." + TEXT_END}}},
     {"12110",
     {{"English", word_wrap_string(beedle950EnglishPronoun + beedle950.getUTF16Name("English") + beedle950EnglishPlurality + u"only " + TEXT_COLOR_RED + u"950 Rupees" + TEXT_COLOR_DEFAULT, 43) + u"\nBuy it! Buy it! Buy buy buy!\n" + TWO_CHOICES + u"I'll buy it\nNo thanks" + TEXT_END},
      {"Spanish", word_wrap_string(CAPITAL + beedle950SpanishConjugation + TEXT_COLOR_RED + beedle950.getUTF16Name("Spanish") + TEXT_COLOR_DEFAULT + beedle950SpanishPlurality + TEXT_COLOR_RED + u"950 Rupias" + TEXT_COLOR_DEFAULT, 43) + u"\nCompra... ¡Compra!\n" + TWO_CHOICES + u"Lo quiero\nNo, gracias" + TEXT_END},
      {"French",  word_wrap_string(beedle950.getUTF16Name("French") + u" seulement " + TEXT_COLOR_RED + u"950 Rubis" + TEXT_COLOR_DEFAULT, 43) + u".\nAcheter!\n" + TWO_CHOICES + u"Oui\nSans façon" + TEXT_END}}},

     // Rock Spire Shop 900 Rupee Item
     {"12108",
     {{"English", word_wrap_string(TEXT_COLOR_RED + beedle900.getUTF16Name("English") + u"  900 Rupees" + TEXT_COLOR_DEFAULT, 43) + u"\nThe price may be high, but it'll pay\noff handsomely in the end!" + TEXT_END},
      {"Spanish", word_wrap_string(TEXT_COLOR_RED + beedle900.getUTF16Name("Spanish") + u"  900 Rupias" + TEXT_COLOR_DEFAULT, 43) + u"\nParece muy caro, pero ¡créeme que no te arrepentirás!" + TEXT_END},
      {"French",  word_wrap_string(TEXT_COLOR_RED + beedle900.getUTF16Name("French")  + u".  900 Rubis." + TEXT_COLOR_DEFAULT, 43) + u"\nC'est " + beedle900.getUTF16Name("French") + u", n'est-ce pas...\nDonc quel que soit son prix,\nvous parviendrez à l'amortir..." + TEXT_END}}},
     {"12111",
     {{"English", word_wrap_string(beedle900EnglishPronoun + beedle900.getUTF16Name("English") + beedle900EnglishPlurality + u"just " + TEXT_COLOR_RED + u"900 Rupees" + TEXT_COLOR_DEFAULT, 43) + u"\nBuy it! Buy it! Buy buy buy!\n" + TWO_CHOICES + u"I'll buy it\nNo thanks" + TEXT_END},
      {"Spanish", word_wrap_string(CAPITAL + beedle900SpanishConjugation + beedle900.getUTF16Name("Spanish") + beedle900SpanishPlurality + TEXT_COLOR_RED + u"900 Rupias" + TEXT_COLOR_DEFAULT, 43) + u"\nCompra... ¡Compra!\n" + TWO_CHOICES + u"Lo quiero\nNo, gracias" + TEXT_END},
      {"French",  word_wrap_string(beedle900.getUTF16Name("French") + u" seulement " + TEXT_COLOR_RED + u"900 Rubis" + TEXT_COLOR_DEFAULT, 43) + u".\nAcheter!\n" + TWO_CHOICES + u"Oui\nSans façon" + TEXT_END}}},
 };
}
