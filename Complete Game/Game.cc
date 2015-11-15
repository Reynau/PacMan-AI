#include "Game.hh"
#include "Registry.hh"


using namespace std;


void Game::run (vector<string> names, istream& is, ostream& os) {
    cerr << "info: loading game" << endl;
    Board b0(is);
    cerr << "info: loaded game" << endl;

    if (int(names.size()) != b0.nb_players()) {
        cerr << "fatal: wrong number of players." << endl;
        exit(EXIT_FAILURE);
    }

    vector<Player*> players;        
    for (int player = 0; player < b0.nb_players(); ++player) {
        string name = names[player];
        cerr << "info: loading player " << name << endl;
        players.push_back(Registry::new_player(name));
        b0.names_[player] = name;
    }
    cerr << "info: players loaded" << endl;

    // ofstream ofs("/tmp/debug.txt");

    b0.print_preamble(os);
    b0.print(os);

    // b0.print_debug(ofs);
    
    Board b1 = b0;
    for (int round = 1; round < b0.nb_rounds(); ++round) {
        cerr << "info: start round " << round << endl;
        os << "actions" << endl;
        vector<Action> actions;
        for (int player = 0; player < b0.nb_players(); ++player) {
            cerr << "info:     start player " << player << endl;
            Action a;
            players[player]->reset(player, b1, a);
            players[player]->play();
            actions.push_back(*players[player]);

            os << player << endl;
            Action(*players[player]).print(os);
            cerr << "info:     end player " << player << endl;
        }
        Action actions_done;
        cerr << "start next" << endl;
        Board b2 = b1.next(actions, actions_done, b0);
        cerr << "end next" << endl;
        os << endl << "movements" << endl;
        actions_done.print(os);
        b2.print(os);
        // b2.print_debug(ofs);
        b1 = b2;
        cerr << "info: end round " << round << endl;
    }
    cerr << "info: game played" << endl;
}


