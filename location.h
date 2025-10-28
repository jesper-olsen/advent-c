typedef enum {
    inhand = -1, limbo, road, hill, house, valley, forest, woods, slit, outside,
    inside, cobbles, debris, awk, bird, spit, emist, nugget, efiss, wfiss, wmist,
    like1, like2, like3, like4, like5, like6, like7, like8, like9, like10, like11, like12, like13, like14,
    brink, elong, wlong,
    diff0, diff1, diff2, diff3, diff4, diff5, diff6, diff7, diff8, diff9, diff10,
    pony, cross, hmk, west, south, ns, y2, jumble, windoe, dirty, clean, wet, dusty,
    complex, shell, arch, ragged, sac, ante, witt, bedquilt, cheese, soft, e2pit, w2pit, epit,
    wpit, narrow, giant, block, immense, falls, steep, abovep, sjunc, tite, low, crawl, window,
    oriental, misty, alcove, proom, droom, slab, abover, mirror, res, scan1, scan2, scan3,
    secret, wide, tight, tall, boulders, scorr, swside,
    dead0, dead1, dead2, dead3, dead4, dead5, dead6, dead7, dead8, dead9, dead10, dead11,
    neside, corr, fork, warm, view, chamber, lime, fbarr, barr, neend, swend, crack, neck, lose,
    cant, climb, check, snaked, thru, duck, sewer, upnout, didit,
    ppass, pdrop, troll
} location;

typedef enum : uint16_t {
    lighted = 1 << 0,
    oil     = 1 << 1,
    liquid  = 1 << 2,
    cave_hint = 1 << 3,
    bird_hint = 1 << 4,
    snake_hint = 1 << 5,
    twist_hint = 1 << 6,
    dark_hint = 1 << 7,
    witt_hint = 1 << 8,
} BitFlags;


constexpr location max_loc = didit;
char const *const all_alike = "You are in a maze of twisty little passages, all alike.";
char const *const dead_end = "Dead end.";
static char const *const description[max_loc + 1][2] = {
    [road] = {
        [0] = "You are standing at the end of a road before a small brick building.\n"
              "Around you is a forest.  A small stream flows out of the building and\n"
              "down a gully.",
        [1] = "You're at end of road again."
    },
    [hill] = {
        [0] = "You have walked up a hill, still in the forest.  The road slopes back\n"
              "down the other side of the hill.  There is a building in the distance.",
        [1] = "You're at hill in road."
    },
    [house] = {
        [0] = "You are inside a building, a well house for a large spring.",
        [1] = "You're inside building."
    },
    [valley] = {
        [0] = "You are in a valley in the forest beside a stream tumbling along a\n"
              "rocky bed.",
        [1] = "You're in valley."
    },
    [forest] = {
        [0] = "You are in open forest, with a deep valley to one side.",
        [1] = "You're in forest.",
    },
    [woods] = {
        [0] = "You are in open forest near both a valley and a road.",
        [1] = "You're in forest.",
    },
    [slit] = {
        [0] = "At your feet all the water of the stream splashes into a 2-inch slit\n"
              "in the rock.  Downstream the streambed is bare rock.",
        [1] = "You're at slit in streambed.",
    },
    [outside] = {
        [0] = "You are in a 20-foot depression floored with bare dirt.  Set into the\n"
              "dirt is a strong steel grate mounted in concrete.  A dry streambed\n"
              "leads into the depression.",
        [1] = "You're outside grate.",
    },
    [inside] = {
        [0] = "You are in a small chamber beneath a 3x3 steel grate to the surface.\n"
              "A low crawl over cobbles leads inwards to the west.",
        [1] = "You're below the grate.",
    },
    [cobbles] = {
        [0] = "You are crawling over cobbles in a low passage.  There is a dim light\n"
              "at the east end of the passage.",
        [1] = "You're in cobble crawl.",
    },
    [debris] = {
        [0] = "You are in a debris room filled with stuff washed in from the surface.\n"
              "A low wide passage with cobbles becomes plugged with mud and debris\n"
              "here, but an awkward canyon leads upward and west.  A note on the wall\n"
              "says \"MAGIC WORD XYZZY\".",
        [1] = "You're in debris room.",
    },
    [awk] = {
        [0] = "You are in an awkward sloping east/west canyon.",
        [1] = nullptr,
    },
    [bird] = {
        [0] = "You are in a splendid chamber thirty feet high.  The walls are frozen\n"
              "rivers of orange stone.  An awkward canyon and a good passage exit\n"
              "from east and west sides of the chamber.",
        [1] = "You're in bird chamber.",
    },
    [spit] = {
        [0] = "At your feet is a small pit breathing traces of white mist.  An east\n"
              "passage ends here except for a small crack leading on.",
        [1] = "You're at top of small pit.",
    },
    [emist] = {
        [0] = "You are at one end of a vast hall stretching forward out of sight to\n"
              "the west.  There are openings to either side.  Nearby, a wide stone\n"
              "staircase leads downward.  The hall is filled with wisps of white mist\n"
              "swaying to and fro almost as if alive.  A cold wind blows up the\n"
              "staircase.  There is a passage at the top of a dome behind you.",
        [1] = "You're in Hall of Mists.",
    },
    [nugget] = {
        [0] = "This is a low room with a crude note on the wall.  The note says,\n"
              "\"You won't get it up the steps\".",
        [1] = "You're in nugget of gold room.",
    },
    [efiss] = {
        [0] = "You are on the east bank of a fissure slicing clear across the hall.\n"
              "The mist is quite thick here, and the fissure is too wide to jump.",
        [1] = "You're on east bank of fissure.",
    },
    [wfiss] = {
        [0] = "You are on the west side of the fissure in the Hall of Mists.",
        [1] = nullptr,
    },
    [wmist] = {
        [0] = "You are at the west end of the Hall of Mists.  A low wide crawl\n\
continues west and another goes north.  To the south is a little\n\
passage 6 feet off the floor.",
        [1] = "You're at west end of Hall of Mists.",
    },
    [like1] = {
        [0] = all_alike,
        [1] = nullptr
    },
    [like2] = {
        [0] = all_alike,
        [1] = nullptr
    },
    [like3] = {
        [0] = all_alike,
        [1] = nullptr
    },
    [like4] = {
        [0] = all_alike,
        [1] = nullptr
    },
    [like5] = {
        [0] = all_alike,
        [1] = nullptr
    },
    [like6] = {
        [0] = all_alike,
        [1] = nullptr
    },
    [like7] = {
        [0] = all_alike,
        [1] = nullptr
    },
    [like8] = {
        [0] = all_alike,
        [1] = nullptr
    },
    [like9] = {
        [0] = all_alike,
        [1] = nullptr
    },
    [like10] = {
        [0] = all_alike,
        [1] = nullptr
    },
    [like11] = {
        [0] = all_alike,
        [1] = nullptr
    },
    [like12] = {
        [0] = all_alike,
        [1] = nullptr
    },
    [like13] = {
        [0] = all_alike,
        [1] = nullptr
    },
    [like14] = {
        [0] = all_alike,
        [1] = nullptr
    },
    [brink] = {
        [0] = "You are on the brink of a thirty-foot pit with a massive orange column\n"
              "down one wall.  You could climb down here but you could not get back\n"
              "up.  The maze continues at this level.",
        [1] = "You're at brink of pit.",
    },
    [elong] = {
        [0] = "You are at the east end of a very long hall apparently without side\n"
              "chambers.  To the east a low wide crawl slants up.  To the north a\n"
              "round two-foot hole slants down.",
        [1] = "You're at east end of long hall.",
    },
    [wlong] = {
        [0] = "You are at the west end of a very long featureless hall.  The hall\n"
              "joins up with a narrow north/south passage.",
        [1] = "You're at west end of long hall.",
    },
    [diff0] = {[0] = "You are in a maze of twisty little passages, all different."},
    [diff1] = { [0] = "You are in a maze of twisting little passages, all different." },
    [diff2] = { [0] = "You are in a little maze of twisty passages, all different." },
    [diff3] = { [0] = "You are in a twisting maze of little passages, all different."},
    [diff4] = { [0] = "You are in a twisting little maze of passages, all different."},
    [diff5] = { [0] = "You are in a twisty little maze of passages, all different."},
    [diff6] = { [0] = "You are in a twisty maze of little passages, all different."},
    [diff7] = { [0] = "You are in a little twisty maze of passages, all different."},
    [diff8] = { [0] = "You are in a maze of little twisting passages, all different." },
    [diff9] = { [0] = "You are in a maze of little twisty passages, all different."},
    [diff10] = { [0] = "You are in a little maze of twisting passages, all different."},
    [pony] = {
        [0] = dead_end,
        [1] = nullptr,
    },
    [cross] = {
        [0] = "You are at a crossover of a high N/S passage and a low E/W one.",
        [1] = nullptr,
    },
    [hmk] = {
        [0] = "You are in the Hall of the Mountain King, with passages off in all\n"
              "directions.",
        [1] = "You're in Hall of Mt King.",
    },
    [west] = {
        [0] = "You are in the west side chamber of the Hall of the Mountain King.\n"
              "A passage continues west and up here.",
        [1] = "You're in west side chamber.",
    },
    [south] = { [0] = "You are in the south side chamber." },
    [ns] = {
        [0] = "You are in a low N/S passage at a hole in the floor.  The hole goes\n"
              "down to an E/W passage.",
        [1] = "You're in N/S passage.",
    },
    [y2] = {
        [0] = "You are in a large room, with a passage to the south, a passage to the\n"
              "west, and a wall of broken rock to the east.  There is a large \"Y2\" on\n"
              "a rock in the room's center.",
        [1] = "You're at \"Y2\".",
    },
    [jumble] = {[0] = "You are in a jumble of rock, with cracks everywhere."},
    [windoe] = {
        [0] = "You're at a low window overlooking a huge pit, which extends up out of\n"
              "sight.  A floor is indistinctly visible over 50 feet below.  Traces of\n"
              "white mist cover the floor of the pit, becoming thicker to the right.\n"
              "Marks in the dust around the window would seem to indicate that\n"
              "someone has been here recently.  Directly across the pit from you and\n"
              "25 feet away there is a similar window looking into a lighted room.\n"
              "A shadowy figure can be seen there peering back at you.",
        [1] = "You're at window on pit."
    },
    [dirty] = {
        [0] = "You are in a dirty broken passage.  To the east is a crawl.  To the\n"
              "west is a large passage.  Above you is a hole to another passage.",
        [1] = "You're in dirty passage."
    },
    [clean] = {
        [0] = "You are on the brink of a small clean climbable pit.  A crawl leads\n"
              "west.",
        [1] = "You're by a clean pit."
    },
    [wet] = {
        [0] = "You are in the bottom of a small pit with a little stream, which\n"
              "enters and exits through tiny slits.",
        [1] = "You're in pit by stream."
    },
    [dusty] = {
        [0] = "You are in a large room full of dusty rocks.  There is a big hole in\n"
              "the floor.  There are cracks everywhere, and a passage leading east.",
        [1] = "You're in dusty rock room.",
    },
    [complex] = {
        [0] = "You are at a complex junction.  A low hands-and-knees passage from the\n"
              "north joins a higher crawl from the east to make a walking passage\n"
              "going west.  There is also a large room above.  The air is damp here.",
        [1] = "You're at complex junction.",
    },
    [shell] = {
        [0] = "You're in a large room carved out of sedimentary rock.  The floor\n"
              "and walls are littered with bits of shells embedded in the stone.\n"
              "A shallow passage proceeds downward, and a somewhat steeper one\n"
              "leads up.  A low hands-and-knees passage enters from the south.",
        [1] = "You're in Shell Room."
    },
    [arch] = {
        [0] = "You are in an arched hall.  A coral passage once continued up and east\n\
from here, but is now blocked by debris.  The air smells of sea water.",
        [1] = "You're in arched hall.",
    },
    [ragged] = { [0] = "You are in a long sloping corridor with ragged sharp walls." },
    [sac] = { [0] = "You are in a cul-de-sac about eight feet across." },
    [ante] = {
        [0] = "You are in an anteroom leading to a large passage to the east.  Small\n"
              "passages go west and up.  The remnants of recent digging are evident.\n"
              "A sign in midair here says \"CAVE UNDER CONSTRUCTION BEYOND THIS POINT.\n"
              "PROCEED AT OWN RISK.  [WITT CONSTRUCTION COMPANY]\"",
        [1] = "You're in anteroom.",
    },
    [witt] = {
        [0] = "You are at Witt's End.  Passages lead off in \"all\" directions.",
        [1] = "You're at Witt's End.",
    },
    [bedquilt] = {
        [0] = "You are in Bedquilt, a long east/west passage with holes everywhere.\n"
              "To explore at random select north, south, up, or down.",
        [1] = "You're in Bedquilt.",
    },
    [cheese] = {
        [0] = "You are in a room whose walls resemble Swiss cheese.  Obvious passages\n"
              "go west, east, NE, and NW.  Part of the room is occupied by a large\n"
              "bedrock block.",
        [1] = "You're in Swiss cheese room.",
    },
    [soft] = {
        [0] = "You are in the Soft Room.  The walls are covered with heavy curtains,\n"
              "the floor with a thick pile carpet.  Moss covers the ceiling.",
        [1] = "You're in Soft Room.",
    },
    [e2pit] = {
        [0] = "You are at the east end of the Twopit Room.  The floor here is\n"
              "littered with thin rock slabs, which make it easy to descend the pits.\n"
              "There is a path here bypassing the pits to connect passages from east\n"
              "and west.  There are holes all over, but the only big one is on the\n"
              "wall directly over the west pit where you can't get to it.",
        [1] = "You're at east end of Twopit Room.",
    },
    [w2pit] = {
        [0] = "You are at the west end of the Twopit Room.  There is a large hole in\n"
              "the wall above the pit at this end of the room.",
        [1] = "You're at west end of Twopit Room.",
    },
    [epit] = {
        [0] = "You are at the bottom of the eastern pit in the Twopit Room.  There is\n"
              "a small pool of oil in one corner of the pit.",
        [1] = "You're in east pit.",
    },
    [wpit] = {
        [0] = "You are at the bottom of the western pit in the Twopit Room.  There is\n"
              "a large hole in the wall about 25 feet above you.",
        [1] = "You're in west pit.",
    },
    [narrow] = {
        [0] = "You are in a long, narrow corridor stretching out of sight to the\n"
              "west.  At the eastern end is a hole through which you can see a\n"
              "profusion of leaves.",
        [1] = "You're in narrow corridor.",
    },
    [giant] = {
        [0] = "You are in the Giant Room.  The ceiling here is too high up for your\n"
              "lamp to show it.  Cavernous passages lead east, north, and south.  On\n"
              "the west wall is scrawled the inscription, \"FEE FIE FOE FOO\" [sic].",
        [1] = "You're in Giant Room.",
    },
    [block] = { [0] = "The passage here is blocked by a recent cave-in.", },
    [immense] = { [0] = "You are at one end of an immense north/south passage." },
    [falls] = {
        [0] = "You are in a magnificent cavern with a rushing stream, which cascades\n"
              "over a sparkling waterfall into a roaring whirlpool that disappears\n"
              "through a hole in the floor.  Passages exit to the south and west.",
        [1] = "You're in cavern with waterfall."
    },
    [steep] = {
        [0] = "You are at the top of a steep incline above a large room.  You could\n"
              "climb down here, but you would not be able to climb up.  There is a\n"
              "passage leading back to the north.",
        [1] = "You're at steep incline above large room."
    },
    [abovep] = { [0] = "You are in a secret N/S canyon above a sizable passage.", },
    [sjunc] = {
        [0] = "You are in a secret canyon at a junction of three canyons, bearing\n"
              "north, south, and SE.  The north one is as tall as the other two\n"
              "combined.",
        [1] = "You're at junction of three secret canyons.",
    },
    [tite] = {
        [0] = "A large stalactite extends from the roof and almost reaches the floor\n"
              "below.  You could climb down it, and jump from it to the floor, but\n"
              "having done so you would be unable to reach it to climb back up.",
        [1] = "You're on top of stalactite.",
    },
    [low] = { [0] = "You are in a large low room.  Crawls lead north, SE, and SW." },
    [crawl] = { [0] = "Dead end crawl." },
    [window] = {
        [0] = "You're at a low window overlooking a huge pit, which extends up out of\n"
              "sight.  A floor is indistinctly visible over 50 feet below.  Traces of\n"
              "white mist cover the floor of the pit, becoming thicker to the left.\n"
              "Marks in the dust around the window would seem to indicate that\n"
              "someone has been here recently.  Directly across the pit from you and\n"
              "25 feet away there is a similar window looking into a lighted room.\n"
              "A shadowy figure can be seen there peering back at you.",
        [1] = "You're at window on pit.",
    },
    [oriental] = {
        [0] = "This is the Oriental Room.  Ancient oriental cave drawings cover the\n"
              "walls.  A gently sloping passage leads upward to the north, another\n"
              "passage leads SE, and a hands-and-knees crawl leads west.",
        [1] = "You're in Oriental Room.",
    },
    [misty] = {
        [0] = "You are following a wide path around the outer edge of a large cavern.\n"
              "Far below, through a heavy white mist, strange splashing noises can be\n"
              "heard.  The mist rises up through a fissure in the ceiling.  The path\n"
              "exits to the south and west.",
        [1] = "You're in misty cavern.",
    },
    [alcove] = {
        [0] = "You are in an alcove.  A small NW path seems to widen after a short\n"
              "distance.  An extremely tight tunnel leads east.  It looks like a very\n"
              "tight squeeze.  An eerie light can be seen at the other end.",
        [1] = "You're in alcove.",
    },
    [proom] = {
        [0] = "You're in a small chamber lit by an eerie green light.  An extremely\n\
narrow tunnel exits to the west.  A dark corridor leads NE.",
        [1] = "You're in Plover Room.",
    },
    [droom] = {
        [0] = "You're in the Dark-Room.  A corridor leading south is the only exit.",
        [1] = "You're in Dark-Room.",
    },
    [slab] = {
        [0] = "You are in a large low circular chamber whose floor is an immense slab\n"
              "fallen from the ceiling (Slab Room).  There once were large passages\n"
              "to the east and west, but they are now filled with boulders.  Low\n"
              "small passages go north and south, and the south one quickly bends\n"
              "east around the boulders.",
        [1] = "You're in Slab Room.",
    },
    [abover] = { [0] = "You are in a secret N/S canyon above a large room."},
    [mirror] = {
        [0] = "You are in a north/south canyon about 25 feet across.  The floor is\n"
              "covered by white mist seeping in from the north.  The walls extend\n"
              "upward for well over 100 feet.  Suspended from some unseen point far\n"
              "above you, an enormous two-sided mirror is hanging parallel to and\n"
              "midway between the canyon walls.  (The mirror is obviously provided\n"
              "for the use of the dwarves, who as you know are extremely vain.)\n"
              "A small window can be seen in either wall, some fifty feet up.",
        [1] = "You're in mirror canyon.",
    },
    [res] = {
        [0] = "You are at the edge of a large underground reservoir.  An opaque cloud\n"
              "of white mist fills the room and rises rapidly upward.  The lake is\n"
              "fed by a stream, which tumbles out of a hole in the wall about 10 feet\n"
              "overhead and splashes noisily into the water somewhere within the\n"
              "mist.  The only passage goes back toward the south.",
        [1] = "You're at reservoir.",
    },
    [scan1] = { [0] = "You are in a secret canyon that exits to the north and east."},
    [scan2] = { [0] = "You are in a secret canyon that exits to the north and east."},
    [scan3] = { [0] = "You are in a secret canyon that exits to the north and east."},
    [secret] = {
        [0] = "You are in a secret canyon, which here runs E/W.  It crosses over a\n"
              "very tight canyon 15 feet below.  If you go down you may not be able\n"
              "to get back up.",
        [1] = "You're in secret E/W canyon above tight canyon.",
    },
    [wide] = {
        [0] = "You are at a wide place in a very tight N/S canyon.",
    },
    [tight] = { [0] = "The canyon here becomes too tight to go further south." },
    [tall] = {
        [0] = "You are in a tall E/W canyon.  A low tight crawl goes 3 feet north and\n"
              "seems to open up.",
        [1] = "You're in tall E/W canyon."
    },
    [boulders] = { [0] = "The canyon runs into a mass of boulders --- dead end." },
    [scorr] = {
        [0] = "You are in a long winding corridor sloping out of sight in both\n"
              "directions.",
        [1] = "You're in sloping corridor."
    },
    [swside] = {
        [0] = "You are on one side of a large, deep chasm.  A heavy white mist rising\n"
              "up from below obscures all view of the far side.  A SW path leads away\n"
              "from the chasm into a winding corridor.",
        [1] = "You're on SW side of chasm."
    },
    [dead0] = { [0] = dead_end },
    [dead1] = { [0] = dead_end },
    [dead2] = { [0] = dead_end },
    [dead3] = { [0] = dead_end },
    [dead4] = { [0] = dead_end },
    [dead5] = { [0] = dead_end },
    [dead6] = { [0] = dead_end },
    [dead7] = { [0] = dead_end },
    [dead8] = { [0] = dead_end },
    [dead9] = { [0] = dead_end },
    [dead10] = { [0] = dead_end },
    [dead11] = { [0] = dead_end },
    [neside] = {
        [0] = "You are on the far side of the chasm.  A NE path leads away from the\n"
              "chasm on this side.",
        [1] = "You're on NE side of chasm."
    },
    [corr] = {
        [0] = "You're in a long east/west corridor.  A faint rumbling noise can be\n"
              "heard in the distance.",
        [1] = "You're in corridor."
    },
    [fork] = {
        [0] = "The path forks here.  The left fork leads northeast.  A dull rumbling\n"
              "seems to get louder in that direction.  The right fork leads southeast\n"
              "down a gentle slope.  The main corridor enters from the west.",
        [1] = "You're at fork in path."
    },
    [warm] = {
        [0] = "The walls are quite warm here.  From the north can be heard a steady\n"
              "roar, so loud that the entire cave seems to be trembling.  Another\n"
              "passage leads south, and a low crawl goes east.",
        [1] = "You're at junction with warm walls.",
    },
    [view] = {
        [0] = "You are on the edge of a breath-taking view.  Far below you is an\n"
              "active volcano, from which great gouts of molten lava come surging\n"
              "out, cascading back down into the depths.  The glowing rock fills the\n"
              "farthest reaches of the cavern with a blood-red glare, giving every-\n"
              "thing an eerie, macabre appearance.  The air is filled with flickering\n"
              "sparks of ash and a heavy smell of brimstone.  The walls are hot to\n"
              "the touch, and the thundering of the volcano drowns out all other\n"
              "sounds.  Embedded in the jagged roof far overhead are myriad twisted\n"
              "formations, composed of pure white alabaster, which scatter the murky\n"
              "light into sinister apparitions upon the walls.  To one side is a deep\n"
              "gorge, filled with a bizarre chaos of tortured rock that seems to have\n"
              "been crafted by the Devil himself.  An immense river of fire crashes\n"
              "out from the depths of the volcano, burns its way through the gorge,\n"
              "and plummets into a bottomless pit far off to your left.  To the\n"
              "right, an immense geyser of blistering steam erupts continuously\n"
              "from a barren island in the center of a sulfurous lake, which bubbles\n"
              "ominously.  The far right wall is aflame with an incandescence of its\n"
              "own, which lends an additional infernal splendor to the already\n"
              "hellish scene.  A dark, foreboding passage exits to the south.",
        [1] = "You're at breath-taking view."
    },
    [chamber] = {
        [0] = "You are in a small chamber filled with large boulders.  The walls are\n"
              "very warm, causing the air in the room to be almost stifling from the\n"
              "heat.  The only exit is a crawl heading west, through which a low\n"
              "rumbling noise is coming.",
        [1] = "You're in chamber of boulders."
    },
    [lime] = {
        [0] = "You are walking along a gently sloping north/south passage lined with\n"
              "oddly shaped limestone formations.",
        [1] = "You're in limestone passage."
    },
    [fbarr] = {
        [0] = "You are standing at the entrance to a large, barren room.  A sign\n"
              "posted above the entrance reads:  \"CAUTION!  BEAR IN ROOM!\"",
        [1] = "You're in front of barren room."
    },
    [barr] = {
        [0] = "You are inside a barren room.  The center of the room is completely\n"
              "empty except for some dust.  Marks in the dust lead away toward the\n"
              "far end of the room.  The only exit is the way you came in.",
        [1] = "You're in barren room."
    },
    [neend] = {
        [0] = "You are at the northeast end of an immense room, even larger than the\n"
              "Giant Room.  It appears to be a repository for the \"Adventure\"\n"
              "program.  Massive torches far overhead bathe the room with smoky\n"
              "yellow light.  Scattered about you can be seen a pile of bottles (all\n"
              "of them empty), a nursery of young beanstalks murmuring quietly, a bed\n"
              "of oysters, a bundle of black rods with rusty stars on their ends, and\n"
              "a collection of brass lanterns.  Off to one side a great many dwarves\n"
              "are sleeping on the floor, snoring loudly.  A sign nearby reads:  \"DO\n"
              "NOT DISTURB THE DWARVES!\"  An immense mirror is hanging against one\n"
              "wall, and stretches to the other end of the room, where various other\n"
              "sundry objects can be glimpsed dimly in the distance.",
        [1] = "You're at NE end."
    },
    [swend] = {
        [0] = "You are at the southwest end of the repository.  To one side is a pit\n"
              "full of fierce green snakes.  On the other side is a row of small\n"
              "wicker cages, each of which contains a little sulking bird.  In one\n"
              "corner is a bundle of black rods with rusty marks on their ends.\n"
              "A large number of velvet pillows are scattered about on the floor.\n"
              "A vast mirror stretches off to the northeast.  At your feet is a\n"
              "large steel grate, next to which is a sign that reads, \"TREASURE\n"
              "VAULT.  KEYS IN MAIN OFFICE.\"",
        [1] = "You're at SW end."
    },
    [crack] = { [0] = "The crack is far too small for you to follow."},
    [neck] = {[0] = "You are at the bottom of the pit with a broken neck."},
    [lose] = {[0] = "You didn't make it."},
    [cant] = {[0] = "The dome is unclimbable."},
    [climb] = {[0] = "You clamber up the plant and scurry through the hole at the top."},
    [check] = {},
    [snaked] = {[0] = "You can't get by the snake."},
    [thru] = {
        [0] = "You have crawled through a very low wide passage parallel to and north\n"
              "of the Hall of Mists."
    },
    [duck] = {
        [0] = "You have crawled through a very low wide passage parallel to and north\n"
              "of the Hall of Mists."
    },
    [sewer] = {
        [0] = "The stream flows out through a pair of 1-foot-diameter sewer pipes.\n"
              "It would be advisable to use the exit."
    },
    [upnout] = {
        [0] = "There is nothing here to climb.  Use \"up\" or \"out\" to leave the pit."
    },
    [didit] = { [0] = "You have climbed up the plant and out of the pit." },
};

char const *get_description_long(location loc) {
    return description[loc][0];
}

char const *get_description_short(location loc) {
    return description[loc][1];
}

// bitmaps for special properties
const uint16_t flags[max_loc + 1] = {
    [road] = lighted + liquid,
    [hill] = lighted,
    [house] = lighted + liquid,
    [valley] = lighted + liquid,
    [forest] = lighted,
    [woods] = lighted,
    [slit] = lighted + liquid,
    [outside] = lighted + cave_hint,
    [inside] = lighted,
    [cobbles] = lighted,
    [bird] = bird_hint,
    [like1] = twist_hint,
    [like2] = twist_hint,
    [like3] = twist_hint,
    [like4] = twist_hint,
    [like5] = twist_hint,
    [like6] = twist_hint,
    [like7] = twist_hint,
    [like8] = twist_hint,
    [like9] = twist_hint,
    [like10] = twist_hint,
    [like11] = twist_hint,
    [like12] = twist_hint,
    [like13] = twist_hint,
    [like14] = twist_hint,
    [hmk] = snake_hint,
    [wet] = liquid,
    [witt] = witt_hint,
    [epit] = liquid + oil,
    [falls] = liquid,
    [alcove] = dark_hint,
    [proom] = lighted + dark_hint,
    [droom] = dark_hint,
    [res] = liquid,
    [dead1] = twist_hint,
    [dead2] = 0,
    [dead3] = twist_hint,
    [dead4] = twist_hint,
    [dead5] = twist_hint,
    [dead6] = twist_hint,
    [dead7] = twist_hint,
    [dead8] = twist_hint,
    [dead9] = twist_hint,
    [dead10] = twist_hint,
    [dead11] = twist_hint,
    [view] = lighted,
    [neend] = lighted,
    [swend] = lighted,
};

//ROAD2 0
//#define HILL2 1
//#define BUILDING2 2
//#define EAST2 3
//#define WEST2 3
//
//typedef struct connection {
//    int mot;
//    int cond;
//    int dest;
//} connection;
//
//typedef struct location {
//    const char *long_desc;
//    const char *short_desc;
//    const connection *connections;
//    size_t num_connections;
//} location2;
//
//static const location2 locations[] = {
//    [ROAD2] = {
//        .long_desc = "You are standing at the end of a road...",
//        .short_desc = "End of road",
//        .connections = (const connection[]){
//            {WEST2, 0, HILL2},
//            {EAST2, 0, BUILDING2},
//        },
//        .num_connections = 2
//    },
//};
//
