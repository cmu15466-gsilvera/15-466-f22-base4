#pragma once

#include <iostream>
#include <sstream> // ostringstream
#include <string>
#include <unordered_map>

struct PlayerState {
    bool hasSword = false;
    bool hasShield = false;
    bool isDamaged = false;
    bool hasMedicine = false;
    bool hasFlower = false;
    bool hasTorch = false;
    bool hasRope = false;
    int treasure = 0;
    void print(int selections_made = 0) const
    {
        std::ostringstream oss;
        oss << "Selection: " << selections_made << " | ";
        oss << "Inventory: {";
        std::vector<std::string> inventory;
        if (hasSword)
            inventory.push_back("sword");
        if (hasShield)
            inventory.push_back("shield");
        if (hasMedicine)
            inventory.push_back("medicine");
        if (hasFlower)
            inventory.push_back("flower");
        if (hasTorch)
            inventory.push_back("torch");
        if (hasRope)
            inventory.push_back("rope");
        for (int i = 0; i < inventory.size(); i++) {
            oss << inventory[i];
            if (i < inventory.size() - 1)
                oss << ", ";
        }
        oss << "}";
        if (isDamaged)
            oss << " | is hurt!";
        oss << " | Treasure: " << treasure;
        std::cout << oss.str() << std::endl;
    }
};

struct State {
    const std::string context;
    const std::string left_text, right_text;
    const std::function<int(PlayerState&)> left, right;
    State(const std::string& c, const std::string& left_t, const std::string& right_t, const std::function<int(PlayerState&)>& l, const std::function<int(PlayerState&)>& r)
        : context(c)
        , left_text(left_t)
        , right_text(right_t)
        , left(l)
        , right(r)
    {
    }
};

struct Story {
    std::vector<State> story_graph = {
        /* 0 */ { "The hero's journey begins here", "Exit", "Play",
            [&](PlayerState& p) { exit(0); return 0; },
            [&](PlayerState& p) { return 1; } },
        /* 1 */ { "You enter a dark cave and are preparing to explore.", "Pick up shield", "Pick up sword", //
            [&](PlayerState& p) { p.hasShield = true; return 2; }, //
            [&](PlayerState& p) { p.hasSword = true; return 2; } },
        /* 2 */ { "You venture further and see 2 tunnels.", "Spikey vines tunnel", "Dark empty tunnel", //
            [&](PlayerState& p) { return 3; }, //
            [&](PlayerState& p) { return (p.hasShield) ? 6 : 7; } },
        /* 3 */ { "You get blocked by the vines.", "Cut through vines", "Go back", //
            [&](PlayerState& p) { return (p.hasSword) ? 4 : 2; }, //
            [&](PlayerState& p) { return 2; } }, // always go back
        /* 4 */ { "You see a rope and torch on the ground", "Pick up rope", "Pick up torch", //
            [&](PlayerState& p) { p.hasRope = true; return 5; }, //
            [&](PlayerState& p) { p.hasTorch = true; return 5; } },
        /* 5 */ { "You see a climbable-looking cliff", "Try to climb", "Continue forward", //
            [&](PlayerState& p) { return (p.hasRope) ? 8 : 5; }, //
            [&](PlayerState& p) { return 14; } },
        /* 6 */ { "Booby traps appear! But your shield protected you.", "Continue forward", "Go back", //
            [&](PlayerState& p) { return 8; }, //
            [&](PlayerState& p) { return 2; } },
        /* 7 */ { "Booby traps appear! You take an arrow to the knee!.", "Continue forward", "Go back", //
            [&](PlayerState& p) { p.isDamaged = true; return 8; }, //
            [&](PlayerState& p) { p.isDamaged = true; return 2; } },
        /* 8 */ { "At the end of this tunnel you see more items", "Get medicine (heal wounds)", "Get flower", //
            [&](PlayerState& p) { if (p.isDamaged) {p.isDamaged = false;} else {p.hasMedicine = true; }; return 9; }, //
            [&](PlayerState& p) { p.hasFlower = true; return 9; } },
        /* 9 */ { "You see a well and a path forward", "Go down well", "Go down path", //
            [&](PlayerState& p) { return 10; }, //
            [&](PlayerState& p) { return 14; } },
        /* 10 */ { "You fall down and hurt yourself", "Use medicine", "Get up", //
            [&](PlayerState& p) { if (p.hasMedicine) {p.hasMedicine = false; p.isDamaged = false;return 12;} else {return 10;}; }, //
            [&](PlayerState& p) { return 11; } },
        /* 11 */ { "You tried getting up but died.", "Exit", "Retry", //
            [&](PlayerState& p) { exit(0); return 0; }, //
            [&](PlayerState& p) { return 0; } },
        /* 12 */ { "You are healed and see a shiny object", "Pick up treasure", "Try to climb out", //
            [&](PlayerState& p) { p.treasure++; return 13; }, //
            [&](PlayerState& p) { return 13; } },
        /* 13 */ { "You try to climb out but need a rope!", "Use rope", "Stuck", //
            [&](PlayerState& p) { return p.hasRope ? 9 : 13; }, //
            [&](PlayerState& p) { return 13; } }, // infinite loop?
        /* 14 */ { "You go down the path forward and see a goblin", "Act hostile", "Act peaceful", //
            [&](PlayerState& p) { if (p.hasSword) {return (p.hasTorch) ? 16 : 15;} else {return 14;} }, //
            [&](PlayerState& p) { return p.hasSword ? 26 : 24; } }, //
        /* 15 */ { "Goblins surprize attack!", "Use medicine to heal", "Fight back", //
            [&](PlayerState& p) { if (p.hasMedicine) {p.hasMedicine = false; return 17;} else{return 18;}; }, //
            [&](PlayerState& p) { return 18; } }, //
        /* 16 */ { "Your torch scares the goblins but gets extinguished!", "Continue forward", "Continue forward", //
            [&](PlayerState& p) { p.hasTorch = false; return 19; }, //
            [&](PlayerState& p) { p.hasTorch = false; return 19; } }, //
        /* 17 */ { "You see a large tree with a chest at the top", "Climb tree", "Cut down tree", //
            [&](PlayerState& p) { return p.hasRope ? 20 : 17; }, //
            [&](PlayerState& p) { return p.hasSword ? 21 : 17; } }, //
        /* 18 */ { "You died to your goblin-wounds", "Retry", "Exit", //
            [&](PlayerState& p) { return 0; }, //
            [&](PlayerState& p) { exit(0); return 0; } }, //
        /* 19 */ { "You see a large tree with a chest at the top", "Climb tree", "Burn down tree", //
            [&](PlayerState& p) { return p.hasRope ? 20 : 19; }, //
            [&](PlayerState& p) { return p.hasTorch ? 22 : 19; } }, //
        /* 20 */ { "You climb the tree and find treasure!", "Leave cave", "Leave cave", //
            [&](PlayerState& p) { p.treasure++; return 0; }, //
            [&](PlayerState& p) { p.treasure++; return 0; } }, //
        /* 21 */ { "You cut down the tree and goblins attack!", "Heal and escape", "Try to escape", //
            [&](PlayerState& p) { return p.hasMedicine ? 23 : 18; }, //
            [&](PlayerState& p) { return 18; } }, //
        /* 22 */ { "You burn down the tree and goblins attack!", "Heal and escape", "Try to escape", //
            [&](PlayerState& p) { return p.hasMedicine ? 23 : 18; }, //
            [&](PlayerState& p) { return 18; } }, //
        /* 23 */ { "You escaped!", "Exit", "Retry", // TODO: add number of treasures
            [&](PlayerState& p) { exit(0); return 0; }, //
            [&](PlayerState& p) { return 0; } }, //
        /* 24 */ { "Goblins are friendly", "Offer flower", "Escape", //
            [&](PlayerState& p) { return p.hasFlower ? 25 : 24; }, //
            [&](PlayerState& p) { return 23; } }, //
        /* 25 */ { "Goblins give you treasure!", "Leave", "Leave", //
            [&](PlayerState& p) { return 23; }, //
            [&](PlayerState& p) { return 23; } }, //
        /* 26 */ {
            "The goblins are scared of your sword", "Discard sword", "Attack with sword", //
            [&](PlayerState& p) { return 17; }, //
            [&](PlayerState& p) { return 15; }, //
        }

    };
    PlayerState player;
    int state_id = 0;
};