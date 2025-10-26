typedef enum : int {
    NOTHING, KEYS, LAMP, GRATE, GRATE_, CAGE, ROD, ROD2, TREADS, TREADS_, BIRD, DOOR,
    PILLOW, SNAKE, CRYSTAL, CRYSTAL_, TABLET, CLAM, OYSTER, MAG, DWARF, KNIFE, FOOD,
    BOTTLE, WATER, OIL, MIRROR, MIRROR_, PLANT, PLANT2, PLANT2_, STALACTITE, SHADOW,
    SHADOW_, AXE, ART, PIRATE, DRAGON, DRAGON_, BRIDGE, BRIDGE_, TROLL, TROLL_, TROLL2, TROLL2_,
    BEAR, MESSAGE, GEYSER, PONY, BATTERIES, MOSS, GOLD, DIAMONDS, SILVER, JEWELS, COINS, CHEST,
    EGGS, TRIDENT, VASE, EMERALD, PYRAMID, PEARL, RUG, RUG_, SPICES, CHAIN
} object;

constexpr object max_obj = CHAIN;
constexpr object min_treasure = GOLD;

static inline bool is_treasure(object t) {
    return t >= min_treasure;
}

// object names (for inventory listing).
// Some objects don't have a name - they are initialised as 0 (not "")
char const *const obj_name[max_obj + 1] = {
    [RUG] = "Persian rug",
    [CHAIN] = "Golden chain",
    [SPICES] = "Rare spices",
    [PEARL] = "Glistening pearl",
    [PYRAMID] = "Platinum pyramid",
    [EMERALD] = "Egg-sized emerald",
    [VASE] = "Ming vase",
    [TRIDENT] = "Jeweled trident",
    [EGGS] = "Golden eggs",
    [CHEST] = "Treasure chest",
    [COINS] = "Rare coins",
    [JEWELS] = "Precious jewelry",
    [SILVER] = "Bars of silver",
    [DIAMONDS] = "Several diamonds",
    [GOLD] = "Large gold nugget",
    [BATTERIES] = "Batteries",
    [AXE] = "Dwarf's axe",
    [OIL] = "Oil in the bottle",
    [WATER] = "Water in the bottle",
    [BOTTLE] = "Small bottle",
    [FOOD] = "Tasty food",
    [MAG] = "\"Spelunker Today\"",
    [OYSTER] = "Giant oyster >GROAN!<",
    [CLAM] = "Giant clam >GRUNT!<",
    [PILLOW] = "Velvet pillow",
    [BIRD] = "Little bird in cage",
    [ROD2] = "Black rod",
    [ROD] = "Black rod",
    [CAGE] = "Wicker cage",
    [LAMP] = "Brass lantern",
    [KEYS] = "Set of keys"
};

static constexpr size_t MAX_PROPERTIES = 6;

// notes - notes describe property values
static char const *const object_notes[max_obj + 1][MAX_PROPERTIES] = {
    [RUG] = {
        [0] = "There is a Persian rug spread out on the floor!",
        [1] = "The dragon is sprawled out on a Persian rug!!",
    },
    [TROLL2] = {
        [0] = "The troll is nowhere to be seen."
    },
    [TROLL] = {
        [0] = "A burly troll stands by the bridge and insists you throw him a\n"
              "treasure before you may cross.",
        [1] = "The troll steps out from beneath the bridge and blocks your way."
    },
    [BRIDGE] = {
        [0] = "A rickety wooden bridge extends across the chasm, vanishing into the\n"
              "mist.  A sign posted on the bridge reads, \"STOP!  PAY TROLL!\"",
        [1] = "The wreckage of a bridge (and a dead bear) can be seen at the bottom of the chasm."
    },
    [DRAGON] = {
        [0] = "A huge green fierce dragon bars the way!",
        [1] = "Congratulations!  You have just vanquished a dragon with your bare\n"
              "hands! (Unbelievable, isn't it?)",
        [2] = "The body of a huge green dead dragon is lying off to one side."
    },
    [SHADOW] = {
        [0] = "The shadowy figure seems to be trying to attract your attention."
    },
    [PLANT2] = {
        [0] = nullptr,
        [1] = "The top of a 12-foot-tall beanstalk is poking out of the west pit.",
        [2] = "There is a huge beanstalk growing out of the west pit up to the hole."
    },
    [CRYSTAL] = {
        [0] = nullptr,
        [1] = "A crystal bridge now spans the fissure.",
        [2] = "The crystal bridge has vanished!",
    },
    [TREADS] = {
        [0] = "Rough stone steps lead down the pit.",
        [1] = "Rough stone steps lead up the dome."
    },
    [GRATE] = {
        [0] = "The grate is locked.",
        [1] = "The grate is open."
    },
    [CHAIN] = {
        [0] = "There is a golden chain lying in a heap on the floor!",
        [1] = "The bear is locked to the wall with a golden chain!",
        [2] = "There is a golden chain locked to the wall!"
    },
    [SPICES] = {
        [0] = "There are rare spices here!",
    },
    [PEARL] = {
        [0] = "Off to one side lies a glistening pearl!",
    },
    [PYRAMID] = {
        [0] = "There is a platinum pyramid here, 8 inches on a side!"
    },
    [EMERALD] = {
        [0] = "There is an emerald here the size of a plover's egg!",
    },
    [VASE] = {
        [0] = "There is a delicate, precious, Ming vase here!",
        [1] = "The vase is now resting, delicately, on a velvet pillow.",
        [2] = "The floor is littered with worthless shards of pottery.",
        [3] = "The Ming vase drops with a delicate crash."
    },
    [TRIDENT] = {
        [0] = "There is a jewel-encrusted trident here!"
    },
    [EGGS] = {
        [0] = "There is a large nest here, full of golden eggs!",
        [1] = "The nest of golden eggs has vanished!",
        [2] = "Done!",
    },
    [CHEST] = {
        [0] = "The pirate's treasure chest is here!"
    },
    [COINS] = {
        [0] = "There are many coins here!"
    },
    [JEWELS] = {
        [0] = "There is precious jewelry here!"
    },
    [SILVER] = {
        [0] = "There are bars of silver here!"
    },
    [DIAMONDS] = {
        [0] = "There are diamonds here!"
    },
    [GOLD] = {
        [0] = "There is a large sparkling nugget of gold here!"
    },
    [BATTERIES] = {
        [0] = "There are fresh batteries here.",
        [1] = "Some worn-out batteries have been discarded nearby."
    },
    [PONY] = {
        [0] = "There is a massive vending machine here.  The instructions on it read:\n"
              "\"Drop coins here to receive fresh batteries.\""
    },
    [MESSAGE] = {
        [0] = "There is a message scrawled in the dust in a flowery script, reading:\n"
              "\"This is not the maze where the pirate leaves his treasure chest.\""
    },
    [BEAR] = {
        [0] = "There is a ferocious cave bear eying you from the far end of the room!",
        [1] = "There is a gentle cave bear sitting placidly in one corner.",
        [2] = "There is a contented-looking bear wandering about nearby."
    },
    [AXE] = {
        [0] = "There is a little axe here.",
        [1] = "There is a little axe lying beside the bear."
    },
    [PLANT] = {
        [0] = "There is a tiny little plant in the pit, murmuring \"Water, water, ...\"",
        [1] = "The plant spurts into furious growth for a few seconds.",
        [2] = "There is a 12-foot-tall beanstalk stretching up out of the pit,\n"
              "bellowing \"Water!!  Water!!\"",
        [3] = "The plant grows explosively, almost filling the bottom of the pit.",
        [4] = "There is a gigantic beanstalk stretching all the way up to the hole.",
        [5] = "You've over-watered the plant!  It's shriveling up!  It's, it's..."
    },
    [BOTTLE] = {
        [0] = "There is a bottle of water here.",
        [1] = "There is an empty bottle here.",
        [2] = "There is a bottle of oil here."
    },
    [FOOD] = {
        [0] = "There is food here."
    },
    [MAG] = {
        [0] = "There are a few recent issues of \"Spelunker Today\" magazine here."
    },
    [OYSTER] = {
        [0] = "There is an enormous oyster here with its shell tightly closed.",
        [1] = "Interesting.  There seems to be something written on the underside of\nthe oyster."
    },
    [CLAM] = {
        [0] = "There is an enormous clam here with its shell tightly closed."
    },
    [TABLET] = {
        [0] = "A massive stone tablet embedded in the wall reads:\n"
              "\"CONGRATULATIONS ON BRINGING LIGHT INTO THE DARK-ROOM!\""
    },
    [SNAKE] = {
        [0] = "A huge green fierce snake bars the way!"
    },
    [PILLOW] = {
        [0] = "A small velvet pillow lies on the floor."
    },
    [DOOR] = {
        [0] = "The way north is barred by a massive, rusty, iron door.",
        [1] = "The way north leads through a massive, rusty, iron door."
    },
    [BIRD] = {
        [0] = "A cheerful little bird is sitting here singing.",
        [1] = "There is a little bird in the cage."
    },
    [ROD2] = {
        [0] = "A three-foot black rod with a rusty mark on an end lies nearby."
    },
    [CAGE] = {
        [0] = "There is a small wicker cage discarded nearby."
    },
    [LAMP] = {
        [0] = "There is a shiny brass lamp nearby.",
        [1] = "There is a lamp shining nearby."
    },
    [KEYS] = {
        [0] = "There are some keys on the ground here."
    }
};


char const *get_note(int obj, int property) {
    if (property >= 0 && property < (int) MAX_PROPERTIES)
        return object_notes[obj][property];
    return nullptr;
}


