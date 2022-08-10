#pragma once

#include <unordered_map>

struct Choice {
    std::string text;
    int next;
};

struct State {
    std::string context;
    Choice left, right;
    State(const std::string& c, const struct Choice& l, const struct Choice& r)
        : context(c)
        , left(l)
        , right(r)
    {
    }
};

struct Story {
    std::vector<State> story_graph = {
        /* 0 */ { State("The hero's journey begins here", { "Play", 1 }, { "Exit", 0 }) },
        /* 1 */ { State("You enter a dark cave and are preparing to explore.", { "Pick up sword", 2 }, { "Pick up shield", 3 }) },
        /* 2 */ { State("You are attacked by a troll", { "game over l", 0 }, { "game over r", 0 }) },
        /* 3 */ { State("You fall to your death.", { "game over l", 0 }, { "game over r", 0 }) },
    };
    int state_id = 0;
};