#include "seed.hpp"

#include <cctype>
#include <vector>

#include <seedgen/random.hpp>
#include <seedgen/config.hpp>
#include <seedgen/permalink.hpp>
#include <utility/file.hpp>
#include <libs/zlib-ng/zlib-ng.h>

static std::vector<std::string> adjectives = {
    "able",
    "accommodating",
    "accomplished",
    "accurate",
    "actual",
    "adept",
    "admirable",
    "adorable",
    "adult",
    "afraid",
    "aggressive",
    "agile",
    "agreeable",
    "airborne",
    "alert",
    "alike",
    "alive",
    "all",
    "alone",
    "ancient",
    "angry",
    "angular",
    "annual",
    "another",
    "antiquated",
    "any",
    "apparent",
    "appropriate",
    "arid",
    "artistic",
    "ashamed",
    "asleep",
    "assuming",
    "atrocious",
    "available",
    "aware",
    "aweigh",
    "awesome",
    "awful",
    "bad",
    "barren",
    "bashful",
    "bats",
    "battered",
    "big",
    "black",
    "bleak",
    "blessed",
    "blind",
    "blinking",
    "blue",
    "bold",
    "born",
    "bossy",
    "both",
    "bottomless",
    "brave",
    "breathtaking",
    "brief",
    "bright",
    "brilliant",
    "burly",
    "burning",
    "bushed",
    "busy",
    "calm",
    "careful",
    "catching",
    "central",
    "certain",
    "charming",
    "cheap",
    "cheery",
    "childish",
    "chubby",
    "clean",
    "clear",
    "clever",
    "close",
    "cold",
    "colored",
    "comfortable",
    "coming",
    "common",
    "compatible",
    "complete",
    "complimentary",
    "concerned",
    "confident",
    "confirmed",
    "constant",
    "convenient",
    "cool",
    "correct",
    "countless",
    "courageous",
    "cowardly",
    "cozy",
    "crazy",
    "creative",
    "credible",
    "critical",
    "cruel",
    "crying",
    "cunning",
    "curious",
    "current",
    "cursed",
    "customary",
    "cute",
    "daily",
    "damp",
    "dangerous",
    "dark",
    "dashing",
    "dauntless",
    "dead",
    "deadly",
    "dear",
    "decent",
    "decided",
    "decorative",
    "decrepit",
    "deep",
    "defensive",
    "delicate",
    "delicious",
    "departed",
    "desperate",
    "destructive",
    "determined",
    "devoted",
    "different",
    "difficult",
    "dignified",
    "dingy",
    "direct",
    "dirty",
    "disagreeable",
    "dishonorable",
    "distant",
    "distinct",
    "distinguished",
    "divine",
    "done",
    "double",
    "downright",
    "dramatic",
    "drawn",
    "dreadful",
    "dreamy",
    "dry",
    "due",
    "dull",
    "dumb",
    "dutiful",
    "each",
    "eager",
    "early",
    "earnest",
    "easterly",
    "eastern",
    "easy",
    "eerie",
    "effective",
    "eight",
    "either",
    "elder",
    "elderly",
    "elegant",
    "elusive",
    "empty",
    "endless",
    "energetic",
    "english",
    "enough",
    "entire",
    "extra",
    "extravagant",
    "eternal",
    "even",
    "every",
    "evil",
    "excellent",
    "exceptional",
    "exciting",
    "exotic",
    "expensive",
    "experienced",
    "expert",
    "explosive",
    "fabled",
    "fabulous",
    "failed",
    "fair",
    "famed",
    "familiar",
    "famous",
    "fantastic",
    "fast",
    "fatal",
    "favorable",
    "favorite",
    "fearsome",
    "feeble",
    "ferocious",
    "few",
    "fickle",
    "fiddling",
    "fierce",
    "fiery",
    "fifteen",
    "fifty",
    "filthy",
    "final",
    "financial",
    "fine",
    "firm",
    "first",
    "fishy",
    "fit",
    "fitted",
    "five",
    "flaming",
    "flashy",
    "floating",
    "flying",
    "fond",
    "foolhardy",
    "foolish",
    "foreign",
    "foremost",
    "former",
    "formidable",
    "forty",
    "forward",
    "foul",
    "four",
    "fourth",
    "fragrant",
    "free",
    "french",
    "frequent",
    "frigid",
    "frivolous",
    "full",
    "functional",
    "funky",
    "funny",
    "furtive",
    "future",
    "gangly",
    "gallant",
    "gelatinous",
    "general",
    "generous",
    "gentle",
    "gifted",
    "gigantic",
    "glad",
    "glorious",
    "gnarled",
    "golden",
    "gone",
    "good",
    "gooey",
    "gorgeous",
    "gracious",
    "grand",
    "grateful",
    "great",
    "greedy",
    "green",
    "gross",
    "grubby",
    "guarded",
    "guilty",
    "gummy",
    "hale",
    "handsome",
    "handy",
    "happy",
    "hard",
    "harsh",
    "hasty",
    "healthy",
    "heartfelt",
    "hearty",
    "heavy",
    "hefty",
    "helpful",
    "helpless",
    "heroic",
    "high",
    "homesick",
    "homing",
    "honest",
    "honorable",
    "honorary",
    "hooded",
    "horizontal",
    "horrible",
    "horrific",
    "hot",
    "huge",
    "human",
    "humble",
    "hundred",
    "hungry",
    "hyper",
    "icy",
    "idle",
    "ill",
    "impatient",
    "impenetrable",
    "important",
    "impossible",
    "impressive",
    "impulsive",
    "incarnate",
    "incredible",
    "incredulous",
    "indebted",
    "indigenous",
    "individual",
    "inexpensive",
    "ingenious",
    "injured",
    "inner",
    "innocent",
    "inquisitive",
    "insane",
    "insistent",
    "instant",
    "intact",
    "intelligent",
    "interesting",
    "interior",
    "intricate",
    "invincible",
    "junior",
    "just",
    "keen",
    "knowing",
    "knowledgeable",
    "lacking",
    "lame",
    "lamentable",
    "landed",
    "large",
    "last",
    "late",
    "laudable",
    "leading",
    "learned",
    "least",
    "left",
    "legendary",
    "legitimate",
    "less",
    "lifelong",
    "like",
    "likely",
    "limited",
    "little",
    "lively",
    "local",
    "lonely",
    "long",
    "loose",
    "loud",
    "lovable",
    "lovely",
    "loving",
    "low",
    "lower",
    "lowly",
    "lucky",
    "lush",
    "luxurious",
    "lyric",
    "mad",
    "magical",
    "main",
    "major",
    "manned",
    "mannered",
    "manual",
    "many",
    "marked",
    "married",
    "marvelous",
    "massive",
    "mature",
    "measly",
    "medicinal",
    "mental",
    "mere",
    "merry",
    "messy",
    "met",
    "middle",
    "mighty",
    "mild",
    "minded",
    "mischievous",
    "miserable",
    "missing",
    "mistaken",
    "mixed",
    "modern",
    "molten",
    "monstrous",
    "more",
    "most",
    "moving",
    "much",
    "multiple",
    "mushy",
    "musical",
    "musty",
    "mute",
    "mysterious",
    "mystical",
    "narrow",
    "nasty",
    "natural",
    "naughty",
    "nearby",
    "neat",
    "necessary",
    "neither",
    "nervous",
    "new",
    "next",
    "nice",
    "nifty",
    "nightly",
    "nine",
    "noble",
    "normal",
    "northerly",
    "northern",
    "nosy",
    "numb",
    "nuts",
    "objective",
    "oblivious",
    "occasional",
    "odd",
    "official",
    "old",
    "olden",
    "one",
    "open",
    "opposite",
    "opulent",
    "orderly",
    "ordinary",
    "original",
    "other",
    "outdoor",
    "outgoing",
    "outrageous",
    "own",
    "painful",
    "pale",
    "particular",
    "passionate",
    "past",
    "pathetic",
    "patient",
    "peaceful",
    "peculiar",
    "penniless",
    "perfect",
    "persuasive",
    "pesky",
    "physical",
    "picky",
    "piggish",
    "pitiable",
    "plain",
    "pleasant",
    "plentiful",
    "pointless",
    "poor",
    "popular",
    "positive",
    "possible",
    "postal",
    "potent",
    "powerful",
    "precious",
    "present",
    "pressing",
    "pretty",
    "priceless",
    "pristine",
    "private",
    "professional",
    "prone",
    "proper",
    "prosperous",
    "protective",
    "proud",
    "pure",
    "pushing",
    "quick",
    "quiet",
    "rare",
    "raw",
    "readable",
    "ready",
    "real",
    "reasonable",
    "reckless",
    "red",
    "regal",
    "regardless",
    "regular",
    "related",
    "reliable",
    "reminiscent",
    "responsible",
    "restorative",
    "retiring",
    "rewarding",
    "rich",
    "ridiculous",
    "right",
    "righteous",
    "rising",
    "robust",
    "romantic",
    "rough",
    "round",
    "royal",
    "rude",
    "sacred",
    "sad",
    "safe",
    "salty",
    "same",
    "sandy",
    "satisfactory",
    "saucy",
    "savage",
    "saving",
    "seagoing",
    "seasick",
    "second",
    "secret",
    "secure",
    "seedy",
    "seeming",
    "senseless",
    "sensitive",
    "sentimental",
    "separate",
    "serious",
    "seven",
    "seventeen",
    "several",
    "shabby",
    "shady",
    "sharp",
    "shelled",
    "shiny",
    "shocking",
    "short",
    "showy",
    "shy",
    "sick",
    "sided",
    "sighted",
    "silent",
    "silly",
    "simple",
    "sincere",
    "single",
    "six",
    "skilled",
    "skillful",
    "skinny",
    "sleepy",
    "slick",
    "slight",
    "slimy",
    "slow",
    "small",
    "smart",
    "smooth",
    "snappy",
    "soft",
    "solid",
    "some",
    "sorry",
    "sour",
    "southern",
    "spacious",
    "spare",
    "special",
    "specific",
    "spectacular",
    "speechless",
    "splendid",
    "steady",
    "stellar",
    "stern",
    "sticky",
    "stiff",
    "still",
    "straight",
    "strange",
    "strapping",
    "strict",
    "striking",
    "strong",
    "stubborn",
    "stunning",
    "stupendous",
    "stupid",
    "sturdy",
    "stylish",
    "sublime",
    "substandard",
    "successful",
    "such",
    "sudden",
    "suitable",
    "sunken",
    "super",
    "superb",
    "superior",
    "supple",
    "supreme",
    "sure",
    "surly",
    "susceptible",
    "suspicious",
    "swarthy",
    "sweet",
    "taciturn",
    "tacky",
    "taking",
    "talkative",
    "talking",
    "tall",
    "tardy",
    "tarry",
    "teensy",
    "teeny",
    "telling",
    "ten",
    "tender",
    "terrible",
    "terrific",
    "thankful",
    "thankless",
    "the",
    "thick",
    "thinking",
    "third",
    "thirty",
    "thoughtful",
    "thoughtless",
    "three",
    "tied",
    "tight",
    "timeless",
    "timid",
    "tiny",
    "tired",
    "tireless",
    "total",
    "tough",
    "traditional",
    "tragic",
    "traveled",
    "treacherous",
    "tremendous",
    "trifling",
    "tropical",
    "troublesome",
    "true",
    "trusting",
    "trying",
    "twenty",
    "two",
    "unable",
    "unarmed",
    "unassuming",
    "unbelievable",
    "unclean",
    "unconscious",
    "undeniable",
    "underwater",
    "undeterred",
    "unexpected",
    "unexplored",
    "unfamiliar",
    "unflagging",
    "unfortunate",
    "unharmed",
    "unhealthy",
    "unheard",
    "uniform",
    "unique",
    "unknown",
    "unlike",
    "unlikely",
    "unmatched",
    "unnatural",
    "unpredictable",
    "unreasonable",
    "unrequited",
    "unruly",
    "unsatisfactory",
    "unsatisfied",
    "unseemly",
    "unsettled",
    "unsolicited",
    "unspoken",
    "unstable",
    "unstoppable",
    "unsurpassed",
    "unsuspecting",
    "unusual",
    "unwanted",
    "unwashed",
    "unworthy",
    "upper",
    "upright",
    "urgent",
    "useful",
    "useless",
    "usual",
    "utmost",
    "utter",
    "vain",
    "valid",
    "valuable",
    "varied",
    "vast",
    "vertical",
    "vile",
    "violent",
    "visible",
    "vital",
    "volcanic",
    "vulnerable",
    "wanting",
    "warm",
    "wary",
    "weak",
    "weary",
    "wee",
    "weepy",
    "weird",
    "westerly",
    "western",
    "wet",
    "whatever",
    "whatsoever",
    "whimsical",
    "white",
    "whole",
    "whopping",
    "wide",
    "wild",
    "willing",
    "winning",
    "wintry",
    "wise",
    "wonderful",
    "wondrous",
    "wooden",
    "woolly",
    "working",
    "worrisome",
    "worse",
    "worst",
    "worth",
    "worthless",
    "worthy",
    "wretched",
    "wrong",
    "wry",
    "yellow",
    "young",
    "undead",
    "ghostly",
    "forbidden",
    "ganon's"
};


static std::vector<std::string> short_names = {
    "link",
    "zelda",
    "medli",
    "makar",
    "korok",
    "hollo",
    "kreeb",
    "kamo",
    "obli",
    "willi",
    "zora",
    "gonzo",
    "'hoy",
    "aryll",
    "mila",
    "ganon",
    "rito",
    "sam",
    "vera",
    "missy",
    "dampa",
    "kane",
    "tetra",
    "valoo",
    "gohma",
    "poe",
    "gyorg",
    "niko",
    "woods",
    "morth",
    "elma",
    "oakin",
    "drona",
    "irch",
    "aldo",
    "rown",
    "keese",
    "basht",
    "bisht",
    "ilari",
    "ivan",
    "jan",
    "jin",
    "mesa",
    "joel",
    "abe",
    "zill",
    "zuko",
    "ankle",
    "lenzo",
    "anton",
    "linda",
    "fado",
    "baito",
    "din",
    "nayru",
    "jabun"
};

static std::vector<std::string> names = {
    "link",
    "zelda",
    "medli",
    "makar",
    "korok",
    "hollo",
    "olivio",
    "chuchu",
    "kreeb",
    "kamo",
    "gillian",
    "potova",
    "joanna",
    "hoskit",
    "obli",
    "willi",
    "bokoblin",
    "miniblin",
    "bombchu",
    "zora",
    "laruto",
    "ganondorf",
    "gonzo",
    "hyrulian",
    "daphnes",
    "nohansen",
    "hyrule",
    "triforce",
    "komali",
    "maggie",
    "beedle",
    "cyclos",
    "zephos",
    "'hoy",
    "aryll",
    "mila",
    "ganon",
    "rito",
    "swabbie",
    "shrimplet",
    "sam",
    "gossack",
    "garrickson",
    "vera",
    "pompie",
    "missy",
    "minenco",
    "dampa",
    "kane",
    "tetra",
    "valoo",
    "gohma",
    "wizzrobe",
    "stalfos",
    "kalleDemos",
    "gohdan",
    "kargaroc",
    "helmarocKing",
    "jalhalla",
    "molgera",
    "redead",
    "octorok",
    "peahat",
    "seahat",
    "poe",
    "gyorg",
    "darknut",
    "mothula",
    "niko",
    "greatfish",
    "woods",
    "moblin",
    "morth",
    "elma",
    "oakin",
    "drona",
    "irch",
    "aldo",
    "rown",
    "keese",
    "magtail",
    "koboli",
    "basht",
    "bisht",
    "pashli",
    "namali",
    "ilari",
    "kogoli",
    "ivan",
    "jan",
    "jin",
    "jun-Roberto",
    "salvatore",
    "docBandam",
    "mesa",
    "joel",
    "abe",
    "zill",
    "sue-Belle",
    "zuko",
    "tingle",
    "ankle",
    "knuckle",
    "davidJr",
    "lenzo",
    "anton",
    "linda",
    "fado",
    "kokiri",
    "hylian",
    "baito",
    "din",
    "farore",
    "nayru",
    "jabun"
};

static std::vector<std::string> nouns = {
    "ability",
    "abode",
    "access",
    "accessory",
    "act",
    "acting",
    "action",
    "activity",
    "acupuncture",
    "adage",
    "addition",
    "addressee",
    "admiral",
    "admiration",
    "advantage",
    "adventure",
    "advertisement",
    "advice",
    "adviser",
    "aerial",
    "affection",
    "age",
    "air",
    "alarm",
    "allowance",
    "ally",
    "amount",
    "amusement",
    "ancestor",
    "anchor",
    "angel",
    "anger",
    "angle",
    "animal",
    "anniversary",
    "answer",
    "antique",
    "anybody",
    "apology",
    "appearance",
    "apple",
    "appreciation",
    "apprentice",
    "arbiter",
    "archer",
    "archipelago",
    "are",
    "area",
    "aria",
    "arm",
    "armor",
    "aroma",
    "arrangement",
    "arrival",
    "arrow",
    "art",
    "artist",
    "assistant",
    "association",
    "assortment",
    "atoll",
    "attendance",
    "attendant",
    "attention",
    "auction",
    "auto",
    "autopilot",
    "average",
    "baby",
    "bachelor",
    "back",
    "backbone",
    "backside",
    "bag",
    "bait",
    "balance",
    "ball",
    "ballad",
    "ballast",
    "bane",
    "bank",
    "banner",
    "bar",
    "bargain",
    "baritone",
    "barnacle",
    "barrier",
    "base",
    "basement",
    "basic",
    "batch",
    "bath",
    "bathroom",
    "baton",
    "battery",
    "battle",
    "beverage",
    "bidding",
    "bird",
    "birth",
    "birthday",
    "birthplace",
    "bit",
    "blade",
    "blast",
    "blemish",
    "bliss",
    "blizzard",
    "block",
    "bloke",
    "blood",
    "blossom",
    "board",
    "boat",
    "boating",
    "bod",
    "body",
    "bomb",
    "bone",
    "bonus",
    "book",
    "boomerang",
    "boots",
    "border",
    "bosom",
    "bottle",
    "bottom",
    "boulder",
    "bounty",
    "bow",
    "box",
    "boy",
    "boyfriend",
    "brain",
    "branch",
    "brand",
    "brat",
    "breath",
    "breeze",
    "brick",
    "bridge",
    "brother",
    "bubble",
    "bud",
    "buddy",
    "bug",
    "building",
    "bulk",
    "bunch",
    "business",
    "businessman",
    "bust",
    "butler",
    "button",
    "buzz",
    "bye",
    "cabin",
    "cable",
    "cake",
    "calamity",
    "calling",
    "candy",
    "cannon",
    "cape",
    "captive",
    "card",
    "care",
    "carpenter",
    "carver",
    "carving",
    "case",
    "cash",
    "casting",
    "castle",
    "cause",
    "caution",
    "cave",
    "cavern",
    "ceiling",
    "cell",
    "cello",
    "center",
    "ceremony",
    "chain",
    "challenge",
    "chamber",
    "champion",
    "chance",
    "change",
    "chap",
    "charity",
    "charm",
    "chart",
    "cheapskate",
    "cheer",
    "chest",
    "chieftain",
    "childhood",
    "chin",
    "china",
    "chock",
    "choice",
    "chore",
    "chortle",
    "chum",
    "chump",
    "chunk",
    "circumstance",
    "class",
    "claw",
    "clearance",
    "click",
    "cliff",
    "clip",
    "clock",
    "clothes",
    "clothing",
    "club",
    "clue",
    "coat",
    "coating",
    "coffee",
    "collapse",
    "collection",
    "color",
    "column",
    "companion",
    "company",
    "compass",
    "compensation",
    "competition",
    "competitor",
    "complexion",
    "composure",
    "compulsion",
    "con",
    "concept",
    "conclusion",
    "conduct",
    "confidence",
    "connection",
    "connoisseur",
    "constellation",
    "container",
    "contest",
    "contract",
    "control",
    "controller",
    "conversation",
    "cookie",
    "cooler",
    "coping",
    "copy",
    "core",
    "corner",
    "cornucopia",
    "counsel",
    "counter",
    "country",
    "couple",
    "coupon",
    "courage",
    "course",
    "crab",
    "craft",
    "crawfish",
    "creature",
    "crescent",
    "crest",
    "crew",
    "cross",
    "crowd",
    "crown",
    "crud",
    "crystal",
    "cup",
    "cupid",
    "curio",
    "curiosity",
    "curse",
    "cursor",
    "curtain",
    "customer",
    "cutlery",
    "cutting",
    "dad",
    "damage",
    "daring",
    "darling",
    "data",
    "date",
    "daughter",
    "dawn",
    "day",
    "daylight",
    "dealings",
    "dearie",
    "death",
    "debt",
    "debutante",
    "decision",
    "deck",
    "decoration",
    "dedication",
    "deed",
    "defense",
    "deity",
    "delinquent",
    "delivery",
    "demand",
    "demise",
    "depression",
    "description",
    "design",
    "desire",
    "despair",
    "destination",
    "destiny",
    "destruction",
    "determination",
    "development",
    "device",
    "diamond",
    "difference",
    "difficulty",
    "digit",
    "dignity",
    "dilemma",
    "dinner",
    "dipper",
    "direction",
    "dirt",
    "disaster",
    "discovery",
    "disposal",
    "disposition",
    "distance",
    "diversion",
    "dog",
    "domain",
    "doom",
    "door",
    "doorway",
    "dose",
    "doubt",
    "dough",
    "downpour",
    "dragon",
    "drama",
    "draught",
    "drawing",
    "dream",
    "dressing",
    "drift",
    "driftwood",
    "drill",
    "drop",
    "dummy",
    "dump",
    "dungeon",
    "dusk",
    "dust",
    "duty",
    "earnings",
    "earth",
    "earthquake",
    "ease",
    "east",
    "echo",
    "economics",
    "edge",
    "effort",
    "element",
    "elixir",
    "emotion",
    "emperor",
    "encyclopedia",
    "end",
    "ending",
    "enemy",
    "energy",
    "entry",
    "envelope",
    "envy",
    "epitome",
    "enthusiasm",
    "enthusiast",
    "entrance",
    "errand",
    "error",
    "extent",
    "extravaganza",
    "eye",
    "eyesight",
    "essence",
    "estimate",
    "evening",
    "event",
    "evidence",
    "example",
    "excess",
    "exchange",
    "excitement",
    "exercise",
    "exhaustion",
    "existence",
    "exit",
    "experience",
    "explanation",
    "expression",
    "fabric",
    "face",
    "facing",
    "fact",
    "failure",
    "fairy",
    "faith",
    "fame",
    "family",
    "fan",
    "fancier",
    "fancy",
    "fashion",
    "fat",
    "fate",
    "father",
    "fathom",
    "fault",
    "favor",
    "fear",
    "feat",
    "feather",
    "feature",
    "fee",
    "feeling",
    "fellow",
    "felt",
    "fence",
    "fencing",
    "festival",
    "field",
    "fiend",
    "figure",
    "figurine",
    "file",
    "filing",
    "filling",
    "film",
    "finance",
    "finding",
    "finger",
    "fire",
    "firefly",
    "fish",
    "fisherman",
    "fishing",
    "flab",
    "flag",
    "flair",
    "flame",
    "flashing",
    "flattery",
    "fledgling",
    "fleet",
    "flight",
    "flock",
    "floor",
    "flower",
    "focus",
    "folk",
    "food",
    "fool",
    "force",
    "foreboding",
    "forest",
    "fork",
    "form",
    "format",
    "fortress",
    "fortune",
    "foundation",
    "fountain",
    "fox",
    "fragment",
    "frame",
    "friend",
    "friendship",
    "front",
    "fruit",
    "fruition",
    "fun",
    "funeral",
    "gal",
    "gale",
    "gallery",
    "game",
    "gander",
    "gang",
    "garb",
    "garden",
    "gate",
    "gathering",
    "gear",
    "gem",
    "generation",
    "genius",
    "gentleman",
    "ghost",
    "giant",
    "gift",
    "girl",
    "girlfriend",
    "girth",
    "glass",
    "glob",
    "gloom",
    "glory",
    "gloss",
    "goal",
    "god",
    "goddess",
    "going",
    "gold",
    "gondola",
    "goo",
    "goodness",
    "goody",
    "goose",
    "gossip",
    "gourmet",
    "grade",
    "graffiti",
    "granddaughter",
    "grandfather",
    "grandma",
    "grandmother",
    "grandpa",
    "grass",
    "gratitude",
    "grave",
    "grit",
    "grotto",
    "ground",
    "group",
    "grove",
    "grub",
    "guarantee",
    "guardian",
    "guidance",
    "guide",
    "gull",
    "gunpowder",
    "guru",
    "gust",
    "gutter",
    "guy",
    "habit",
    "habitat",
    "hag",
    "hail",
    "hair",
    "hairstyle",
    "half",
    "hall",
    "halt",
    "hammer",
    "hand",
    "handful",
    "handle",
    "handwriting",
    "hanging",
    "happening",
    "hardship",
    "harm",
    "harmony",
    "harness",
    "harp",
    "haste",
    "hat",
    "haven",
    "havoc",
    "hearing",
    "heart",
    "heat",
    "heir",
    "heirloom",
    "helping",
    "hem",
    "herald",
    "herd",
    "hero",
    "hick",
    "hiding",
    "hill",
    "hint",
    "hip",
    "history",
    "head",
    "headache",
    "heading",
    "headstone",
    "health",
    "hobby",
    "holding",
    "hole",
    "home",
    "homeland",
    "homework",
    "honor",
    "hood",
    "hoof",
    "hook",
    "hooligan",
    "hope",
    "horizon",
    "horror",
    "horseshoe",
    "host",
    "hour",
    "house",
    "hue",
    "hull",
    "hunter",
    "hurricane",
    "hurry",
    "husband",
    "hutch",
    "ice",
    "icon",
    "idea",
    "identity",
    "idiot",
    "idol",
    "ignorance",
    "image",
    "imp",
    "impression",
    "impulse",
    "incident",
    "incursion",
    "index",
    "induction",
    "industry",
    "inflation",
    "info",
    "information",
    "ingredient",
    "injury",
    "insect",
    "inside",
    "inspiration",
    "instance",
    "instruction",
    "instrument",
    "intellect",
    "intensity",
    "interest",
    "interview",
    "introduction",
    "intuition",
    "invention",
    "invitation",
    "iron",
    "island",
    "isle",
    "islet",
    "issue",
    "item",
    "jamboree",
    "jar",
    "jelly",
    "jerk",
    "jewel",
    "job",
    "joke",
    "journey",
    "joy",
    "judge",
    "judgement",
    "juice",
    "junk",
    "justice",
    "kaleidoscope",
    "keeping",
    "keepsake",
    "key",
    "kid",
    "kiddie",
    "kiddo",
    "killer",
    "killing",
    "kin",
    "kind",
    "kindness",
    "king",
    "kingdom",
    "knapsack",
    "knife",
    "knight",
    "knowledge",
    "labyrinth",
    "lack",
    "lad",
    "ladder",
    "lady",
    "lair",
    "lamb",
    "land",
    "landing",
    "landlubber",
    "language",
    "lass",
    "latch",
    "laughing",
    "lava",
    "law",
    "leader",
    "leaf",
    "league",
    "learning",
    "ledge",
    "leftover",
    "leg",
    "legend",
    "length",
    "lent",
    "lesson",
    "letter",
    "level",
    "liar",
    "life",
    "lifestyle",
    "lifetime",
    "light",
    "lighthouse",
    "lights",
    "likeness",
    "limit",
    "line",
    "link",
    "lion",
    "living",
    "load",
    "loan",
    "location",
    "lock",
    "log",
    "longing",
    "loo",
    "lookout",
    "loot",
    "lord",
    "loss",
    "lot",
    "lout",
    "love",
    "lover",
    "luck",
    "lust",
    "ma'am",
    "machine",
    "maestro",
    "magic",
    "magician",
    "magma",
    "mail",
    "maker",
    "making",
    "man",
    "management",
    "manager",
    "manner",
    "mansion",
    "map",
    "marionette",
    "mark",
    "marriage",
    "mask",
    "mass",
    "master",
    "mastery",
    "match",
    "material",
    "math",
    "matter",
    "maximum",
    "may",
    "meaning",
    "means",
    "measure",
    "medicine",
    "medium",
    "meeting",
    "melody",
    "member",
    "membership",
    "memento",
    "memory",
    "merchant",
    "mercy",
    "mess",
    "message",
    "metal",
    "meter",
    "metronome",
    "might",
    "mile",
    "milk",
    "mind",
    "minimum",
    "minute",
    "mirror",
    "mischief",
    "misfortune",
    "mist",
    "mistake",
    "mister",
    "mode",
    "mold",
    "mom",
    "moment",
    "money",
    "monochrome",
    "monopoly",
    "monster",
    "monstrosity",
    "monument",
    "mood",
    "moon",
    "morning",
    "moth",
    "mother",
    "motif",
    "motion",
    "mountain",
    "mouth",
    "mr",
    "mrs",
    "mum",
    "muscle",
    "music",
    "mystery",
    "mystic",
    "nail",
    "name",
    "native",
    "nature",
    "navigation",
    "neck",
    "necklace",
    "nectar",
    "needle",
    "nerve",
    "nest",
    "news",
    "nickname",
    "night",
    "nightfall",
    "nightmare",
    "nitwit",
    "nobility",
    "noise",
    "nonsense",
    "nook",
    "noon",
    "north",
    "nose",
    "notch",
    "note",
    "nothing",
    "notice",
    "number",
    "nurse",
    "oasis",
    "oath",
    "object",
    "obsession",
    "occupation",
    "ocean",
    "offering",
    "onrush",
    "onslaught",
    "opener",
    "opening",
    "opinion",
    "opportunity",
    "orange",
    "orca",
    "ordeal",
    "order",
    "outfit",
    "outset",
    "outside",
    "outsider",
    "owner",
    "pace",
    "pack",
    "pad",
    "paddock",
    "page",
    "pain",
    "painter",
    "painting",
    "pair",
    "pal",
    "palace",
    "palette",
    "panel",
    "panic",
    "pants",
    "paper",
    "paradise",
    "parasite",
    "parcel",
    "parchment",
    "pardon",
    "part",
    "partner",
    "party",
    "passage",
    "passion",
    "password",
    "patch",
    "path",
    "patience",
    "patronage",
    "payment",
    "peace",
    "peak",
    "pear",
    "pearl",
    "pedestal",
    "peg",
    "pen",
    "pendant",
    "people",
    "pepper",
    "performance",
    "perimeter",
    "permission",
    "person",
    "personality",
    "personnel",
    "perspective",
    "pet",
    "phantom",
    "photo",
    "phrase",
    "physics",
    "pickle",
    "pictograph",
    "picture",
    "piece",
    "pig",
    "piggy",
    "piglet",
    "pinnacle",
    "piping",
    "pirate",
    "place",
    "plague",
    "plan",
    "plank",
    "plankton",
    "plant",
    "plateau",
    "platform",
    "player",
    "pleasure",
    "plenty",
    "plight",
    "plot",
    "pocket",
    "point",
    "police",
    "policy",
    "pomp",
    "pond",
    "poof",
    "pool",
    "pop",
    "population",
    "portal",
    "portion",
    "position",
    "possession",
    "possibility",
    "post",
    "postage",
    "postbox",
    "postman",
    "potion",
    "pouch",
    "pound",
    "poverty",
    "power",
    "powerhouse",
    "practice",
    "prank",
    "prayer",
    "predecessor",
    "premium",
    "presence",
    "prey",
    "price",
    "pride",
    "prince",
    "princess",
    "priority",
    "prison",
    "privacy",
    "prize",
    "problem",
    "process",
    "product",
    "program",
    "progress",
    "project",
    "projectile",
    "promise",
    "proof",
    "propeller",
    "property",
    "proposal",
    "protector",
    "proverb",
    "pulp",
    "punk",
    "pupil",
    "puppet",
    "purist",
    "purple",
    "purpose",
    "purse",
    "pursuit",
    "puzzle",
    "quality",
    "quandary",
    "quantum",
    "queen",
    "query",
    "quest",
    "question",
    "quill",
    "race",
    "racehorse",
    "racket",
    "radar",
    "raft",
    "ragamuffin",
    "rage",
    "rain",
    "rainbow",
    "rainstorm",
    "range",
    "rascal",
    "rat",
    "rate",
    "ray",
    "reading",
    "realm",
    "rear",
    "reason",
    "reception",
    "record",
    "reef",
    "reflection",
    "refreshment",
    "refuge",
    "regimen",
    "region",
    "register",
    "relaxation",
    "relic",
    "remains",
    "rent",
    "request",
    "requiem",
    "research",
    "resident",
    "resort",
    "resource",
    "respect",
    "response",
    "responsibility",
    "result",
    "reunion",
    "reward",
    "rhythm",
    "riches",
    "riddle",
    "ridicule",
    "ring",
    "road",
    "roar",
    "rock",
    "rocket",
    "role",
    "romance",
    "room",
    "roost",
    "rope",
    "rose",
    "royalty",
    "rubbish",
    "ruckus",
    "ruin",
    "rule",
    "rumor",
    "rung",
    "running",
    "rupee",
    "sack",
    "safety",
    "sage",
    "sail",
    "sailor",
    "sake",
    "salary",
    "sale",
    "salesman",
    "salute",
    "salvage",
    "sand",
    "sap",
    "sapling",
    "satchel",
    "savior",
    "saw",
    "saying",
    "scale",
    "scamp",
    "scene",
    "scent",
    "schedule",
    "scholar",
    "school",
    "schoolteacher",
    "schooner",
    "science",
    "score",
    "scoundrel",
    "screen",
    "scroll",
    "sculptor",
    "sculpture",
    "scurvy",
    "sea",
    "seagull",
    "seal",
    "searchlight",
    "sector",
    "seed",
    "seer",
    "selection",
    "self",
    "semi",
    "sense",
    "service",
    "serving",
    "setting",
    "sewing",
    "shadow",
    "shaft",
    "shame",
    "shape",
    "shard",
    "share",
    "shark",
    "sheep",
    "sheet",
    "shelf",
    "shield",
    "ship",
    "shipment",
    "shock",
    "shop",
    "shopping",
    "shore",
    "shot",
    "showing",
    "shrimp",
    "shrine",
    "sibling",
    "sickle",
    "side",
    "sight",
    "sign",
    "signal",
    "silence",
    "silhouette",
    "silver",
    "sir",
    "sister",
    "site",
    "sitting",
    "situation",
    "size",
    "skeleton",
    "skill",
    "skin",
    "skull",
    "sky",
    "slab",
    "slacker",
    "sleep",
    "slice",
    "slob",
    "slot",
    "smoke",
    "snake",
    "snob",
    "society",
    "socket",
    "soil",
    "sole",
    "someone",
    "something",
    "son",
    "sonar",
    "song",
    "sort",
    "soul",
    "sound",
    "soup",
    "source",
    "south",
    "space",
    "spark",
    "speaker",
    "species",
    "spectacle",
    "speech",
    "speed",
    "spice",
    "spine",
    "spire",
    "spirit",
    "splendor",
    "spoke",
    "sponsor",
    "spot",
    "sprite",
    "spunk",
    "square",
    "squid",
    "stall",
    "stamina",
    "standing",
    "star",
    "starboard",
    "state",
    "statement",
    "statue",
    "status",
    "stead",
    "stealth",
    "steel",
    "step",
    "steppe",
    "stereo",
    "steward",
    "stick",
    "stinker",
    "stock",
    "stole",
    "stomach",
    "stone",
    "store",
    "storm",
    "story",
    "stranger",
    "street",
    "strength",
    "studio",
    "study",
    "stuff",
    "stump",
    "sturgeon",
    "style",
    "subject",
    "submarine",
    "substance",
    "success",
    "suit",
    "suitor",
    "summer",
    "summit",
    "summons",
    "sun",
    "surface",
    "surgery",
    "surprise",
    "surroundings",
    "suspense",
    "suspension",
    "suspicion",
    "sweat",
    "sweetie",
    "switch",
    "sword",
    "swordplay",
    "swordsman",
    "symbol",
    "system",
    "tag",
    "tail",
    "tale",
    "talent",
    "tap",
    "target",
    "task",
    "taste",
    "teacher",
    "teaching",
    "tech",
    "technique",
    "tee",
    "telepathy",
    "telescope",
    "temple",
    "terrain",
    "terror",
    "test",
    "theme",
    "thief",
    "thing",
    "thought",
    "threshold",
    "thrill",
    "thug",
    "tic",
    "ticking",
    "tide",
    "tiller",
    "time",
    "timer",
    "tip",
    "titan",
    "title",
    "toe",
    "token",
    "tomfoolery",
    "tone",
    "tongue",
    "tool",
    "tooth",
    "top",
    "topic",
    "torment",
    "tourist",
    "tower",
    "town",
    "townsfolk",
    "toy",
    "track",
    "trade",
    "trading",
    "tradition",
    "tragedy",
    "training",
    "traveler",
    "travesty",
    "treasure",
    "tree",
    "trial",
    "triangle",
    "tribe",
    "tribute",
    "trick",
    "trinket",
    "triumph",
    "trooper",
    "trouble",
    "troublemaker",
    "trust",
    "truth",
    "tune",
    "tuner",
    "tunnel",
    "turf",
    "turning",
    "turnout",
    "tutelage",
    "twig",
    "twin",
    "twister",
    "type",
    "tyrant",
    "umbrella",
    "underling",
    "underpants",
    "underwear",
    "urchin",
    "valley",
    "value",
    "variety",
    "vase",
    "vegetable",
    "venture",
    "vermin",
    "version",
    "victim",
    "victory",
    "view",
    "vigor",
    "village",
    "vintage",
    "violin",
    "vitality",
    "voice",
    "volcano",
    "volunteer",
    "voyage",
    "walking",
    "wall",
    "wallet",
    "warning",
    "water",
    "way",
    "weakling",
    "weakness",
    "wealth",
    "weapon",
    "weasel",
    "weather",
    "wedding",
    "weeds",
    "weight",
    "welcome",
    "welfare",
    "west",
    "whale",
    "whatnot",
    "wheel",
    "while",
    "whippersnapper",
    "whirlpool",
    "whirlwind",
    "wife",
    "wimp",
    "wind",
    "windfall",
    "windmill",
    "window",
    "winner",
    "winter",
    "wisdom",
    "wit",
    "witness",
    "woe",
    "woman",
    "wonder",
    "wood",
    "word",
    "work",
    "worker",
    "workplace",
    "world",
    "worm",
    "wrath",
    "wreckage",
    "wretch",
    "wrist",
    "writing",
    "yank",
    "year",
    "youngster",
    "youth",
    "zee",
    "zero"
};

std::string generate_seed() {
    const auto noun_file = Random(0, 2);
    std::string adjective1, adjective2, noun;

    adjective1 = RandomElement(adjectives);
    adjective2 = RandomElement(adjectives);

    if(noun_file == 0) {
        noun = RandomElement(names);
    }
    else {
        noun = RandomElement(nouns);
    }

    adjective1[0] = std::toupper(adjective1[0]);
    adjective2[0] = std::toupper(adjective2[0]);
    noun[0] = std::toupper(noun[0]);

    return adjective1 + adjective2 + noun;
}

std::string generate_seed_hash() {
    std::string name1, name2;

    name1 = RandomElement(short_names);
    name2 = RandomElement(short_names);

    name1[0] = std::toupper(name1[0]);
    name2[0] = std::toupper(name2[0]);

    return name1 + " " + name2;
}

std::string hash_for_seed(const std::string& seed) {
    Config config;
    config.loadFromFile(APP_SAVE_PATH "config.yaml");
    return hash_for_seed(seed, config);
}

std::string hash_for_seed(const std::string& seed, const Config& config) {
    auto permalink = create_permalink(config.settings, seed);

    if(config.settings.do_not_generate_spoiler_log) permalink += SEED_KEY;

    // Add the plandomizer file contents to the permalink when plandomzier is enabled
    if (config.settings.plandomizer) {
        std::string plandoContents = "";
        Utility::getFileContents(config.settings.plandomizerFile, plandoContents);
        permalink += plandoContents;
    }

    // Seed RNG
    auto integer_seed = zng_crc32(0L, (uint8_t*)permalink.data(), permalink.length());

    Random_Init(integer_seed);
    return generate_seed_hash();
}