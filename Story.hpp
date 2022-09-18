#pragma once

#include <iostream>
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
    void print() const
    {
        std::cout << "Sword: " << hasSword << " Shield: " << hasShield << " isDamaged: " << isDamaged << " hasMedicine: " << hasMedicine << " hasFlower: " << hasFlower << " hasTorch: " << hasTorch << " hasRope: " << hasRope << " numTreasure: " << treasure << std::endl;
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
            [&](PlayerState& player) { return 0; },
            [&](PlayerState& player) { return 1; } },
        /* 1 */ { "You enter a dark cave and are preparing to explore.", "Pick up shield", "Pick up sword", //
            [&](PlayerState& player) { player.hasShield = true; return 2; }, //
            [&](PlayerState& player) { player.hasSword = true; return 2; } },
        /* 2 */ { "You venture further and see 2 tunnels.", "Enter the spikey vines tunnel", "Enter the empty tunnel", //
            [&](PlayerState& player) { return 3; }, //
            [&](PlayerState& player) { return (player.hasShield) ? 6 : 7; } },
        /* 3 */ { "You get blocked by the vines.", "Cut through the vines (sword)", "Go back", //
            [&](PlayerState& player) { return (player.hasSword) ? 4 : 2; }, //
            [&](PlayerState& player) { return 2; } }, // always go back
        /* 4 */ { "You see a rope and torch on the ground", "Pick up rope", "Pick up torch", //
            [&](PlayerState& player) { player.hasRope = true; return 5; }, //
            [&](PlayerState& player) { player.hasTorch = true; return 5; } },
        /* 5 */ { "You see a climbable-looking cliff", "Try to climb", "Continue forward", //
            [&](PlayerState& player) { return (player.hasRope) ? 8 : 5; }, //
            [&](PlayerState& player) { return 14; } },
        /* 6 */ { "Booby traps appear! But your shield protected you.", "Continue forward", "Go back", //
            [&](PlayerState& player) { return 8; }, //
            [&](PlayerState& player) { return 2; } },
        /* 7 */ { "Booby traps appear! You take an arrow to the knee!.", "Continue forward", "Go back", //
            [&](PlayerState& player) { player.isDamaged = true; return 8; }, //
            [&](PlayerState& player) { player.isDamaged = true; return 2; } },
        /* 8 */ { "At the end of this tunnel you see more items", "Pick up medicine (heal wounds)", "Pick up flower", //
            [&](PlayerState& player) { if (player.isDamaged) {player.isDamaged = false;} else {player.hasMedicine = true; }; return 9; }, //
            [&](PlayerState& player) { player.hasFlower = true; return 9; } },
        /* 9 */ { "You see a well and a path forward", "Go down well", "Go down path", //
            [&](PlayerState& player) { return 10; }, //
            [&](PlayerState& player) { return 14; } },
        /* 10 */ { "You fall down and hurt yourself", "Use medicine", "Get up", //
            [&](PlayerState& player) { if (player.hasMedicine) {player.hasMedicine = false; player.isDamaged = false;return 12;} else {return 10;}; }, //
            [&](PlayerState& player) { return 11; } },
        /* 11 */ { "You tried getting up but died.", "Exit", "Retry", //
            [&](PlayerState& player) { return 0; }, //
            [&](PlayerState& player) { return 0; } },
        /* 12 */ { "You are healed and see a shiny object", "Pick up treasure", "Try to climb out", //
            [&](PlayerState& player) { player.treasure++; return 13; }, //
            [&](PlayerState& player) { return 13; } },
        /* 13 */ { "You try to climb out but need a rope!", "Use rope", "Stuck", //
            [&](PlayerState& player) { return player.hasRope ? 9 : 13; }, //
            [&](PlayerState& player) { return 13; } }, // infinite loop?
        /* 14 */ { "You go down the path forward and see a goblin", "Act hostile", "Act peaceful", //
            [&](PlayerState& player) { if (player.hasSword) {return (player.hasTorch) ? 16 : 15;} else {return 14;} }, //
            [&](PlayerState& player) { return player.hasSword ? 26 : 24; } }, //
        /* 15 */ { "Goblins surprize attack!", "Use medicine to heal", "Fight back", //
            [&](PlayerState& player) { if (player.hasMedicine) {player.hasMedicine = false; return 17;} else{return 18;}; }, //
            [&](PlayerState& player) { return 18; } }, //
        /* 16 */ { "Your torch scares the goblins but gets extinguished!", "Continue forward", "Continue forward", //
            [&](PlayerState& player) { player.hasTorch = false; return 19; }, //
            [&](PlayerState& player) { player.hasTorch = false; return 19; } }, //
        /* 17 */ { "You see a large tree with a chest at the top", "Climb tree", "Cut down tree", //
            [&](PlayerState& player) { return player.hasRope ? 20 : 17; }, //
            [&](PlayerState& player) { return player.hasSword ? 21 : 17; } }, //
        /* 18 */ { "You died to your goblin-wounds", "Retry", "Exit", //
            [&](PlayerState& player) { return 0; }, //
            [&](PlayerState& player) { return 0; } }, //
        /* 19 */ { "You see a large tree with a chest at the top", "Climb tree", "Burn down tree", //
            [&](PlayerState& player) { return player.hasRope ? 20 : 19; }, //
            [&](PlayerState& player) { return player.hasTorch ? 22 : 19; } }, //
        /* 20 */ { "You climb the tree and find treasure!", "Leave cave", "Leave cave", //
            [&](PlayerState& player) { player.treasure++; return 0; }, //
            [&](PlayerState& player) { player.treasure++; return 0; } }, //
        /* 21 */ { "You cut down the tree and goblins attack!", "Heal and escape", "Try to escape", //
            [&](PlayerState& player) { return player.hasMedicine ? 23 : 18; }, //
            [&](PlayerState& player) { return 18; } }, //
        /* 22 */ { "You burn down the tree and goblins attack!", "Heal and escape", "Try to escape", //
            [&](PlayerState& player) { return player.hasMedicine ? 23 : 18; }, //
            [&](PlayerState& player) { return 18; } }, //
        /* 23 */ { "You escaped!", "Exit", "Retry", // TODO: add number of treasures
            [&](PlayerState& player) { return 0; }, //
            [&](PlayerState& player) { return 0; } }, //
        /* 24 */ { "Goblins are friendly", "Offer flower", "Escape", //
            [&](PlayerState& player) { return player.hasFlower ? 25 : 24; }, //
            [&](PlayerState& player) { return 23; } }, //
        /* 25 */ { "Goblins give you treasure!", "Leave", "Leave", //
            [&](PlayerState& player) { return 23; }, //
            [&](PlayerState& player) { return 23; } }, //
        /* 26 */ {
            "The goblins are scared of your sword", "Discard sword", "Attack with sword", //
            [&](PlayerState& player) { return 17; }, //
            [&](PlayerState& player) { return 15; }, //
        }

    };
    PlayerState player;
    int state_id = 0;
};