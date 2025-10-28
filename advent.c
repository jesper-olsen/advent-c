#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "object.h"
#include "location.h"

constexpr int HASH_PRIME = 1009;
#define travel_size 739
#define make_loc(x)                                                            \
  {                                                                            \
    start[x] = q;                                                              \
  }
#define make_inst(m, c, d)                                                     \
  {                                                                            \
    q->mot = m;                                                                \
    q->cond = c;                                                               \
    q->dest = d;                                                               \
    q++;                                                                       \
  }
#define ditto(m)                                                               \
  {                                                                            \
    q->mot = m;                                                                \
    q->cond = (q - 1)->cond;                                                   \
    q->dest = (q - 1)->dest;                                                   \
    q++;                                                                       \
  }
#define holds(o) (100 + (o))
#define sees(o) (200 + (o))
#define not(o, k) (300 + (o) + 100 * (k))
#define twist(l, n, s, e, w, ne, se, nw, sw, u, d)                             \
  make_loc(l);                                                              \
  make_inst(N, 0, n);                                                          \
  make_inst(S, 0, s);                                                          \
  make_inst(E, 0, e);                                                          \
  make_inst(W, 0, w);                                                          \
  make_inst(NE, 0, ne);                                                        \
  make_inst(SE, 0, se);                                                        \
  make_inst(NW, 0, nw);                                                        \
  make_inst(SW, 0, sw);                                                        \
  make_inst(U, 0, u);                                                          \
  make_inst(D, 0, d);
#define forced_move(loc) (loc >= min_forced_loc)
#define FORCE 0
#define water_here ((flags[loc] & (liquid + oil)) == liquid)
#define oil_here ((flags[loc] & (liquid + oil)) == liquid + oil)
#define no_liquid_here ((flags[loc] & liquid) == 0)
#define try_motion(m)                                                          \
  {                                                                            \
    mot = m;                                                                   \
    goto try_move;                                                             \
  }
#define stay_put try_motion(NOWHERE)
#define report(m)                                                              \
  {                                                                            \
    printf("%s\n", m);                                                         \
    continue;                                                                  \
  }
#define default_to(v) report(default_msg[v])
#define change_to(v)                                                           \
  {                                                                            \
    oldverb = verb;                                                            \
    verb = v;                                                                  \
    goto transitive;                                                           \
  }
#define dark ((flags[loc] & lighted) == 0 && (prop[LAMP] == 0 || !here(LAMP)))
#define object_in_bottle                                                       \
  ((obj == WATER && prop[BOTTLE] == 0) || (obj == OIL && prop[BOTTLE] == 2))
#define bottle_empty (prop[BOTTLE] == 1 || prop[BOTTLE] < 0)
#define clam_oyster (obj == CLAM ? "clam" : "oyster")
#define pirate_not_spotted (place[MESSAGE] == limbo)
#define too_easy(i) (i == PYRAMID && (loc == proom || loc == droom))
#define closing (clock1 < 0)
constexpr int MAX_SCORE = 350;
constexpr int HIGHEST_CLASS = 8;

typedef enum {
    no_type,
    motion_type,
    object_type,
    action_type,
    message_type
} wordtype;

typedef struct {
    char text[6];
    char word_type;
    unsigned char meaning;
} hash_entry;

typedef enum {
    N, S, E, W, NE, SE, NW, SW, U, D, L, R, IN, OUT, FORWARD, BACK, OVER, ACROSS,
    UPSTREAM, DOWNSTREAM, ENTER, CRAWL, JUMP, CLIMB, LOOK, CROSS, ROAD, WOODS, VALLEY,
    HOUSE, GULLY, STREAM, DEPRESSION, ENTRANCE, CAVE, ROCK, SLAB, BED, PASSAGE,
    CAVERN, CANYON, AWKWARD, SECRET, BEDQUILT, RESERVOIR, GIANT, ORIENTAL, SHELL,
    BARREN, BROKEN, DEBRIS, VIEW, FORK, PIT, SLIT, CRACK, DOME, HOLE, WALL, HALL,
    ROOM, FLOOR, STAIRS, STEPS, COBBLES, SURFACE, DARK, LOW, OUTDOORS, Y2, XYZZY,
    PLUGH, PLOVER, OFFICE, NOWHERE
} motion;

typedef enum {
    ABSTAIN, TAKE, DROP, OPEN, CLOSE, ON, OFF, WAVE, CALM, GO, RELAX, POUR, EAT, DRINK, RUB,
    TOSS, WAKE, FEED, FILL, BREAK, BLAST, KILL, SAY, READ, FEEFIE, BRIEF, FIND, INVENTORY,
    SCORE, QUIT
} action;

// ANO_EXPLOSION1-3 are triggered by the context rather than by words; Their integer value is significant
enum {
    MAGIC, HELP, TREE, DIG, LOST, ANO_EXPLOSION1, ANO_EXPLOSION2, MIST, FUCK, ANO_EXPLOSION3,
    STOP, INFO, SWIM
} MessageTriggers;

char const *const pitch_dark_msg = "It is now pitch dark.  "
                                   "If you proceed you will most likely fall into a pit.";
const char *const ok = "OK.";

char const *const message[13] = {
    [MAGIC] = "Good try, but that is an old worn-out magic word.",
    [HELP] = "I know of places, actions, and things.  Most of my vocabulary\n"
             "describes places and is used to move you there.  To move, try words\n"
             "like forest, building, downstream, enter, east, west, north, south,\n"
             "up, or down.  I know about a few special objects, like a black rod\n"
             "hidden in the cave.  These objects can be manipulated using some of\n"
             "the action words that I know.  Usually you will need to give both the\n"
             "object and action words (in either order), but sometimes I can infer\n"
             "the object from the verb alone.  Some objects also imply verbs; in\n"
             "particular, \"inventory\" implies \"take inventory\", which causes me to\n"
             "give you a list of what you're carrying.  The objects have side\n"
             "effects; for instance, the rod scares the bird.  Usually people having\n"
             "trouble moving just need to try a few more words.  Usually people\n"
             "trying unsuccessfully to manipulate an object are attempting something\n"
             "beyond their (or my!) capabilities and should try a completely\n"
             "different tack.  To speed the game you can sometimes move long\n"
             "distances with a single word.  For example, \"building\" usually gets\n"
             "you to the building from anywhere above ground except when lost in the\n"
             "forest.  Also, note that cave passages turn a lot, and that leaving a\n"
             "room to the north does not guarantee entering the next from the south.\n"
             "Good luck!",
    [TREE] = "The trees of the forest are large hardwood oak and maple, with an\n"
             "occasional grove of pine or spruce.  There is quite a bit of under-\n"
             "growth, largely birch and ash saplings plus nondescript bushes of\n"
             "various sorts.  This time of year visibility is quite restricted by\n"
             "all the leaves, but travel is quite easy if you detour around the\n"
             "spruce and berry bushes.",
    [DIG] = "Digging without a shovel is quite impractical.  Even with a shovel\n"
             "progress is unlikely.",
    [LOST] = "I'm as confused as you are.",
    [ANO_EXPLOSION1] = "There is a loud explosion and you are suddenly splashed across the\n"
                       "walls of the room.",
    [ANO_EXPLOSION2] = "There is a loud explosion and a twenty-foot hole appears in the far\n"
                       "wall, burying the snakes in the rubble.  A river of molten lava pours\n"
                       "in through the hole, destroying everything in its path, including you!",
    [MIST] = "Mist is a white vapor, usually water, seen from time to time in\n"
                       "caverns.  It can be found anywhere but is frequently a sign of a deep\n"
                       "pit leading down to water.",
    [FUCK] = "Watch it!",
    [ANO_EXPLOSION3] = "There is a loud explosion, and a twenty-foot hole appears in the far\n"
                       "wall, burying the dwarves in the rubble.  You march through the hole\n"
                       "and find yourself in the main office, where a cheering band of\n"
                       "friendly elves carry the conquering adventurer off into the sunset.",
    [STOP] = "I don't know the word \"stop\".  Use \"quit\" if you want to give up.",
    [INFO] = "If you want to end your adventure early, say \"quit\".  To get full\n"
                       "credit for a treasure, you must have left it safely in the building,\n"
                       "though you get partial credit just for locating it.  You lose points\n"
                       "for getting killed, or for quitting, though the former costs you more.\n"
                       "There are also points based on how much (if any) of the cave you've\n"
                       "managed to explore; in particular, there is a large bonus just for\n"
                       "getting in (to distinguish the beginners from the rest of the pack),\n"
                       "and there are other ways to determine whether you've been through some\n"
                       "of the more harrowing sections.  If you think you've found all the\n"
                       "treasures, just keep exploring for a while.  If nothing interesting\n"
                       "happens, you haven't found them all yet.  If something interesting\n"
                       "DOES happen, it means you're getting a bonus and have an opportunity\n"
                       "to garner many more points in the master's section.\n"
                       "I may occasionally offer hints if you seem to be having trouble.\n"
                       "If I do, I'll warn you in advance how much it will affect your score\n"
                       "to accept the hints.  Finally, to save paper, you may specify \"brief\",\n"
                       "which tells me never to repeat the full description of a place\n"
                       "unless you explicitly ask me to.",
    [SWIM] = "I don't know how."
};

// default response to action
char const *const default_msg[] = {
    [TAKE] = "You are already carrying it!",
    [OPEN] = "I don't know how to lock or unlock such a thing.",
    [CLOSE] = "I don't know how to lock or unlock such a thing.",
    [ON] = "You have no source of light.",
    [OFF] = "You have no source of light.",
    [WAVE] = "Nothing happens.",
    [CALM] = "I'm game.  Would you care to explain how?",
    [GO] = "Where?",
    [RELAX] = ok,
    [DROP] = "You aren't carrying it!",
    [TOSS] = "You aren't carrying it!",
    [POUR] = "You aren't carrying it!",
    [EAT] = "Don't be ridiculous!",
    [KILL] = "Don't be ridiculous!",
    [WAKE] = "Don't be ridiculous!",
    [DRINK] = "You have taken a drink from the stream.  The water tastes strongly of\n"
              "minerals, but is not unpleasant.  It is extremely cold.",
    [RUB] = "Rubbing the electric lamp is not particularly rewarding.  Anyway,\n"
              "nothing exciting happens.",
    [FEED] = "There is nothing here to eat.",
    [FILL] = "You can't fill that.",
    [BREAK] = "It is beyond your power to do that.",
    [BLAST] = "Blasting requires dynamite.",
    [READ] = "I'm afraid I don't understand.",
    [FEEFIE] = "I don't know how.",
    [BRIEF] = "On what?",
    [FIND] = "I can only tell you what you see as you move about and manipulate\n"
               "things.  I cannot tell you where remote things are.",
    [INVENTORY] = "I can only tell you what you see as you move about and manipulate\n"
                  "things.  I cannot tell you where remote things are.",
    [SCORE] = "Eh?",
    [QUIT] = "Eh?",
};

constexpr location max_pirate_loc = dead2;
constexpr location min_in_cave = inside;
constexpr location min_lower_loc = emist;
constexpr location min_forced_loc = crack;  // crack and above are pseudo-locations
constexpr location max_spec = troll;
constexpr int REM_SIZE = 15;
constexpr location message_loc = pony;
constexpr location chest_loc = dead2;
constexpr int sayit = max_spec + REM_SIZE;

typedef struct {
    motion mot;
    int cond;
    location dest;
} instruction;

hash_entry hash_table[HASH_PRIME];
wordtype current_type;

instruction travels[travel_size];
instruction *start[max_loc + 2];
char const *const remarks[REM_SIZE] = {
    [1] = "You don't fit through a two-inch slit!",
    [2] = "You can't go through a locked steel grate!",
    [3] = "I respectfully suggest you go across the bridge instead of jumping.",
    [4] = "There is no way across the fissure.",
    [5] = "You can't fit this five-foot clam through that little passage!",
    [6] = "You can't fit this five-foot oyster through that little passage!",
    [7] = "You have crawled around in some little holes and wound up back in the\n"
          "main passage.",
    [8] = "You have crawled around in some little holes and found your way\n"
          "blocked by a recent cave-in.  You are now back in the main passage.",
    [9] = "It is too far up for you to reach.",
    [10] = "The door is extremely rusty and refuses to open.",
    [11] = "The dragon looks rather nasty.  You'd best not try to get by.",
    [12] = "The troll refuses to let you cross.",
    [13] = "There is no longer any way across the chasm.",
    [14] = default_msg[EAT],
};
int visits[max_loc + 1]; // how often have you been here?

object first[max_loc + 1];  // first object present at a location
object link[max_obj + 1];   // next object present in the same location
object base[max_obj + 2];   // the smallestobject in each object's group, if any
int prop[max_obj + 1];      // current property value for each object

location place[max_obj + 1];  // each objects current location

static inline bool toting(object t) {
    return place[t] < 0;
}

int holding;                   // number of objects with prop[t]<0

constexpr int BUF_SIZE = 72;
char buffer[BUF_SIZE];
char word1[BUF_SIZE], word2[BUF_SIZE];

location oldoldloc, oldloc, loc, newloc; // recent and future locations

static inline bool here(object t) {
    return toting(t) || place[t] == loc;
}

motion mot;
action verb;
action oldverb;
object obj;
object oldobj;
wordtype command_type;
int turns;
int west_count;
bool was_dark;

int interval = 5;
int tally = 15;     // unseen treasures
int lost_treasures;

bool gave_up;
int limit;

char const *const incantation[] = {"fee", "fie", "foe", "foo", "fum"};
int foobar;

int look_count;

int rx;

int dflag;
int dkill;

constexpr int nd = 5;     // number of dwarves
location dloc[nd + 1] = { // dwarf location
    [0] = chest_loc, [1] = hmk,   [2] = wfiss,
    [3] = y2,        [4] = like3, [5] = complex
};
location odloc[nd + 1];   // old dwarf location
bool dseen[nd + 1];       // dwarf seen

int dtotal;
int attack;
int stick;
location ploc[19];

int knife_loc;

char const *const attack_msg[] = {[0] = "it misses",
                                  [1] = "it gets you",
                                  [2] = "none of them hit you",
                                  [3] = "one of them gets you"
                                 };

int clock1 = 15, clock2 = 30;
bool panic, closed;
bool warned;
int death_count;

constexpr int max_deaths = 3;
char const *const death_wishes[2 * max_deaths] = {
    [0] = "Oh dear, you seem to have gotten yourself killed.  I might be able to\n"
          "help you out, but I've never really done this before.  Do you want me\n"
          "to try to reincarnate you?",
    [1] = "All right.  But don't blame me if something goes wr......\n"
          "                      --- POOF!! ---\n"
          "You are engulfed in a cloud of orange smoke.  Coughing and gasping,\n"
          "you emerge from the smoke and find....",
    [2] = "You clumsy oaf, you've done it again!  I don't know how long I can\n"
          "keep this up.  Do you want me to try reincarnating you again?",
    [3] = "Okay, now where did I put my resurrection kit?....  >POOF!<\n"
          "Everything disappears in a dense cloud of orange smoke.",
    [4] = "Now you've really done it!  I'm out of orange smoke!  You don't expect\n"
          "me to do a decent reincarnation without any orange smoke, do you?",
    [5] = "Okay, if you're so smart, do it yourself!  I'm leaving!"
};

int bonus;

constexpr size_t n_hints = 8;
int hint_count[n_hints];
int hint_thresh[n_hints] = {0, 0, 4, 5, 8, 75, 25, 20};
int hint_cost[n_hints] = {5, 10, 2, 2, 2, 4, 5, 3};
char const *const hint_prompt[n_hints] = {
    [0] = "Welcome to Adventure!!  Would you like instructions?",
    [1] = ("Hmmm, this looks like a clue, which means it'll cost you 10 points to\n"
           "read it.  Should I go ahead and read it anyway?"),
    [2] = "Are you trying to get into the cave?",
    [3] = "Are you trying to catch the bird?",
    [4] = "Are you trying to deal somehow with the snake?",
    [5] = "Do you need help getting out of the maze?",
    [6] = "Are you trying to explore beyond the Plover Room?",
    [7] = "Do you need help getting out of here?"
};

char const *const hint[n_hints] = {
    [0] = "Somewhere nearby is Colossal Cave, where others have found fortunes in\n"
          "treasure and gold, though it is rumored that some who enter are never\n"
          "seen again.  Magic is said to work in the cave.  I will be your eyes\n"
          "and hands.  Direct me with commands of one or two words.  I should\n"
          "warn you that I look at only the first five letters of each word, so\n"
          "you'll have to enter \"NORTHEAST\" as \"NE\" to distinguish it from\n"
          "\"NORTH\".  Should you get stuck, type \"HELP\" for some general hints.\n"
          "For information on how to end your adventure, etc., type \"INFO\".\n"
          "                            -  -  -\n"
          "The first adventure program was developed by Willie Crowther.\n"
          "Most of the features of the current program were added by Don Woods;\n"
          "all of its bugs were added by Don Knuth.",
    [1] = "It says, \"There is something strange about this place, such that one\n"
          "of the words I've always known now has a new effect.\"",
    [2] = "The grate is very solid and has a hardened steel lock.  You cannot\n"
          "enter without a key, and there are no keys in sight.  I would recommend\n"
          "looking elsewhere for the keys.",
    [3] = "Something seems to be frightening the bird just now and you cannot\n"
          "catch it no matter what you try.  Perhaps you might try later.",
    [4] = "You can't kill the snake, or drive it away, or avoid it, or anything\n"
          "like that.  There is a way to get by, but you don't have the necessary\n"
          "resources right now.",
    [5] = "You can make the passages look less alike by dropping things.",
    [6] = "There is a way to explore that region without having to worry about\n"
          "falling into a pit.  None of the objects available is immediately\n"
          "useful for discovering the secret.",
    [7] = "Don't go west."
};
bool hinted[n_hints];

const int CLASS_SCORE[] = {
    [0] = 35,  [1] = 100, [2] = 130, [3] = 200, [4] = 250,
    [5] = 300, [6] = 330, [7] = 349, [8] = 9999
};
char const *const CLASS_MESSAGE[] = {
    [0] = "You are obviously a rank amateur.  Better luck next time.",
    [1] = "Your score qualifies you as a novice class adventurer.",
    [2] = "You have achieved the rating \"Experienced Adventurer\".",
    [3] = "You may now consider yourself a \"Seasoned Adventurer\".",
    [4] = "You have reached \"Junior Master\" status.",
    [5] = "Your score puts you in Master Adventurer Class C.",
    [6] = "Your score puts you in Master Adventurer Class B.",
    [7] = "Your score puts you in Master Adventurer Class A.",
    [8] = "All of Adventuredom gives tribute to you, Adventure Grandmaster!"
};

static inline bool streq(const char *a, const char *b) {
    return strncmp(a, b, 5) == 0;
}

void new_word(const char *word, int meaning) {
    // rolling polynomial hash
    int hash = 0;
    for (const char *p = word; *p; p++)
        hash = *p + hash + hash;
    hash %= HASH_PRIME;

    // linear probing
    while (hash_table[hash].word_type) {
        hash++;
        if (hash == HASH_PRIME)
            hash = 0;
    }
    strcpy(hash_table[hash].text, word);
    hash_table[hash].word_type = current_type;
    hash_table[hash].meaning = meaning;
}

int lookup(char *w) {
    int hash = 0;
    char t = w[5];
    w[5] = '\0';
    for (char *p = w; *p; p++)
        hash = *p + hash + hash;
    hash %= HASH_PRIME;
    w[5] = t;
    if (hash < 0)
        return -1;
    while (hash_table[hash].word_type) {
        if (streq(w, hash_table[hash].text))
            return hash;
        hash++;
        if (hash == HASH_PRIME)
            hash = 0;
    }
    return -1;
}

void drop(object t, location l) {
    if (toting(t))
        holding--;
    place[t] = l;
    if (l < 0)
        holding++;
    else if (l > 0) {
        link[t] = first[l];
        first[l] = t;
    }
}

void carry(object t) {
    location l = place[t];
    if (l >= limbo) {
        place[t] = inhand;
        holding++;
        if (l > limbo) {
            object r, s;
            for (r = 0, s = first[l]; s != t; r = s, s = link[s])
                ;
            if (r == 0)
                first[l] = link[s];
            else
                link[r] = link[s];
        }
    }
}

static inline void move(object t, location l) {
    carry(t);
    drop(t, l);
}

static inline void destroy(object t) {
    move(t, limbo);
}

bool is_at_loc(object t) {
    if (base[t] == NOTHING)
        return place[t] == loc;
    for (object tt = t; base[tt] == t; tt++)
        if (place[tt] == loc)
            return true;
    return false;
}

void new_obj(object t, object b, location l) {
    base[t] = b;
    prop[t] = (is_treasure(t) ? -1 : 0);
    drop(t, l);
}

bool yes(const char *q, const char *y, const char *n) {
    while (true) {
        printf("%s\n** ", q);
        fflush(stdout);
        if (!fgets(buffer, BUF_SIZE, stdin)) {
            printf("Bye!\n");
            // can not goto quit;
            exit(0);
        }
        if (tolower(*buffer) == 'y') {
            if (y)
                printf("%s\n", y);
            return true;
        } else if (tolower(*buffer) == 'n') {
            if (n)
                printf("%s\n", n);
            return false;
        } else
            printf(" Please answer Yes or No.\n");
    }
}

bool listen() {
    char *p, *q;
    while (true) {
        printf("* ");
        fflush(stdout);
        if (!fgets(buffer, BUF_SIZE, stdin)) {
            return false;
        }
        for (p = buffer; isspace(*p); p++)
            ;
        if (*p == 0) {
            printf(" Tell me to do something.\n");
            continue;
        }
        for (q = word1; *p; p++, q++) {
            if (isspace(*p))
                break;
            *q = tolower(*p);
        }
        *q = '\0';
        for (; isspace(*p); p++)
            ;
        if (*p == 0) {
            *word2 = '\0';
            return true;
        }
        for (q = word2; *p; p++, q++) {
            if (isspace(*p))
                break;
            *q = tolower(*p);
        }
        *q = '\0';
        for (; isspace(*p); p++)
            ;
        if (*p == 0)
            return true;
        printf(" Please stick to 1- and 2-word commands.\n");
    }
    return true;
}

int ran(int range) {
    rx = (1021 * rx) & 0xfffff;
    return (range * rx) >> 20;
}

bool pct(int r) {
    return ran(100) < r;
}

bool dwarf() {
    if (dflag < 2)
        return false;
    for (size_t j = 1; j <= nd; j++)
        if (dloc[j] == loc)
            return true;
    return false;
}

void offer(int j) {
    if (j > 1) {
        if (!yes(hint_prompt[j], " I am prepared to give you a hint,", ok))
            return;
        printf(" but it will cost you %d points.  ", hint_cost[j]);
        hinted[j] = yes("Do you want the hint?", hint[j], ok);
    } else
        hinted[j] = yes(hint_prompt[j], hint[j], ok);
    if (hinted[j] && limit > 30)
        limit += 30 * hint_cost[j];
}

int score() {
    int s = 2;
    if (dflag)
        s += 25;
    for (object k = min_treasure; k <= max_obj; k++) {
        if (prop[k] >= 0) {
            s += 2;
            if (place[k] == house && prop[k] == 0)
                s += (k < CHEST ? 10 : k == CHEST ? 12 : 14);
        }
    }
    s += 10 * (max_deaths - death_count);
    if (!gave_up)
        s += 4;
    if (place[MAG] == witt)
        s++;
    if (closing)
        s += 25;
    s += bonus;
    for (size_t j = 0; j < n_hints; j++)
        if (hinted[j])
            s -= hint_cost[j];
    return s;
}

int main() {
    int j, k;
    const char *p;
    instruction *q;
    instruction *qq;
    object t;
    location l, ll;

    rx = (((int)time(NULL)) & 0xfffff) | 1;

    offer(0);
    limit = (hinted[0] ? 1000 : 330);

    current_type = motion_type;
    new_word("north", N);
    new_word("n", N);
    new_word("south", S);
    new_word("s", S);
    new_word("east", E);
    new_word("e", E);
    new_word("west", W);
    new_word("w", W);
    new_word("ne", NE);
    new_word("se", SE);
    new_word("nw", NW);
    new_word("sw", SW);
    new_word("upwar", U);
    new_word("up", U);
    new_word("u", U);
    new_word("above", U);
    new_word("ascen", U);
    new_word("downw", D);
    new_word("down", D);
    new_word("d", D);
    new_word("desce", D);
    new_word("left", L);
    new_word("right", R);
    new_word("inwar", IN);
    new_word("insid", IN);
    new_word("in", IN);
    new_word("out", OUT);
    new_word("outsi", OUT);
    new_word("exit", OUT);
    new_word("leave", OUT);
    new_word("forwa", FORWARD);
    new_word("conti", FORWARD);
    new_word("onwar", FORWARD);
    new_word("back", BACK);
    new_word("retur", BACK);
    new_word("retre", BACK);
    new_word("over", OVER);
    new_word("acros", ACROSS);
    new_word("upstr", UPSTREAM);
    new_word("downs", DOWNSTREAM);
    new_word("enter", ENTER);
    new_word("crawl", CRAWL);
    new_word("jump", JUMP);
    new_word("climb", CLIMB);
    new_word("look", LOOK);
    new_word("exami", LOOK);
    new_word("touch", LOOK);
    new_word("descr", LOOK);
    new_word("cross", CROSS);
    new_word("road", ROAD);
    new_word("hill", ROAD);
    new_word("fores", WOODS);
    new_word("valle", VALLEY);
    new_word("build", HOUSE);
    new_word("house", HOUSE);
    new_word("gully", GULLY);
    new_word("strea", STREAM);
    new_word("depre", DEPRESSION);
    new_word("entra", ENTRANCE);
    new_word("cave", CAVE);
    new_word("rock", ROCK);
    new_word("slab", SLAB);
    new_word("slabr", SLAB);
    new_word("bed", BED);
    new_word("passa", PASSAGE);
    new_word("tunne", PASSAGE);
    new_word("caver", CAVERN);
    new_word("canyo", CANYON);
    new_word("awkwa", AWKWARD);
    new_word("secre", SECRET);
    new_word("bedqu", BEDQUILT);
    new_word("reser", RESERVOIR);
    new_word("giant", GIANT);
    new_word("orien", ORIENTAL);
    new_word("shell", SHELL);
    new_word("barre", BARREN);
    new_word("broke", BROKEN);
    new_word("debri", DEBRIS);
    new_word("view", VIEW);
    new_word("fork", FORK);
    new_word("pit", PIT);
    new_word("slit", SLIT);
    new_word("crack", CRACK);
    new_word("dome", DOME);
    new_word("hole", HOLE);
    new_word("wall", WALL);
    new_word("hall", HALL);
    new_word("room", ROOM);
    new_word("floor", FLOOR);
    new_word("stair", STAIRS);
    new_word("steps", STEPS);
    new_word("cobbl", COBBLES);
    new_word("surfa", SURFACE);
    new_word("dark", DARK);
    new_word("low", LOW);
    new_word("outdo", OUTDOORS);
    new_word("y2", Y2);
    new_word("xyzzy", XYZZY);
    new_word("plugh", PLUGH);
    new_word("plove", PLOVER);
    new_word("main", OFFICE);
    new_word("offic", OFFICE);
    new_word("null", NOWHERE);
    new_word("nowhe", NOWHERE);

    current_type = object_type;
    new_word("key", KEYS);
    new_word("keys", KEYS);
    new_word("lamp", LAMP);
    new_word("lante", LAMP);
    new_word("headl", LAMP);
    new_word("grate", GRATE);
    new_word("cage", CAGE);
    new_word("rod", ROD);
    new_word("bird", BIRD);
    new_word("door", DOOR);
    new_word("pillo", PILLOW);
    new_word("velve", PILLOW);
    new_word("snake", SNAKE);
    new_word("fissu", CRYSTAL);
    new_word("table", TABLET);
    new_word("clam", CLAM);
    new_word("oyste", OYSTER);
    new_word("magaz", MAG);
    new_word("issue", MAG);
    new_word("spelu", MAG);
    new_word("\"spel", MAG);
    new_word("dwarf", DWARF);
    new_word("dwarv", DWARF);
    new_word("knife", KNIFE);
    new_word("knive", KNIFE);
    new_word("food", FOOD);
    new_word("ratio", FOOD);
    new_word("bottl", BOTTLE);
    new_word("jar", BOTTLE);
    new_word("water", WATER);
    new_word("h2o", WATER);
    new_word("oil", OIL);
    new_word("mirro", MIRROR);
    new_word("plant", PLANT);
    new_word("beans", PLANT);
    new_word("stala", STALACTITE);
    new_word("shado", SHADOW);
    new_word("figur", SHADOW);
    new_word("axe", AXE);
    new_word("drawi", ART);
    new_word("pirat", PIRATE);
    new_word("drago", DRAGON);
    new_word("chasm", BRIDGE);
    new_word("troll", TROLL);
    new_word("bear", BEAR);
    new_word("messa", MESSAGE);
    new_word("volca", GEYSER);
    new_word("geyse", GEYSER);
    new_word("vendi", PONY);
    new_word("machi", PONY);
    new_word("batte", BATTERIES);
    new_word("moss", MOSS);
    new_word("carpe", MOSS);
    new_word("gold", GOLD);
    new_word("nugge", GOLD);
    new_word("diamo", DIAMONDS);
    new_word("silve", SILVER);
    new_word("bars", SILVER);
    new_word("jewel", JEWELS);
    new_word("coins", COINS);
    new_word("chest", CHEST);
    new_word("box", CHEST);
    new_word("treas", CHEST);
    new_word("eggs", EGGS);
    new_word("egg", EGGS);
    new_word("nest", EGGS);
    new_word("tride", TRIDENT);
    new_word("ming", VASE);
    new_word("vase", VASE);
    new_word("shard", VASE);
    new_word("potte", VASE);
    new_word("emera", EMERALD);
    new_word("plati", PYRAMID);
    new_word("pyram", PYRAMID);
    new_word("pearl", PEARL);
    new_word("persi", RUG);
    new_word("rug", RUG);
    new_word("spice", SPICES);
    new_word("chain", CHAIN);

    current_type = action_type;
    new_word("take", TAKE);
    new_word("carry", TAKE);
    new_word("keep", TAKE);
    new_word("catch", TAKE);
    new_word("captu", TAKE);
    new_word("steal", TAKE);
    new_word("get", TAKE);
    new_word("tote", TAKE);
    new_word("drop", DROP);
    new_word("relea", DROP);
    new_word("free", DROP);
    new_word("disca", DROP);
    new_word("dump", DROP);
    new_word("open", OPEN);
    new_word("unloc", OPEN);
    new_word("close", CLOSE);
    new_word("lock", CLOSE);
    new_word("light", ON);
    new_word("on", ON);
    new_word("extin", OFF);
    new_word("off", OFF);
    new_word("wave", WAVE);
    new_word("shake", WAVE);
    new_word("swing", WAVE);
    new_word("calm", CALM);
    new_word("placa", CALM);
    new_word("tame", CALM);
    new_word("walk", GO);
    new_word("run", GO);
    new_word("trave", GO);
    new_word("go", GO);
    new_word("proce", GO);
    new_word("explo", GO);
    new_word("goto", GO);
    new_word("follo", GO);
    new_word("turn", GO);
    new_word("nothi", RELAX);
    new_word("pour", POUR);
    new_word("eat", EAT);
    new_word("devou", EAT);
    new_word("drink", DRINK);
    new_word("rub", RUB);
    new_word("throw", TOSS);
    new_word("toss", TOSS);
    new_word("wake", WAKE);
    new_word("distu", WAKE);
    new_word("feed", FEED);
    new_word("fill", FILL);
    new_word("break", BREAK);
    new_word("smash", BREAK);
    new_word("shatt", BREAK);
    new_word("blast", BLAST);
    new_word("deton", BLAST);
    new_word("ignit", BLAST);
    new_word("blowu", BLAST);
    new_word("attac", KILL);
    new_word("kill", KILL);
    new_word("fight", KILL);
    new_word("hit", KILL);
    new_word("strik", KILL);
    new_word("slay", KILL);
    new_word("say", SAY);
    new_word("chant", SAY);
    new_word("sing", SAY);
    new_word("utter", SAY);
    new_word("mumbl", SAY);
    new_word("read", READ);
    new_word("perus", READ);
    new_word("fee", FEEFIE);
    new_word("fie", FEEFIE);
    new_word("foe", FEEFIE);
    new_word("foo", FEEFIE);
    new_word("fum", FEEFIE);
    new_word("brief", BRIEF);
    new_word("find", FIND);
    new_word("where", FIND);
    new_word("inven", INVENTORY);
    new_word("score", SCORE);
    new_word("quit", QUIT);

    current_type = message_type;
    new_word("abra", MAGIC);
    new_word("abrac", MAGIC);
    new_word("opens", MAGIC);
    new_word("sesam", MAGIC);
    new_word("shaza", MAGIC);
    new_word("hocus", MAGIC);
    new_word("pocus", MAGIC);
    new_word("help", HELP);
    new_word("?", HELP);
    new_word("tree", TREE);
    new_word("trees", TREE);
    new_word("dig", DIG);
    new_word("excav", DIG);
    new_word("lost", LOST);
    new_word("mist", MIST);
    new_word("fuck", FUCK);
    new_word("stop", STOP);
    new_word("info", INFO);
    new_word("infor", INFO);
    new_word("swim", SWIM);

    q = travels;
    make_loc( road );
    make_inst(W, 0, hill);
    ditto(U);
    ditto(ROAD);
    make_inst(E, 0, house);
    ditto(IN);
    ditto(HOUSE);
    ditto(ENTER);
    make_inst(S, 0, valley);
    ditto(D);
    ditto(GULLY);
    ditto(STREAM);
    ditto(DOWNSTREAM);
    make_inst(N, 0, forest);
    ditto(WOODS);
    make_inst(DEPRESSION, 0, outside);

    make_loc( hill );
    make_inst(ROAD, 0, road);
    ditto(HOUSE);
    ditto(FORWARD);
    ditto(E);
    ditto(D);
    make_inst(WOODS, 0, forest);
    ditto(N);
    ditto(S);

    make_loc(house );
    make_inst(ENTER, 0, road);
    ditto(OUT);
    ditto(OUTDOORS);
    ditto(W);
    make_inst(XYZZY, 0, debris);
    make_inst(PLUGH, 0, y2);
    make_inst(DOWNSTREAM, 0, sewer);
    ditto(STREAM);

    make_loc( valley );
    make_inst(UPSTREAM, 0, road);
    ditto(HOUSE);
    ditto(N);
    make_inst(WOODS, 0, forest);
    ditto(E);
    ditto(W);
    ditto(U);
    make_inst(DOWNSTREAM, 0, slit);
    ditto(S);
    ditto(D);
    make_inst(DEPRESSION, 0, outside);

    make_loc(forest );
    make_inst(VALLEY, 0, valley);
    ditto(E);
    ditto(D);
    make_inst(WOODS, 50, forest);
    ditto(FORWARD);
    ditto(N);
    make_inst(WOODS, 0, woods);
    make_inst(W, 0, forest);
    ditto(S);

    make_loc(woods);
    make_inst(ROAD, 0, road);
    ditto(N);
    make_inst(VALLEY, 0, valley);
    ditto(E);
    ditto(W);
    ditto(D);
    make_inst(WOODS, 0, forest);
    ditto(S);

    make_loc( slit );
    make_inst(HOUSE, 0, road);
    make_inst(UPSTREAM, 0, valley);
    ditto(N);
    make_inst(WOODS, 0, forest);
    ditto(E);
    ditto(W);
    make_inst(DOWNSTREAM, 0, outside);
    ditto(ROCK);
    ditto(BED);
    ditto(S);
    make_inst(SLIT, 0, sayit);
    ditto(STREAM);
    ditto(D);

    make_loc( outside);
    make_inst(WOODS, 0, forest);
    ditto(E);
    ditto(W);
    ditto(S);
    make_inst(HOUSE, 0, road);
    make_inst(UPSTREAM, 0, slit);
    ditto(GULLY);
    ditto(N);
    make_inst(ENTER, not(GRATE, 0), inside);
    //ditto(ENTER); // Duplicate - typo?
    ditto(IN);
    ditto(D);
    make_inst(ENTER, 0, sayit);

    make_loc( inside);
    make_inst(OUT, not(GRATE, 0), outside);
    ditto(OUT);
    ditto(U);
    make_inst(OUT, 0, sayit);
    make_inst(CRAWL, 0, cobbles);
    ditto(COBBLES);
    ditto(IN);
    ditto(W);
    make_inst(PIT, 0, spit);
    make_inst(DEBRIS, 0, debris);

    make_loc(cobbles);
    make_inst(OUT, 0, inside);
    ditto(SURFACE);
    ditto(NOWHERE);
    ditto(E);
    make_inst(IN, 0, debris);
    ditto(DARK);
    ditto(W);
    ditto(DEBRIS);
    make_inst(PIT, 0, spit);

    make_loc( debris);
    make_inst(DEPRESSION, not(GRATE, 0), outside);
    make_inst(ENTRANCE, 0, inside);
    make_inst(CRAWL, 0, cobbles);
    ditto(COBBLES);
    ditto(PASSAGE);
    ditto(LOW);
    ditto(E);
    make_inst(CANYON, 0, awk);
    ditto(IN);
    ditto(U);
    ditto(W);
    make_inst(XYZZY, 0, house);
    make_inst(PIT, 0, spit);

    make_loc(awk);
    make_inst(DEPRESSION, not(GRATE, 0), outside);
    make_inst(ENTRANCE, 0, inside);
    make_inst(D, 0, debris);
    ditto(E);
    ditto(DEBRIS);
    make_inst(IN, 0, bird);
    ditto(U);
    ditto(W);
    make_inst(PIT, 0, spit);

    make_loc( bird);
    make_inst(DEPRESSION, not(GRATE, 0), outside);
    make_inst(ENTRANCE, 0, inside);
    make_inst(DEBRIS, 0, debris);
    make_inst(CANYON, 0, awk);
    ditto(E);
    make_inst(PASSAGE, 0, spit);
    ditto(PIT);
    ditto(W);

    make_loc(spit);
    make_inst(DEPRESSION, not(GRATE, 0), outside);
    make_inst(ENTRANCE, 0, inside);
    make_inst(DEBRIS, 0, debris);
    make_inst(PASSAGE, 0, bird);
    ditto(E);
    make_inst(D, holds(GOLD), neck);
    ditto(PIT);
    ditto(STEPS);
    make_inst(D, 0, emist);
    make_inst(CRACK, 0, crack);
    ditto(W);

    make_loc( emist);
    make_inst(L, 0, nugget);
    ditto(S);
    make_inst(FORWARD, 0, efiss);
    ditto(HALL);
    ditto(W);
    make_inst(STAIRS, 0, hmk);
    ditto(D);
    ditto(N);
    make_inst(U, holds(GOLD), cant);
    ditto(PIT);
    ditto(STEPS);
    ditto(DOME);
    ditto(PASSAGE);
    ditto(E);
    make_inst(U, 0, spit);
    make_inst(Y2, 0, jumble);

    make_loc(nugget);
    make_inst(HALL, 0, emist);
    ditto(OUT);
    ditto(N);

    make_loc( efiss);
    make_inst(HALL, 0, emist);
    ditto(E);
    make_inst(JUMP, not(CRYSTAL, 0), sayit);
    make_inst(FORWARD, not(CRYSTAL, 1), lose);
    make_inst(OVER, not(CRYSTAL, 1), sayit);
    ditto(ACROSS);
    ditto(W);
    ditto(CROSS);
    make_inst(OVER, 0, wfiss);

    make_loc(wfiss);
    make_inst(JUMP, not(CRYSTAL, 0), sayit);
    make_inst(FORWARD, not(CRYSTAL, 1), lose);
    make_inst(OVER, not(CRYSTAL, 1), sayit);
    ditto(ACROSS);
    ditto(E);
    ditto(CROSS);
    make_inst(OVER, 0, efiss);
    make_inst(N, 0, thru);
    make_inst(W, 0, wmist);

    make_loc(wmist);
    make_inst(S, 0, like1);
    ditto(U);
    ditto(PASSAGE);
    ditto(CLIMB);
    make_inst(E, 0, wfiss);
    make_inst(N, 0, duck);
    make_inst(W, 0, elong);
    ditto(CRAWL);

    make_loc(like1);
    make_inst(U, 0, wmist);
    make_inst(N, 0, like1);
    make_inst(E, 0, like2);
    make_inst(S, 0, like4);
    make_inst(W, 0, like11);

    make_loc(like2);
    make_inst(W, 0, like1);
    make_inst(S, 0, like3);
    make_inst(E, 0, like4);

    make_loc(like3);
    make_inst(E, 0, like2);
    make_inst(D, 0, dead5);
    make_inst(S, 0, like6);
    make_inst(N, 0, dead9);

    make_loc(like4);
    make_inst(W, 0, like1);
    make_inst(N, 0, like2);
    make_inst(E, 0, dead3);
    make_inst(S, 0, dead4);
    make_inst(U, 0, like14);
    ditto(D);

    make_loc(like5);
    make_inst(E, 0, like6);
    make_inst(W, 0, like7);

    make_loc(like6);
    make_inst(E, 0, like3);
    make_inst(W, 0, like5);
    make_inst(D, 0, like7);
    make_inst(S, 0, like8);

    make_loc(like7);
    make_inst(W, 0, like5);
    make_inst(U, 0, like6);
    make_inst(E, 0, like8);
    make_inst(S, 0, like9);

    make_loc(like8);
    make_inst(W, 0, like6);
    make_inst(E, 0, like7);
    make_inst(S, 0, like8);
    make_inst(U, 0, like9);
    make_inst(N, 0, like10);
    make_inst(D, 0, dead11);

    make_loc(like9);
    make_inst(W, 0, like7);
    make_inst(N, 0, like8);
    make_inst(S, 0, dead6);

    make_loc(like10);
    make_inst(W, 0, like8);
    make_inst(N, 0, like10);
    make_inst(D, 0, dead7);
    make_inst(E, 0, brink);

    make_loc(like11);
    make_inst(N, 0, like1);
    make_inst(W, 0, like11);
    ditto(S);
    make_inst(E, 0, dead1);

    make_loc(like12);
    make_inst(S, 0, brink);
    make_inst(E, 0, like13);
    make_inst(W, 0, dead10);

    make_loc(like13);
    make_inst(N, 0, brink);
    make_inst(W, 0, like12);
    make_inst(NW, 0, dead2);

    make_loc(like14);
    make_inst(U, 0, like4);
    ditto(D);

    make_loc( brink);
    make_inst(D, 0, bird);
    ditto(CLIMB);
    make_inst(W, 0, like10);
    make_inst(S, 0, dead8);
    make_inst(N, 0, like12);
    make_inst(E, 0, like13);

    make_loc( elong);
    make_inst(E, 0, wmist);
    ditto(U);
    ditto(CRAWL);
    make_inst(W, 0, wlong);
    make_inst(N, 0, cross);
    ditto(D);
    ditto(HOLE);

    make_loc( wlong);
    make_inst(E, 0, elong);
    make_inst(N, 0, cross);
    make_inst(S, 100, diff0);

    twist(diff0, diff9, diff1, diff7, diff8, diff3, diff4, diff6, diff2, diff5, wlong);
    twist(diff1, diff8, diff9, diff10, diff0, diff5, diff2, diff3, diff4, diff6, diff7);
    twist(diff2, diff3, diff4, diff8, diff5, diff7, diff10, diff0, diff6, diff1, diff9);
    twist(diff3, diff7, diff10, diff6, diff2, diff4, diff9, diff8, diff5, diff0, diff1);
    twist(diff4, diff1, diff7, diff5, diff9, diff0, diff3, diff2, diff10, diff8, diff6);
    twist(diff5, diff0, diff3, diff4, diff6, diff8, diff1, diff9, diff7, diff10, diff2);
    twist(diff6, diff10, diff5, diff0, diff1, diff9, diff8, diff7, diff3, diff2, diff4);
    twist(diff7, diff6, diff2, diff9, diff10, diff1, diff0, diff5, diff8, diff4, diff3);
    twist(diff8, diff5, diff6, diff1, diff4, diff2, diff7, diff10, diff9, diff3, diff0);
    twist(diff9, diff4, diff8, diff2, diff3, diff10, diff6, diff1, diff0, diff7, diff5);
    twist(diff10, diff2, pony, diff3, diff7, diff6, diff5, diff4, diff1, diff9, diff8);

    make_loc(pony);
    make_inst(N, 0, diff10);
    ditto(OUT);

    make_loc(cross);
    make_inst(W, 0, elong);
    make_inst(N, 0, dead0);
    make_inst(E, 0, west);
    make_inst(S, 0, wlong);

    make_loc( hmk);
    make_inst(STAIRS, 0, emist);
    ditto(U);
    ditto(E);
    make_inst(N, not(SNAKE, 0), ns);
    ditto(L);
    make_inst(S, not(SNAKE, 0), south);
    ditto(R);
    make_inst(W, not(SNAKE, 0), west);
    ditto(FORWARD);
    make_inst(N, 0, snaked);
    make_inst(SW, 35, secret);
    make_inst(SW, sees(SNAKE), snaked);
    make_inst(SECRET, 0, secret);

    make_loc(west);
    make_inst(HALL, 0, hmk);
    ditto(OUT);
    ditto(E);
    make_inst(W, 0, cross);
    ditto(U);

    make_loc(south);
    make_inst(HALL, 0, hmk);
    ditto(OUT);
    ditto(N);

    make_loc(ns);
    make_inst(HALL, 0, hmk);
    ditto(OUT);
    ditto(S);
    make_inst(N, 0, y2);
    ditto(Y2);
    make_inst(D, 0, dirty);
    ditto(HOLE);

    make_loc(y2);
    make_inst(PLUGH, 0, house);
    make_inst(S, 0, ns);
    make_inst(E, 0, jumble);
    ditto(WALL);
    ditto(BROKEN);
    make_inst(W, 0, windoe);
    make_inst(PLOVER, holds(EMERALD), pdrop);
    make_inst(PLOVER, 0, proom);

    make_loc(jumble);
    make_inst(D, 0, y2);
    ditto(Y2);
    make_inst(U, 0, emist);

    make_loc(windoe);
    make_inst(E, 0, y2);
    ditto(Y2);
    make_inst(JUMP, 0, neck);

    make_loc(dirty);
    make_inst(E, 0, clean);
    ditto(CRAWL);
    make_inst(U, 0, ns);
    ditto(HOLE);
    make_inst(W, 0, dusty);
    make_inst(BEDQUILT, 0, bedquilt);

    make_loc( clean);
    make_inst(W, 0, dirty);
    ditto(CRAWL);
    make_inst(D, 0, wet);
    ditto(PIT);
    ditto(CLIMB);

    make_loc(wet);
    make_inst(CLIMB, 0, clean);
    ditto(U);
    ditto(OUT);
    make_inst(SLIT, 0, sayit);
    ditto(STREAM);
    ditto(D);
    ditto(UPSTREAM);
    ditto(DOWNSTREAM);

    make_loc(dusty);
    make_inst(E, 0, dirty);
    ditto(PASSAGE);
    make_inst(D, 0, complex);
    ditto(HOLE);
    ditto(FLOOR);
    make_inst(BEDQUILT, 0, bedquilt);

    make_loc(complex);
    make_inst(U, 0, dusty);
    ditto(CLIMB);
    ditto(ROOM);
    make_inst(W, 0, bedquilt);
    ditto(BEDQUILT);
    make_inst(N, 0, shell);
    ditto(SHELL);
    make_inst(E, 0, ante);

    make_loc(shell);
    make_inst(U, 0, arch);
    ditto(HALL);
    make_inst(D, 0, ragged);
    make_inst(S, holds(CLAM), sayit);
    make_inst(S, holds(OYSTER), sayit);
    make_inst(S, 0, complex);

    make_loc(arch);
    make_inst(D, 0, shell);
    ditto(SHELL);
    ditto(OUT);

    make_loc(ragged);
    make_inst(U, 0, shell);
    ditto(SHELL);
    make_inst(D, 0, sac);

    make_loc(sac);
    make_inst(U, 0, ragged);
    ditto(OUT);
    make_inst(SHELL, 0, shell);

    make_loc(ante);
    make_inst(U, 0, complex);
    make_inst(W, 0, bedquilt);
    make_inst(E, 0, witt);

    make_loc(witt);
    make_inst(E, 95, sayit);
    ditto(N);
    ditto(S);
    ditto(NE);
    ditto(SE);
    ditto(SW);
    ditto(NW);
    ditto(U);
    ditto(D);
    make_inst(E, 0, ante);
    make_inst(W, 0, sayit);

    make_loc(bedquilt);
    make_inst(E, 0, complex);
    make_inst(W, 0, cheese);
    make_inst(S, 80, sayit);
    make_inst(SLAB, 0, slab);
    make_inst(U, 80, sayit);
    make_inst(U, 50, abovep);
    make_inst(U, 0, dusty);
    make_inst(N, 60, sayit);
    make_inst(N, 75, low);
    make_inst(N, 0, sjunc);
    make_inst(D, 80, sayit);
    make_inst(D, 0, ante);

    make_loc(cheese);
    make_inst(NE, 0, bedquilt);
    make_inst(W, 0, e2pit);
    make_inst(S, 80, sayit);
    make_inst(CANYON, 0, tall);
    make_inst(E, 0, soft);
    make_inst(NW, 50, sayit);
    make_inst(ORIENTAL, 0, oriental);

    make_loc( soft);
    make_inst(W, 0, cheese);
    ditto(OUT);

    make_loc(e2pit);
    make_inst(E, 0, cheese);
    make_inst(W, 0, w2pit);
    ditto(ACROSS);
    make_inst(D, 0, epit);
    ditto(PIT);

    make_loc(w2pit);
    make_inst(E, 0, e2pit);
    ditto(ACROSS);
    make_inst(W, 0, slab);
    ditto(SLAB);
    make_inst(D, 0, wpit);
    ditto(PIT);
    make_inst(HOLE, 0, sayit);

    make_loc(epit);
    make_inst(U, 0, e2pit);
    ditto(OUT);

    make_loc(wpit);
    make_inst(U, 0, w2pit);
    ditto(OUT);
    make_inst(CLIMB, not(PLANT, 4), check);
    make_inst(CLIMB, 0, climb);

    make_loc(narrow);
    make_inst(D, 0, wpit);
    ditto(CLIMB);
    ditto(E);
    make_inst(JUMP, 0, neck);
    make_inst(W, 0, giant);
    ditto(GIANT);

    make_loc(giant);
    make_inst(S, 0, narrow);
    make_inst(E, 0, block);
    make_inst(N, 0, immense);

    make_loc(block);
    make_inst(S, 0, giant);
    ditto(GIANT);
    ditto(OUT);

    make_loc(immense);
    make_inst(S, 0, giant);
    ditto(GIANT);
    ditto(PASSAGE);
    make_inst(N, not(DOOR, 0), falls);
    ditto(ENTER);
    ditto(CAVERN);
    make_inst(N, 0, sayit);

    make_loc(falls);
    make_inst(S, 0, immense);
    ditto(OUT);
    make_inst(GIANT, 0, giant);
    make_inst(W, 0, steep);

    make_loc( steep);
    make_inst(N, 0, falls);
    ditto(CAVERN);
    ditto(PASSAGE);
    make_inst(D, 0, low);
    ditto(CLIMB);

    make_loc(abovep);
    make_inst(N, 0, sjunc);
    make_inst(D, 0, bedquilt);
    ditto(PASSAGE);
    make_inst(S, 0, tite);

    make_loc( sjunc);
    make_inst(SE, 0, bedquilt);
    make_inst(S, 0, abovep);
    make_inst(N, 0, window);

    make_loc( tite);
    make_inst(N, 0, abovep);
    make_inst(D, 40, like6);
    ditto(JUMP);
    ditto(CLIMB);
    make_inst(D, 50, like9);
    make_inst(D, 0, like4);

    make_loc(low);
    make_inst(BEDQUILT, 0, bedquilt);
    make_inst(SW, 0, scorr);
    make_inst(N, 0, crawl);
    make_inst(SE, 0, oriental);
    ditto(ORIENTAL);

    make_loc(crawl);
    make_inst(S, 0, low);
    ditto(CRAWL);
    ditto(OUT);

    make_loc( window);
    make_inst(W, 0, sjunc);
    make_inst(JUMP, 0, neck);

    make_loc( oriental);
    make_inst(SE, 0, cheese);
    make_inst(W, 0, low);
    ditto(CRAWL);
    make_inst(U, 0, misty);
    ditto(N);
    ditto(CAVERN);

    make_loc( misty);
    make_inst(S, 0, oriental);
    ditto(ORIENTAL);
    make_inst(W, 0, alcove);

    make_loc( alcove);
    make_inst(NW, 0, misty);
    ditto(CAVERN);
    make_inst(E, 0, ppass);
    ditto(PASSAGE);
    make_inst(E, 0, proom);

    make_loc( proom);
    make_inst(W, 0, ppass);
    ditto(PASSAGE);
    ditto(OUT);
    make_inst(W, 0, alcove);
    make_inst(PLOVER, holds(EMERALD), pdrop);
    make_inst(PLOVER, 0, y2);
    make_inst(NE, 0, droom);
    ditto(DARK);

    make_loc( droom);
    make_inst(S, 0, proom);
    ditto(PLOVER);
    ditto(OUT);

    make_loc( slab);
    make_inst(S, 0, w2pit);
    make_inst(U, 0, abover);
    ditto(CLIMB);
    make_inst(N, 0, bedquilt);

    make_loc(abover);
    make_inst(D, 0, slab);
    ditto(SLAB);
    make_inst(S, not(DRAGON, 0), scan2);
    make_inst(S, 0, scan1);
    make_inst(N, 0, mirror);
    make_inst(RESERVOIR, 0, res);

    make_loc( mirror);
    make_inst(S, 0, abover);
    make_inst(N, 0, res);
    ditto(RESERVOIR);

    make_loc( res);
    make_inst(S, 0, mirror);
    ditto(OUT);

    make_loc(scan1);
    make_inst(N, 0, abover);
    ditto(OUT);
    make_inst(E, 0, sayit);
    ditto(FORWARD);

    make_loc(scan2);
    make_inst(N, 0, abover);
    make_inst(E, 0, secret);

    make_loc(scan3);
    make_inst(E, 0, secret);
    ditto(OUT);
    make_inst(N, 0, sayit);
    ditto(FORWARD);

    make_loc( secret);
    make_inst(E, 0, hmk);
    make_inst(W, not(DRAGON, 0), scan2);
    make_inst(W, 0, scan3);
    make_inst(D, 0, wide);

    make_loc(wide);
    make_inst(S, 0, tight);
    make_inst(N, 0, tall);

    make_loc(tight);
    make_inst(N, 0, wide);

    make_loc( tall);
    make_inst(E, 0, wide);
    make_inst(W, 0, boulders);
    make_inst(N, 0, cheese);
    ditto(CRAWL);

    make_loc(boulders);
    make_inst(S, 0, tall);

    make_loc(scorr);
    make_inst(D, 0, low);
    make_inst(U, 0, swside);

    make_loc( swside);
    make_inst(SW, 0, scorr);
    make_inst(OVER, sees(TROLL), sayit);
    ditto(ACROSS);
    ditto(CROSS);
    ditto(NE);
    make_inst(OVER, not(BRIDGE, 0), sayit);
    make_inst(OVER, 0, troll);
    make_inst(JUMP, not(BRIDGE, 0), lose);
    make_inst(JUMP, 0, sayit);

    make_loc(dead0);
    make_inst(S, 0, cross);
    ditto(OUT);

    make_loc(dead1);
    make_inst(W, 0, like11);
    ditto(OUT);

    make_loc(dead2);
    make_inst(SE, 0, like13);

    make_loc(dead3);
    make_inst(W, 0, like4);
    ditto(OUT);

    make_loc(dead4);
    make_inst(E, 0, like4);
    ditto(OUT);

    make_loc(dead5);
    make_inst(U, 0, like3);
    ditto(OUT);

    make_loc(dead6);
    make_inst(W, 0, like9);
    ditto(OUT);

    make_loc(dead7);
    make_inst(U, 0, like10);
    ditto(OUT);

    make_loc(dead8);
    make_inst(E, 0, brink);
    ditto(OUT);

    make_loc(dead9);
    make_inst(S, 0, like3);
    ditto(OUT);

    make_loc(dead10);
    make_inst(E, 0, like12);
    ditto(OUT);

    make_loc(dead11);
    make_inst(U, 0, like8);
    ditto(OUT);

    make_loc( neside);
    make_inst(NE, 0, corr);
    make_inst(OVER, sees(TROLL), sayit - 1);
    ditto(ACROSS);
    ditto(CROSS);
    ditto(SW);
    make_inst(OVER, 0, troll);
    make_inst(JUMP, 0, sayit);
    make_inst(FORK, 0, fork);
    make_inst(VIEW, 0, view);
    make_inst(BARREN, 0, fbarr);

    make_loc( corr);
    make_inst(W, 0, neside);
    make_inst(E, 0, fork);
    ditto(FORK);
    make_inst(VIEW, 0, view);
    make_inst(BARREN, 0, fbarr);

    make_loc( fork);
    make_inst(W, 0, corr);
    make_inst(NE, 0, warm);
    ditto(L);
    make_inst(SE, 0, lime);
    ditto(R);
    ditto(D);
    make_inst(VIEW, 0, view);
    make_inst(BARREN, 0, fbarr);

    make_loc( warm);
    make_inst(S, 0, fork);
    ditto(FORK);
    make_inst(N, 0, view);
    ditto(VIEW);
    make_inst(E, 0, chamber);
    ditto(CRAWL);

    make_loc(view);
    make_inst(S, 0, warm);
    ditto(PASSAGE);
    ditto(OUT);
    make_inst(FORK, 0, fork);
    make_inst(D, 0, sayit);
    ditto(JUMP);

    make_loc( chamber);
    make_inst(W, 0, warm);
    ditto(OUT);
    ditto(CRAWL);
    make_inst(FORK, 0, fork);
    make_inst(VIEW, 0, view);

    make_loc( lime);
    make_inst(N, 0, fork);
    ditto(U);
    ditto(FORK);
    make_inst(S, 0, fbarr);
    ditto(D);
    ditto(BARREN);
    make_inst(VIEW, 0, view);

    make_loc(fbarr);
    make_inst(W, 0, lime);
    ditto(U);
    make_inst(FORK, 0, fork);
    make_inst(E, 0, barr);
    ditto(IN);
    ditto(BARREN);
    ditto(ENTER);
    make_inst(VIEW, 0, view);

    make_loc( barr);
    make_inst(W, 0, fbarr);
    ditto(OUT);
    make_inst(FORK, 0, fork);
    make_inst(VIEW, 0, view);

    make_loc(neend);
    make_inst(SW, 0, swend);

    make_loc( swend);
    make_inst(NE, 0, neend);
    make_inst(D, 0, sayit);

    make_loc(crack);
    make_inst(FORCE, 0, spit);

    make_loc(neck);
    make_inst(FORCE, 0, limbo);

    make_loc(lose);
    make_inst(FORCE, 0, limbo);

    make_loc(cant);
    make_inst(FORCE, 0, emist);

    make_loc(climb);
    make_inst(FORCE, 0, narrow);

    make_loc(check);
    make_inst(FORCE, not(PLANT, 2), upnout);
    make_inst(FORCE, 0, didit);

    make_loc(snaked);
    make_inst(FORCE, 0, hmk);

    make_loc(thru);
    make_inst(FORCE, 0, wmist);

    make_loc(duck);
    make_inst(FORCE, 0, wfiss);

    make_loc( sewer);
    make_inst(FORCE, 0, house);

    make_loc(upnout);
    make_inst(FORCE, 0, wpit);

    make_loc(didit);
    make_inst(FORCE, 0, w2pit);

    start[ppass] = q;
    if (q > &travels[travel_size]) {
        printf("Oops, I'm broken!\n");
        exit(-1);
    }
    //printf("travels %ld", q - &travels[0]);

    new_obj(RUG_, RUG, scan3);
    new_obj(RUG, RUG, scan1);
    new_obj(TROLL2_, TROLL2, limbo);
    new_obj(TROLL2, TROLL2, limbo);
    new_obj(TROLL_, TROLL, neside);
    new_obj(TROLL, TROLL, swside);
    new_obj(BRIDGE_, BRIDGE, neside);
    new_obj(BRIDGE, BRIDGE, swside);
    new_obj(DRAGON_, DRAGON, scan3);
    new_obj(DRAGON, DRAGON, scan1);
    new_obj(SHADOW_, SHADOW, window);
    new_obj(SHADOW, SHADOW, windoe);
    new_obj(PLANT2_, PLANT2, e2pit);
    new_obj(PLANT2, PLANT2, w2pit);
    new_obj(CRYSTAL_, CRYSTAL, wfiss);
    new_obj(CRYSTAL, CRYSTAL, efiss);
    new_obj(TREADS_, TREADS, emist);
    new_obj(TREADS, TREADS, spit);
    new_obj(GRATE_, GRATE, inside);
    new_obj(GRATE, GRATE, outside);
    new_obj(MIRROR_, MIRROR, limbo);
    new_obj(CHAIN, CHAIN, barr);
    new_obj(SPICES, 0, chamber);
    new_obj(PEARL, 0, limbo);
    new_obj(PYRAMID, 0, droom);
    new_obj(EMERALD, 0, proom);
    new_obj(VASE, 0, oriental);
    new_obj(TRIDENT, 0, falls);
    new_obj(EGGS, 0, giant);
    new_obj(CHEST, 0, limbo);
    new_obj(COINS, 0, west);
    new_obj(JEWELS, 0, south);
    new_obj(SILVER, 0, ns);
    new_obj(DIAMONDS, 0, wfiss);
    new_obj(GOLD, 0, nugget);
    new_obj(MOSS, MOSS, soft);
    new_obj(BATTERIES, 0, limbo);
    new_obj(PONY, PONY, pony);
    new_obj(GEYSER, GEYSER, view);
    new_obj(MESSAGE, MESSAGE, limbo);
    new_obj(BEAR, BEAR, barr);
    new_obj(PIRATE, PIRATE, limbo);
    new_obj(ART, ART, oriental);
    new_obj(AXE, 0, limbo);
    new_obj(STALACTITE, STALACTITE, tite);
    new_obj(PLANT, PLANT, wpit);
    new_obj(MIRROR, MIRROR, mirror);
    new_obj(OIL, 0, limbo);
    new_obj(WATER, 0, limbo);
    new_obj(BOTTLE, 0, house);
    new_obj(FOOD, 0, house);
    new_obj(KNIFE, 0, limbo);
    new_obj(DWARF, DWARF, limbo);
    new_obj(MAG, 0, ante);
    new_obj(OYSTER, 0, limbo);
    new_obj(CLAM, 0, shell);
    new_obj(TABLET, TABLET, droom);
    new_obj(SNAKE, SNAKE, hmk);
    new_obj(PILLOW, 0, soft);
    new_obj(DOOR, DOOR, immense);
    new_obj(BIRD, 0, bird);
    new_obj(ROD2, 0, limbo);
    new_obj(ROD, 0, debris);
    new_obj(CAGE, 0, cobbles);
    new_obj(LAMP, 0, house);
    new_obj(KEYS, 0, house);

    oldoldloc = oldloc = loc = newloc = road;

    while (true) {
        if (closing && newloc < min_in_cave && newloc != limbo) {
            {
                if (!panic)
                    clock2 = 15, panic = true;
                printf("A mysterious recorded voice groans into life and announces:\n"
                       "\"This exit is closed.  Please leave via main office.\"\n");
            }
            newloc = loc;
        } else if (newloc != loc)
            if (loc <= max_pirate_loc) {
                for (size_t j = 1; j <= nd; j++)
                    if (odloc[j] == newloc && dseen[j]) {
                        printf("A little dwarf with a big knife blocks your way.\n");
                        newloc = loc;
                        break;
                    }
            }

        loc = newloc;
        if (loc <= max_pirate_loc && loc != limbo) {
            if (dflag == 0) {
                if (loc >= min_lower_loc)
                    dflag = 1;
            } else if (dflag == 1) {
                if (loc >= min_lower_loc && pct(5)) {
                    dflag = 2;
                    for (size_t j = 0; j < 2; j++)
                        if (pct(50))
                            dloc[1 + ran(nd)] = limbo;
                    for (size_t j = 1; j <= nd; j++) {
                        if (dloc[j] == loc)
                            dloc[j] = nugget;
                        odloc[j] = dloc[j];
                    }
                    printf(
                        "A little dwarf just walked around a corner, saw you, threw a little\n"
                        "axe at you, cursed, and ran away.  (The axe missed.)\n");
                    drop(AXE, loc);
                }
            } else {
                dtotal = attack = stick = 0;
                for (size_t j = 0; j <= nd; j++)
                    if (dloc[j] != limbo) {
                        int i;
                        for (i = 0, q = start[dloc[j]]; q < start[dloc[j] + 1]; q++) {
                            newloc = q->dest;
                            if (newloc >= min_lower_loc && newloc != odloc[j] &&
                                    newloc != dloc[j] && (i == 0 || newloc != ploc[i - 1]) &&
                                    i < 19 && q->cond != 100 &&
                                    newloc <= (j == 0 ? max_pirate_loc : min_forced_loc - 1))
                                ploc[i++] = newloc;
                        }
                        if (i == 0)
                            i = 1, ploc[0] = odloc[j];
                        odloc[j] = dloc[j];
                        dloc[j] = ploc[ran(i)];
                        dseen[j] = (dloc[j] == loc || odloc[j] == loc ||
                                    (dseen[j] && loc >= min_lower_loc));
                        if (dseen[j]) {
                            dloc[j] = loc;
                            if (j == 0) {
                                if (loc != max_pirate_loc && prop[CHEST] < 0) {
                                    k = 0;
                                    for (int i = min_treasure; i <= max_obj; i++) {
                                        if (!too_easy(i) & toting(i)) {
                                            k = -1;
                                            break;
                                        }
                                        if (here(i))
                                            k = 1;
                                    }
                                    if (k < 0) {
                                        printf("Out from the shadows behind you pounces a bearded pirate!  "
                                               "\"Har, har,\"\n"
                                               "he chortles, \"I'll just take all this booty and hide it away with me\n"
                                               "chest deep in the maze!\"  He snatches your treasure and vanishes into\n"
                                               "the gloom.\n");
                                        for (int i = min_treasure; i <= max_obj; i++)
                                            if (!too_easy(i)) {
                                                if (base[i] == NOTHING && place[i] == loc)
                                                    carry(i);
                                                if (toting(i))
                                                    drop(i, chest_loc);
                                            }

                                        if (pirate_not_spotted) {
move_chest:
                                            move(CHEST, chest_loc);
                                            move(MESSAGE, message_loc);
                                        }
                                        dloc[0] = odloc[0] = chest_loc;
                                        dseen[0] = false;
                                    } else if (tally == lost_treasures + 1 && k == 0 &&
                                               pirate_not_spotted && prop[LAMP] && here(LAMP)) {
                                        printf(
                                            "There are faint rustling noises from the darkness behind you.  As you\n\
turn toward them, the beam of your lamp falls across a bearded pirate.\n\
He is carrying a large chest.  \"Shiver me timbers!\" he cries, \"I've\n\
been spotted!  I'd best hie meself off to the maze to hide me chest!\"\n\
With that, he vanishes into the gloom.\n");
                                        goto move_chest;
                                    } else if (odloc[0] != dloc[0] && pct(20))
                                        printf("There are faint rustling noises from the darkness "
                                               "behind you.\n");
                                }
                            } else {
                                dtotal++;
                                if (odloc[j] == dloc[j]) {
                                    attack++;
                                    if (knife_loc >= 0)
                                        knife_loc = loc;
                                    if (ran(1000) < 95 * (dflag - 2))
                                        stick++;
                                }
                            }
                        }
                    }
                if (dtotal) {
                    if (dtotal == 1)
                        printf("There is a threatening little dwarf");
                    else
                        printf("There are %d threatening little dwarves", dtotal);
                    printf(" in the room with you!\n");
                    if (attack) {
                        if (dflag == 2)
                            dflag = 3;
                        if (attack == 1)
                            k = 0, printf("One sharp nasty knife is thrown");
                        else
                            k = 2, printf(" %d of them throw knives", attack);
                        printf(" at you --- ");
                        if (stick <= 1)
                            printf("%s!\n", attack_msg[k + stick]);
                        else
                            printf("%d of them get you!\n", stick);
                        if (stick) {
                            oldoldloc = loc;
                            goto death;
                        }
                    }
                }
            }
        }

commence:
        if (loc == limbo)
            goto death;
        if (dark && !forced_move(loc)) {
            if (was_dark && pct(35))
                goto pitch_dark;
            p = pitch_dark_msg;
        } else if (get_description_short(loc) == nullptr || visits[loc] % interval == 0)
            p = get_description_long(loc);
        else
            p = get_description_short(loc);
        if (toting(BEAR))
            printf("You are being followed by a very large, tame bear.\n");
        if (p)
            printf("\n%s\n", p);
        if (forced_move(loc))
            goto try_move;
        if (loc == y2 && pct(25) && !closing)
            printf("A hollow voice says \"PLUGH\".\n");

        if (!dark) {
            object tt;
            visits[loc]++;
            for (t = first[loc]; t; t = link[t]) {
                tt = (base[t] ? base[t] : t);
                if (prop[tt] < 0) {
                    if (closed)
                        continue;
                    prop[tt] = (tt == RUG || tt == CHAIN);
                    tally--;
                    if (tally == lost_treasures && tally > 0 && limit > 35)
                        limit = 35;
                }
                if (tt == TREADS && toting(GOLD))
                    continue;
                const char *p = get_note(tt, prop[tt] + (tt == TREADS && loc == emist));
                if (p)
                    printf("%s\n", p);
            }
        }
        while (true) {
            verb = oldverb = ABSTAIN;
            oldobj = obj;
            obj = NOTHING;
cycle:
            for (j = 2, k = cave_hint; j <= 7; j++, k += k)
                if (!hinted[j]) {
                    if ((flags[loc] & k) == 0)
                        hint_count[j] = 0;
                    else if (++hint_count[j] >= hint_thresh[j]) {
                        switch (j) {
                        case 2:
                            if (prop[GRATE] == 0 && !here(KEYS))
                                break;
                            else
                                goto bypass;
                        case 3:
                            if (here(BIRD) && oldobj == BIRD && toting(ROD))
                                break;
                            else
                                continue;
                        case 4:
                            if (here(SNAKE) && !here(BIRD))
                                break;
                            else
                                goto bypass;
                        case 5:
                            if (first[loc] == 0 && first[oldloc] == 0 &&
                                    first[oldoldloc] == 0 && holding > 1)
                                break;
                            else
                                goto bypass;
                        case 6:
                            if (prop[EMERALD] != -1 && prop[PYRAMID] == -1)
                                break;
                            else
                                goto bypass;
                        case 7:
                            break;
                        }
                        offer(j);
bypass:
                        hint_count[j] = 0;
                    }
                }

            was_dark = dark;
            k = ran(0);

            if (knife_loc > limbo && knife_loc != loc)
                knife_loc = limbo;

            if (closed) {
                if (prop[OYSTER] < 0 && toting(OYSTER)) {
                    printf("%s\n", get_note(OYSTER, 1));
                }
                for (size_t j = 1; j <= max_obj; j++)
                    if (toting(j) && prop[j] < 0)
                        prop[j] = -1 - prop[j];
            }

            if (!listen()) {
                goto quit;
            };
pre_parse:
            turns++;
            if (verb == SAY) {
                if (*word2)
                    verb = ABSTAIN;
                else
                    goto transitive;
            }

            if (foobar > 0)
                foobar = -foobar;
            else
                foobar = 0;

            if (tally == 0 && loc >= min_lower_loc && loc != y2)
                clock1--;
            if (clock1 == 0) {
                printf("A sepulchral voice, reverberating through the cave, says, \"Cave\n"
                       "closing soon.  All adventurers exit immediately through main office.\"\n");
                clock1 = -1;
                prop[GRATE] = 0;
                prop[CRYSTAL] = 0;
                for (j = 0; j <= nd; j++)
                    dseen[j] = 0, dloc[j] = limbo;
                destroy(TROLL);
                destroy(TROLL_);
                move(TROLL2, swside);
                move(TROLL2_, neside);
                move(BRIDGE, swside);
                move(BRIDGE_, neside);
                if (prop[BEAR] != 3)
                    destroy(BEAR);
                prop[CHAIN] = 0;
                base[CHAIN] = NOTHING;
                prop[AXE] = 0;
                base[AXE] = NOTHING;
            } else {
                if (clock1 < 0)
                    clock2--;
                if (clock2 == 0) {
                    printf("The sepulchral voice intones, "
                           "\"The cave is now closed.\"  As the echoes\n"
                           "fade, there is a blinding flash of light (and a small puff of orange\n"
                           "smoke). . . .    Then your eyes refocus; you look around and find...\n");
                    move(BOTTLE, neend);
                    prop[BOTTLE] = -2;
                    move(PLANT, neend);
                    prop[PLANT] = -1;
                    move(OYSTER, neend);
                    prop[OYSTER] = -1;
                    move(LAMP, neend);
                    prop[LAMP] = -1;
                    move(ROD, neend);
                    prop[ROD] = -1;
                    move(DWARF, neend);
                    prop[DWARF] = -1;
                    move(MIRROR, neend);
                    prop[MIRROR] = -1;
                    loc = oldloc = neend;
                    move(GRATE, swend);
                    move(SNAKE, swend);
                    prop[SNAKE] = -2;
                    move(BIRD, swend);
                    prop[BIRD] = -2;
                    move(CAGE, swend);
                    prop[CAGE] = -1;
                    move(ROD2, swend);
                    prop[ROD2] = -1;
                    move(PILLOW, swend);
                    prop[PILLOW] = -1;
                    move(MIRROR_, swend);
                    place[WATER] = limbo;
                    place[OIL] = limbo;
                    for (size_t j = 1; j <= max_obj; j++)
                        if (toting(j))
                            destroy(j);
                    closed = true;
                    bonus = 10;
                    stay_put;
                } else {
                    if (prop[LAMP] == 1)
                        limit--;
                    if (limit <= 30 && here(BATTERIES) && prop[BATTERIES] == 0 &&
                            here(LAMP)) {
                        printf("Your lamp is getting dim.  I'm taking the liberty of replacing\n"
                               "the batteries.\n");
                        prop[BATTERIES] = 1;
                        if (toting(BATTERIES))
                            drop(BATTERIES, loc);
                        limit = 2500;
                    }

                    else if (limit == 0) {
                        limit = -1;
                        prop[LAMP] = 0;
                        if (here(LAMP))
                            printf("Your lamp has run out of power.");
                    } else if (limit < 0 && loc < min_in_cave) {
                        printf(
                            "There's not much point in wandering around out here, and you can't\n"
                            "explore the cave without a lamp.  So let's just call it a day.\n");
                        goto give_up;
                    } else if (limit <= 30 && !warned && here(LAMP)) {
                        printf("Your lamp is getting dim");
                        if (prop[BATTERIES] == 1)
                            printf(", and you're out of spare batteries.  You'd\n"
                                   "best start wrapping this up.\n");
                        else if (place[BATTERIES] == limbo)
                            printf(".  You'd best start wrapping this up, unless\n"
                                   "you can find some fresh batteries.  I seem to recall that there's\n"
                                   "a vending machine in the maze.  Bring some coins with you.\n");
                        else
                            printf(".  You'd best go back for those batteries.\n");
                        warned = true;
                    }
                }
            }

            if (streq(word1, "enter")) {
                if (streq(word2, "water") || streq(word2, "strea")) {
                    if (water_here)
                        report("Your feet are now wet.");
                    default_to(GO);
                } else if (*word2)
                    goto shift;
            }

            if ((streq(word1, "water") || streq(word1, "oil")) &&
                    (streq(word2, "plant") || streq(word2, "door")) &&
                    (loc == place[hash_table[lookup(word2)].meaning]))
                strcpy(word2, "pour");

parse:
            if (streq(word1, "west")) {
                if (++west_count == 10)
                    printf(" If you prefer, simply type W rather than WEST.\n");
            }

            k = lookup(word1);
            if (k < 0) {
                printf("Sorry, I don't know the word \"%s\".\n", word1);
                goto cycle;
            }
branch:
            command_type = hash_table[k].word_type;
            switch (command_type) {
            case motion_type:
                try_motion(hash_table[k].meaning);
            case object_type:
                obj = hash_table[k].meaning;
                if (!toting(obj) && !is_at_loc(obj))
                    switch (obj) {
                    case GRATE:
                        if (loc < min_lower_loc)
                            switch (loc) {
                            case road:
                            case valley:
                            case slit:
                                try_motion(DEPRESSION);
                            case cobbles:
                            case debris:
                            case awk:
                            case bird:
                            case spit:
                                try_motion(ENTRANCE);
                            default:
                                break;
                            }
                        goto cant_see_it;
                    case DWARF:
                        if (dflag >= 2 && dwarf())
                            break;
                        else
                            goto cant_see_it;
                    case PLANT:
                        if (is_at_loc(PLANT2) && prop[PLANT2]) {
                            obj = PLANT2;
                            break;
                        } else
                            goto cant_see_it;
                    case KNIFE:
                        if (loc != knife_loc)
                            goto cant_see_it;
                        knife_loc = -1;
                        report("The dwarves' knives vanish as they strike the walls of the "
                               "cave.");
                    case ROD:
                        if (!here(ROD2))
                            goto cant_see_it;
                        obj = ROD2;
                        break;
                    case WATER:
                    case OIL:
                        if (here(BOTTLE) && object_in_bottle)
                            break;
                        if ((obj == WATER && water_here) || (obj == OIL && oil_here))
                            break;
                    default:
                        goto cant_see_it;
                    }
                if (*word2)
                    break;
                if (verb)
                    goto transitive;
                printf("What do you want to do with the %s?\n", word1);
                goto cycle;
            case action_type:
                verb = hash_table[k].meaning;
                if (verb == SAY)
                    obj = *word2;
                else if (*word2)
                    break;
                if (obj)
                    goto transitive;
                else
                    goto intransitive;
            case message_type:
                goto speakit;
            default:
                break;
            }
shift:
            strcpy(word1, word2);
            *word2 = '\0';
            goto parse;

intransitive:
            k = 0;
            switch (verb) {
            case GO:
            case RELAX:
                goto report_default;
            case ON:
            case OFF:
            case POUR:
            case FILL:
            case DRINK:
            case BLAST:
            case KILL:
                goto transitive;
            case TAKE:
                if (first[loc] == 0 || link[first[loc]] || dwarf())
                    goto get_object;
                obj = first[loc];
                goto transitive;

            case EAT:
                if (!here(FOOD))
                    goto get_object;
                obj = FOOD;
                goto transitive;

            case OPEN:
            case CLOSE:
                if (place[GRATE] == loc || place[GRATE_] == loc)
                    obj = GRATE;
                else if (place[DOOR] == loc)
                    obj = DOOR;
                else if (here(CLAM))
                    obj = CLAM;
                else if (here(OYSTER))
                    obj = OYSTER;
                if (here(CHAIN)) {
                    if (obj)
                        goto get_object;
                    else
                        obj = CHAIN;
                }
                if (obj)
                    goto transitive;
                report("There is nothing here with a lock!");

            case READ:
                if (dark)
                    goto get_object;
                if (here(MAG))
                    obj = MAG;
                if (here(TABLET)) {
                    if (obj)
                        goto get_object;
                    else
                        obj = TABLET;
                }
                if (here(MESSAGE)) {
                    if (obj)
                        goto get_object;
                    else
                        obj = MESSAGE;
                }
                if (closed && toting(OYSTER))
                    obj = OYSTER;
                if (obj)
                    goto transitive;
                else
                    goto get_object;

            case INVENTORY:
                for (t = 1; t <= max_obj; t++)
                    if (toting(t) && (base[t] == NOTHING || base[t] == t) && t != BEAR) {
                        if (k == 0)
                            k = 1, printf("You are currently holding the following:\n");
                        printf(" %s\n", obj_name[t]);
                    }
                if (toting(BEAR))
                    report("You are being followed by a very large, tame bear.");
                if (k == 0)
                    report("You're not carrying anything.");
                continue;

            case BRIEF:
                interval = 10000;
                look_count = 3;
                report("Okay, from now on I'll only describe a place in full the first time\n"
                       "you come to it.  To get the full description, say \"LOOK\".");

            case SCORE:
                printf("If you were to quit now, you would score %d\n"
                       "out of a possible %d.\n",
                       score() - 4, MAX_SCORE);
                if (!yes("Do you indeed wish to quit now?", ok, ok))
                    continue;
                goto give_up;

            case QUIT:
                if (!yes("Do you really want to quit now?", ok, ok))
                    continue;
give_up:
                gave_up = true;
                goto quit;

            case FEEFIE:
                while (!streq(word1, incantation[k]))
                    k++;
                if (foobar == -k) {
                    foobar = k + 1;
                    if (foobar != 4)
                        default_to(RELAX);
                    foobar = 0;
                    if (place[EGGS] == giant || (toting(EGGS) && loc == giant))
nada_sucede:
                        report(default_msg[WAVE]);
                    if (place[EGGS] == limbo && place[TROLL] == limbo && prop[TROLL] == 0)
                        prop[TROLL] = 1;
                    k = (loc == giant ? 0 : here(EGGS) ? 1 : 2);
                    move(EGGS, giant);
                    const char *p = get_note(EGGS, k);
                    report(p);
                }
                if (foobar == 0)
                    goto nada_sucede;
                report(
                    "What's the matter, can't you read?  Now you'd best start over.");
            default:
                goto get_object;
            }
transitive:
            k = 0;
            switch (verb) {
            case SAY:
                if (*word2)
                    strcpy(word1, word2);
                k = lookup(word1);
                switch (hash_table[k].meaning) {
                case FEEFIE:
                    if (hash_table[k].word_type != action_type)
                        break;
                case XYZZY:
                case PLUGH:
                case PLOVER:
                    *word2 = '\0';
                    obj = NOTHING;
                    goto branch;
                default:
                    break;
                }
                printf("Okay, \"%s\".\n", word1);
                continue;
            case EAT:
                switch (obj) {
                case FOOD:
                    destroy(FOOD);
                    report("Thank you, it was delicious!");
                case BIRD:
                case SNAKE:
                case CLAM:
                case OYSTER:
                case DWARF:
                case DRAGON:
                case TROLL:
                case BEAR:
                    report("I think I just lost my appetite.");
                default:
                    goto report_default;
                }

            case WAVE:
                if (obj != ROD || (loc != efiss && loc != wfiss) || !toting(obj) ||
                        closing) {
                    if (toting(obj) || (obj == ROD && toting(ROD2)))
                        goto report_default;
                    default_to(DROP);
                }
                prop[CRYSTAL] = 1 - prop[CRYSTAL];
                const char *p = get_note(CRYSTAL, 2 - prop[CRYSTAL]);
                report(p);

            case BLAST:
                if (closed && prop[ROD2] >= 0) {
                    bonus = (here(ROD2) ? 25 : loc == neend ? 30 : 45);
                    printf("%s\n", message[bonus / 5]);
                    goto quit;
                } else
                    goto report_default;
            case RUB:
                if (obj == LAMP)
                    goto report_default;
                report("Peculiar.  Nothing unexpected happens.");
            case FIND:
            case INVENTORY:
                if (toting(obj))
                    default_to(TAKE);
                if (closed)
                    report("I daresay whatever you want is around here somewhere.");
                if (is_at_loc(obj) || (object_in_bottle && place[BOTTLE] == loc) ||
                        (obj == WATER && water_here) || (obj == OIL && oil_here) ||
                        (obj == DWARF && dwarf()))
                    report("I believe what you want is right here with you.");
                goto report_default;
            case BREAK:
                if (obj == VASE && prop[VASE] == 0) {
                    if (toting(VASE))
                        drop(VASE, loc);
                    printf("You have taken the vase and hurled it delicately to the "
                           "ground.\n");
smash:
                    prop[VASE] = 2;
                    base[VASE] = VASE;
                    continue;
                } else if (obj != MIRROR)
                    goto report_default;
                if (closed) {
                    printf("You strike the mirror a resounding blow, whereupon "
                           "it shatters into a\n"
                           "myriad tiny fragments.");
                    goto dwarves_upset;
                }
                report("It is too far up for you to reach.");

            case WAKE:
                if (closed && obj == DWARF) {
                    printf("You prod the nearest dwarf, who wakes up grumpily, "
                           "takes one look at\n"
                           "you, curses, and grabs for his axe.\n");
                    goto dwarves_upset;
                } else
                    goto report_default;
            case ON:
                if (!here(LAMP))
                    goto report_default;
                if (limit < 0)
                    report("Your lamp has run out of power.");
                prop[LAMP] = 1;
                printf("Your lamp is now on.\n");
                if (was_dark)
                    goto commence;
                continue;

            case OFF:
                if (!here(LAMP))
                    goto report_default;
                prop[LAMP] = 0;
                printf("Your lamp is now off.\n");
                if (dark)
                    printf("%s\n", pitch_dark_msg);
                continue;

            case DRINK:
                if (obj == NOTHING) {
                    if (!water_here && !(here(BOTTLE) && prop[BOTTLE] == 0))
                        goto get_object;
                } else if (obj != WATER)
                    default_to(EAT);
                if (!(here(BOTTLE) && prop[BOTTLE] == 0))
                    goto report_default;
                prop[BOTTLE] = 1;
                place[WATER] = limbo;
                report("The bottle of water is now empty.");

            case POUR:
                if (obj == NOTHING || obj == BOTTLE) {
                    obj = (prop[BOTTLE] == 0 ? WATER : prop[BOTTLE] == 2 ? OIL : 0);
                    if (obj == NOTHING)
                        goto get_object;
                }
                if (!toting(obj))
                    goto report_default;
                if (obj != WATER && obj != OIL)
                    report("You can't pour that.");
                prop[BOTTLE] = 1;
                place[obj] = limbo;
                if (loc == place[PLANT]) {
                    if (obj != WATER)
                        report("The plant indignantly shakes the oil off its leaves "
                               "and asks, \"Water?\"");
                    const char *p = get_note(PLANT, 1 + prop[PLANT]);
                    printf("%s\n", p);
                    prop[PLANT] += 2;
                    if (prop[PLANT] > 4)
                        prop[PLANT] = 0;
                    prop[PLANT2] = prop[PLANT] >> 1;
                    stay_put;
                }
                if (loc == place[DOOR])
                    switch (obj) {
                    case WATER:
                        prop[DOOR] = 0;
                        report(
                            "The hinges are quite thoroughly rusted now and won't budge.");
                    case OIL:
                        prop[DOOR] = 1;
                        report("The oil has freed up the hinges so that the door will now "
                               "open.");
                    default:
                        break;
                    }
                report("Your bottle is empty and the ground is wet.");
            case FILL:
                if (obj == VASE) {
                    if (no_liquid_here)
                        report("There is nothing here with which to fill the vase.\n");
                    if (!toting(VASE))
                        report(default_msg[DROP]);
                    printf("The sudden change in temperature has delicately shattered "
                           "the vase.\n");
                    goto smash;
                }
                if (!here(BOTTLE)) {
                    if (obj == NOTHING)
                        goto get_object;
                    else
                        goto report_default;
                } else if (obj != NOTHING && obj != BOTTLE)
                    goto report_default;
                if (!bottle_empty)
                    report("Your bottle is already full.");
                if (no_liquid_here)
                    report("There is nothing here with which to fill the bottle.");
                prop[BOTTLE] = flags[loc] & oil;
                if (toting(BOTTLE))
                    place[prop[BOTTLE] ? OIL : WATER] = inhand;
                printf("Your bottle is now full of %s.\n",
                       prop[BOTTLE] ? "oil" : "water");
                continue;
            case TAKE:
                if (toting(obj))
                    goto report_default;
                if (base[obj]) {
                    if (obj == CHAIN && prop[BEAR])
                        report("The chain is still locked.");
                    if (obj == BEAR && prop[BEAR] == 1)
                        report("The bear is still chained to the wall.");
                    if (obj == PLANT && prop[PLANT] <= 0)
                        report("The plant has exceptionally deep roots and cannot be "
                               "pulled free.");
                    report("You can't be serious!");
                }
                if (obj == WATER || obj == OIL) {
                    if (here(BOTTLE) && object_in_bottle)
                        obj = BOTTLE;
                    else {
                        obj = BOTTLE;
                        if (toting(BOTTLE))
                            change_to(FILL);
                        report("You have nothing in which to carry it.");
                    }
                }
                if (holding >= 7)
                    report("You can't carry anything more.  You'll have to drop "
                           "something first.");
                if (obj == BIRD && prop[BIRD] == 0) {
                    if (toting(ROD))
                        report("The bird was unafraid when you entered, "
                               "but as you approach it becomes\n"
                               "disturbed and you cannot catch it.");
                    if (toting(CAGE))
                        prop[BIRD] = 1;
                    else
                        report("You can catch the bird, but you cannot carry it.");
                }
                if (obj == BIRD || (obj == CAGE && prop[BIRD]))
                    carry(BIRD + CAGE - obj);
                carry(obj);
                if (obj == BOTTLE && !bottle_empty)
                    place[prop[BOTTLE] ? OIL : WATER] = inhand;
                default_to(RELAX);
            case DROP:
                if (obj == ROD && toting(ROD2) && !toting(ROD))
                    obj = ROD2;
                if (!toting(obj))
                    goto report_default;
                if (obj == COINS && here(PONY)) {
                    destroy(COINS);
                    drop(BATTERIES, loc);
                    prop[BATTERIES] = 0;
                    report(get_note(BATTERIES, 0));
                }
                if (obj == BIRD) {
                    if (here(SNAKE)) {
                        printf("The little bird attacks the green snake, "
                               "and in an astounding flurry\n"
                               "drives the snake away.\n");
                        k = 1;
                        if (closed)
                            goto dwarves_upset;
                        destroy(SNAKE);
                        prop[SNAKE] = 1;
                    } else if (is_at_loc(DRAGON) && prop[DRAGON] == 0) {
                        destroy(BIRD);
                        prop[BIRD] = 0;
                        if (place[SNAKE] == hmk)
                            lost_treasures++;
                        report("The little bird attacks the green dragon, "
                               "and in an astounding flurry\n"
                               "gets burnt to a cinder.  The ashes blow away.");
                    }
                }

                if (obj == VASE && loc != soft) {
                    prop[VASE] = (place[PILLOW] == loc ? 0 : 2);
                    const char *pnote = get_note(VASE, 1 + prop[VASE]);
                    printf("%s\n", pnote);
                    k = 1;
                    if (prop[VASE])
                        base[VASE] = VASE;
                }
                if (obj == BEAR && is_at_loc(TROLL)) {
                    printf("The bear lumbers toward the troll, "
                           "who lets out a startled shriek and\n"
                           "scurries away.  The bear soon gives up the pursuit and wanders back.\n");
                    k = 1;
                    destroy(TROLL);
                    destroy(TROLL_);
                    drop(TROLL2, swside);
                    drop(TROLL2_, neside);
                    prop[TROLL] = 2;
                    move(BRIDGE, swside);
                    move(BRIDGE_, neside);
                }
                if (object_in_bottle)
                    obj = BOTTLE;
                if (obj == BOTTLE && !bottle_empty)
                    place[prop[BOTTLE] ? OIL : WATER] = limbo;
                if (obj == BIRD)
                    prop[BIRD] = 0;
                else if (obj == CAGE && prop[BIRD])
                    drop(BIRD, loc);
                drop(obj, loc);
                if (k)
                    continue;
                else
                    default_to(RELAX);

            case TOSS:
                if (obj == ROD && toting(ROD2) && !toting(ROD))
                    obj = ROD2;
                if (!toting(obj))
                    goto report_default;
                if (is_treasure(obj) && is_at_loc(TROLL)) {
                    drop(obj, limbo);
                    destroy(TROLL);
                    destroy(TROLL_);
                    drop(TROLL2, swside);
                    drop(TROLL2_, neside);
                    move(BRIDGE, swside);
                    move(BRIDGE_, neside);
                    report("The troll catches your treasure and scurries away out of sight.");
                }

                if (obj == FOOD && here(BEAR)) {
                    obj = BEAR;
                    change_to(FEED);
                }
                if (obj != AXE)
                    change_to(DROP);
                if (dwarf()) {
                    for (j = 1; j <= nd; j++)
                        if (dloc[j] == loc)
                            break;
                    if (ran(3) < 2) {
                        dloc[j] = limbo;
                        dseen[j] = 0;
                        dkill++;
                        if (dkill == 1)
                            printf("You killed a little dwarf.  The body vanishes in a cloud of greasy\n"
                                   "black smoke.\n");
                        else
                            printf("You killed a little dwarf.\n");
                    } else
                        printf("You attack a little dwarf, but he dodges out of the way.\n");
                    drop(AXE, loc);
                    stay_put;
                }

                if (is_at_loc(DRAGON) && prop[DRAGON] == 0)
                    printf("The axe bounces harmlessly off the dragon's thick scales.\n");
                else if (is_at_loc(TROLL))
                    printf("The troll deftly catches the axe, examines it carefully, "
                           "and tosses it\n"
                           "back, declaring, \"Good workmanship, but it's not valuable enough.\"\n");
                else if (here(BEAR) && prop[BEAR] == 0) {
                    drop(AXE, loc);
                    prop[AXE] = 1;
                    base[AXE] = AXE;
                    if (place[BEAR] == loc)
                        move(BEAR, loc);
                    report("The axe misses and lands near the bear where you can't get at it.");
                } else {
                    obj = NOTHING;
                    change_to(KILL);
                }
                drop(AXE, loc);
                stay_put;

            case KILL:
                if (obj == NOTHING) {
                    if (dwarf())
                        k++, obj = DWARF;
                    if (here(SNAKE))
                        k++, obj = SNAKE;
                    if (is_at_loc(DRAGON) && prop[DRAGON] == 0)
                        k++, obj = DRAGON;
                    if (is_at_loc(TROLL))
                        k++, obj = TROLL;
                    if (here(BEAR) && prop[BEAR] == 0)
                        k++, obj = BEAR;
                    if (k == 0) {
                        if (here(BIRD) && oldverb != TOSS)
                            k++, obj = BIRD;
                        if (here(CLAM) || here(OYSTER))
                            k++, obj = CLAM;
                    }
                    if (k > 1)
                        goto get_object;
                }
                switch (obj) {
                case 0:
                    report("There is nothing here to attack.");
                case BIRD: {
                    if (closed)
                        report("Oh, leave the poor unhappy bird alone.");
                    destroy(BIRD);
                    prop[BIRD] = 0;
                    if (place[SNAKE] == hmk)
                        lost_treasures++;
                    report("The little bird is now dead.  Its body disappears.");
                }

                case DRAGON:
                    if (prop[DRAGON] == 0) {
                        printf("With what?  Your bare hands?\n");
                        verb = ABSTAIN;
                        obj = NOTHING;
                        if (!listen()) {
                            goto quit;
                        };
                        if (!(streq(word1, "yes") || streq(word1, "y")))
                            goto pre_parse;
                        const char *p = get_note(DRAGON, 1);
                        printf("%s\n", p);
                        prop[DRAGON] = 2;
                        prop[RUG] = 0;
                        base[RUG] = NOTHING;
                        base[DRAGON_] = DRAGON_;
                        destroy(DRAGON_);
                        base[RUG_] = RUG_;
                        destroy(RUG_);
                        for (t = 1; t <= max_obj; t++)
                            if (place[t] == scan1 || place[t] == scan3)
                                move(t, scan2);
                        loc = scan2;
                        stay_put;
                    }

cry:
                    report("For crying out loud, the poor thing is already dead!");
                case CLAM:
                case OYSTER:
                    report("The shell is very strong and impervious to attack.");
                case SNAKE:
                    report(
                        "Attacking the snake both doesn't work and is very dangerous.");
                case DWARF:
                    if (closed)
                        goto dwarves_upset;
                    report("With what?  Your bare hands?");
                case TROLL:
                    report("Trolls are close relatives with the rocks "
                           "and have skin as tough as\n"
                           "a rhinoceros hide.  The troll fends off your blows effortlessly.");
                case BEAR:
                    switch (prop[BEAR]) {
                    case 0:
                        report("With what?  Your bare hands?  Against HIS bear hands?");
                    case 3:
                        goto cry;
                    default:
                        report("The bear is confused; he only wants to be your friend.");
                    }
                default:
                    goto report_default;
                }

            case FEED:
                switch (obj) {
                case BIRD:
                    report("It's not hungry (it's merely pinin' for the fjords).  Besides, you\n"
                           "have no bird seed.");
                case TROLL:
                    report("Gluttony is not one of the troll's vices.  Avarice, however, is.");
                case DRAGON:
                    if (prop[DRAGON])
                        report(default_msg[EAT]);
                    break;
                case SNAKE:
                    if (closed || !here(BIRD))
                        break;
                    destroy(BIRD);
                    prop[BIRD] = 0;
                    lost_treasures++;
                    report("The snake has now devoured your bird.");
                case BEAR:
                    if (!here(FOOD)) {
                        if (prop[BEAR] == 0)
                            break;
                        if (prop[BEAR] == 3)
                            verb = EAT;
                        goto report_default;
                    }
                    destroy(FOOD);
                    prop[BEAR] = 1;
                    prop[AXE] = 0;
                    base[AXE] = NOTHING;
                    report("The bear eagerly wolfs down your food, after which he seems to calm\n"
                           "down considerably and even becomes rather friendly.");
                case DWARF:
                    if (!here(FOOD))
                        goto report_default;
                    dflag++;
                    report("You fool, dwarves eat only coal!  Now you've made him REALLY mad!");
                default:
                    report(default_msg[CALM]);
                }
                report("There's nothing here it wants to eat (except perhaps you).");

            case OPEN:
            case CLOSE:
                switch (obj) {
                case OYSTER:
                case CLAM:
                    if (verb == CLOSE)
                        report("What?");
                    if (!toting(TRIDENT)) {
                        printf("You don't have anything strong enough to open the %s",
                               clam_oyster);
                        report(".");
                    }
                    if (toting(obj)) {
                        printf("I advise you to put down the %s before opening it.  ",
                               clam_oyster);
                        report(obj == CLAM ? ">STRAIN!<" : ">WRENCH!<");
                    }
                    if (obj == CLAM) {
                        destroy(CLAM);
                        drop(OYSTER, loc);
                        drop(PEARL, sac);
                        report(
                            "A glistening pearl falls out of the clam and rolls away.  Goodness,\n"
                            "this must really be an oyster.  (I never was very good at identifying\n"
                            "bivalves.)  Whatever it is, it has now snapped shut again.");
                    } else
                        report(
                            "The oyster creaks open, revealing nothing but oyster inside.\n"
                            "It promptly snaps shut again.");

                case GRATE:
                case CHAIN:
                    if (!here(KEYS))
                        report("You have no keys!");
                    if (obj == CHAIN) {
                        if (verb == OPEN) {
                            if (prop[CHAIN] == 0)
                                report("It was already unlocked.");
                            if (prop[BEAR] == 0)
                                report("There is no way to get past the bear "
                                       "to unlock the chain, which is\n"
                                       "probably just as well.");
                            prop[CHAIN] = 0, base[CHAIN] = NOTHING;
                            if (prop[BEAR] == 3)
                                base[BEAR] = BEAR;
                            else
                                prop[BEAR] = 2, base[BEAR] = NOTHING;
                            report("The chain is now unlocked.");
                        }
                        if (loc != barr)
                            report("There is nothing here to which the chain can be locked.");
                        if (prop[CHAIN])
                            report("It was already locked.");
                        prop[CHAIN] = 2, base[CHAIN] = CHAIN;
                        if (toting(CHAIN))
                            drop(CHAIN, loc);
                        report("The chain is now locked.");
                    }

                    if (closing) {
                        {
                            if (!panic)
                                clock2 = 15, panic = true;
                            printf(
                                "A mysterious recorded voice groans into life and announces:\n"
                                "\"This exit is closed.  Please leave via main office.\"\n");
                        }
                        continue;
                    }
                    k = prop[GRATE];
                    prop[GRATE] = (verb == OPEN);
                    switch (k + 2 * prop[GRATE]) {
                    case 0:
                        report("It was already locked.");
                    case 1:
                        report("The grate is now locked.");
                    case 2:
                        report("The grate is now unlocked.");
                    case 3:
                        report("It was already unlocked.");
                    }

                case KEYS:
                    report("You can't lock or unlock the keys.");
                case CAGE:
                    report("It has no lock.");
                case DOOR:
                    if (prop[DOOR])
                        default_to(RELAX);
                    report("The door is extremely rusty and refuses to open.");
                default:
                    goto report_default;
                }

            case READ:
                if (dark)
                    goto cant_see_it;
                switch (obj) {
                case MAG:
                    report("I'm afraid the magazine is written in dwarvish.");
                case TABLET:
                    report("\"CONGRATULATIONS ON BRINGING LIGHT INTO THE DARK-ROOM!\"");
                case MESSAGE:
                    report("\"This is not the maze where the pirate hides his treasure "
                           "chest.\"");
                case OYSTER:
                    if (hinted[1]) {
                        if (toting(OYSTER))
                            report("It says the same thing it did before.");
                    } else if (closed && toting(OYSTER)) {
                        offer(1);
                        continue;
                    }
                default:
                    goto report_default;
                }

            default:
                goto report_default;
            }
speakit:
            report(message[hash_table[k].meaning]);
report_default:
            if (default_msg[verb])
                report(default_msg[verb]) else continue;
get_object:
            word1[0] = toupper(word1[0]);
            printf("%s what?\n", word1);
            goto cycle;
cant_see_it:
            if ((verb == FIND || verb == INVENTORY) && *word2 == '\0')
                goto transitive;
            printf("I see no %s here.\n", word1);
            continue;
        }
try_move:
        newloc = loc;
        if (mot == NOWHERE)
            continue;
        if (mot == BACK) {
            l = (forced_move(oldloc) ? oldoldloc : oldloc);
            oldoldloc = oldloc;
            oldloc = loc;
            if (l == loc) {
                printf("Sorry, but I no longer seem to remember how you got here.\n");
                continue;
            }
            for (q = start[loc], qq = nullptr; q < start[loc + 1]; q++) {
                ll = q->dest;
                if (ll == l)
                    goto found;
                if (ll <= max_loc && forced_move(ll) && start[ll]->dest == l)
                    qq = q;
            }
            if (qq == nullptr) {
                printf("You can't get there from here.\n");
                continue;
            } else
                q = qq;
found:
            mot = q->mot;
            goto go_for_it;
        }
        if (mot == LOOK) {
            if (++look_count <= 3)
                printf("Sorry, but I am not allowed to give more detail.  I will repeat the\n"
                       "long description of your location.\n");
            was_dark = false;
            visits[loc] = 0;
            continue;
        }

        if (mot == CAVE) {
            if (loc < min_in_cave)
                printf(
                    "I can't see where the cave is, but hereabouts no stream can run on\n"
                    "the surface for long.  I would try the stream.\n");
            else
                printf("I need more detailed instructions to do that.\n");
            continue;
        }

        oldoldloc = oldloc;
        oldloc = loc;
go_for_it:
        for (q = start[loc]; q < start[loc + 1]; q++) {
            if (forced_move(loc) || q->mot == mot)
                break;
        }
        if (q == start[loc + 1]) {
            if (mot == CRAWL)
                printf("Which way?");
            else if (mot == XYZZY || mot == PLUGH)
                printf("%s", default_msg[WAVE]);
            else if (verb == FIND || verb == INVENTORY)
                printf("%s", default_msg[FIND]);
            else if (mot <= FORWARD)
                switch (mot) {
                case IN:
                case OUT:
                    printf("I don't know in from out here.  Use compass points or name something\n"
                           "in the general direction you want to go.");
                    break;
                case FORWARD:
                case L:
                case R:
                    printf("I am unsure how you are facing.  Use compass points or nearby objects.");
                    break;
                default:
                    printf("There is no way to go in that direction.");
                }
            else
                printf("I don't know how to apply that word here.");
            printf("\n");
            continue;
        }

        while (true) {
            j = q->cond;
            if (j > 300) {
                if (prop[j % 100] != (int)((j - 300) / 100))
                    break;
            } else if (j <= 100) {
                if (j == 0 || pct(j))
                    break;
            } else if (toting(j % 100) || (j >= 200 && is_at_loc(j % 100)))
                break;
no_good:
            for (qq = q++; q->dest == qq->dest && q->cond == qq->cond; q++)
                ;
        }

        newloc = q->dest;
        if (newloc <= max_loc)
            continue;
        if (newloc > max_spec) {
            printf("%s\n", remarks[newloc - max_spec]);
stay:
            newloc = loc;
            continue;
        }
        switch (newloc) {
        case ppass:
            if (holding == 0 || (toting(EMERALD) && holding == 1)) {
                newloc = alcove + proom - loc;
                continue;
            } else {
                printf(
                    "Something you're carrying won't fit through the tunnel with you.\n"
                    "You'd best take inventory and drop something.\n");
                goto stay;
            }
        case pdrop:
            drop(EMERALD, loc);
            goto no_good;
        case troll:
            if (prop[TROLL] == 1) {
                move(TROLL, swside);
                move(TROLL_, neside);
                prop[TROLL] = 0;
                destroy(TROLL2);
                destroy(TROLL2_);
                move(BRIDGE, swside);
                move(BRIDGE_, neside);
                const char *p = get_note(TROLL, 1);
                printf("%s\n", p);
                goto stay;
            }
            newloc = neside + swside - loc;
            if (prop[TROLL] == 0)
                prop[TROLL] = 1;
            if (!toting(BEAR))
                continue;
            printf(
                "Just as you reach the other side, the bridge buckles beneath the\n"
                "weight of the bear, who was still following you around.  You\n"
                "scrabble desperately for support, but as the bridge collapses you\n"
                "stumble back and fall into the chasm.\n");
            prop[BRIDGE] = 1;
            prop[TROLL] = 2;
            drop(BEAR, newloc);
            base[BEAR] = BEAR;
            prop[BEAR] = 3;
            if (prop[SPICES] < 0 && place[SPICES] >= neside)
                lost_treasures++;
            if (prop[CHAIN] < 0 && place[CHAIN] >= neside)
                lost_treasures++;
            oldoldloc = newloc;
            goto death;
        default:
            break;
        }
    }

pitch_dark:
    printf("You fell into a pit and broke every bone in your body!\n");
    oldoldloc = loc;

death:
    death_count++;
    if (closing) {
        printf(
            "It looks as though you're dead.  Well, seeing as how it's so close\n"
            "to closing time anyway, let's just call it a day.\n");
        goto quit;
    }
    if (!yes(death_wishes[2 * death_count - 2], death_wishes[2 * death_count - 1],
             ok) ||
            death_count == max_deaths)
        goto quit;

    if (toting(LAMP))
        prop[LAMP] = 0;
    place[WATER] = limbo;
    place[OIL] = limbo;
    for (int j = max_obj; j > 0; j--)
        if (toting(j))
            drop(j, j == LAMP ? road : oldoldloc);
    loc = oldloc = house;
    goto commence;

dwarves_upset:
    printf("The resulting ruckus has awakened the dwarves.  There are now several\n"
           "threatening little dwarves in the room with you!  Most of them throw\n"
           "knives at you!  All of them get you!\n");

quit:
    k = score();
    printf("You scored %d point%s out of a possible %d, using %d turn%s.\n", k,
           k == 1 ? "" : "s", MAX_SCORE, turns, turns == 1 ? "" : "s");
    for (j = 0; CLASS_SCORE[j] < k; j++)
        ;

    printf("%s\nTo achieve the next higher rating", CLASS_MESSAGE[j]);

    if (j < HIGHEST_CLASS)
        printf(", you need %d more point%s.\n", CLASS_SCORE[j] + 1 - k,
               CLASS_SCORE[j] == k ? "" : "s");
    else
        printf(" would be a neat trick!\nCongratulations!!\n");

    exit(0);
}
