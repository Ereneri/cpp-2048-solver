#include "Expectimax.hpp"
#include<tuple>

float expectimaxScore(int depth, Game game, bool is_max) {
    float score = 0.0;
    for (int stage = 0; stage < depth; ++stage) {
        movelist possible_moves;
        game.generatePossibleMoves(possible_moves);
        if (possible_moves.size() == 0) {
            return 0.0;
        }
        
        const Game *best_game = NULL;
        float best_score = -std::numeric_limits<float>::max();
        for (auto const& move : possible_moves) {
            float copy_h_score = Heuristics::get_h_score(move.second.state);
            if (copy_h_score > best_score) {
                best_game = &(move.second);
                best_score = copy_h_score;
            }
        }

        // average of the worst scores
        float worst_score = std::numeric_limits<float>::max();
        float sum_worst_scores = 0.0;
        for (int i = 0; i < DIM; ++i) {
            for (int j = 0; j < DIM; ++j) {
                if (best_game->state[i][j] == 0) {
                    board state_copy = best_game->state;
                    state_copy[i][j] = 2;
                    float copy_h_score = Heuristics::get_h_score(state_copy);
                    if (copy_h_score < worst_score) {
                    worst_score = copy_h_score;
                    }
                    state_copy[i][j] = 4;
                    copy_h_score = Heuristics::get_h_score(state_copy);
                    if (copy_h_score < worst_score) {
                    worst_score = copy_h_score;
                    }
                    sum_worst_scores += worst_score;
                }
            }
        }
        score = sum_worst_scores / (float) 4;
    }
    return score;
}

std::tuple<int, int, int> expectimaxSearch(int depth, int display_level, Game game) {
    int moves = 0;
    // std::cout << "Attempting to solve a new game with Expectimax... " << std::flush;
    while (game.canContinue()) {
        if (display_level >= 2) {
            std::cout << std::endl << game;
        }

        std::map<int, weightedmoves> possibilities = game.computePossibilities();
        std::map<int, float> scores;

        for (auto const& entry : possibilities) {
            int move = entry.first;                             // Direction of move
            weightedmoves weighted_subset = entry.second;       // Vector of (prob, game)
            int len = (int) weighted_subset.size();
            if (len > 0) {
                scores[move] = 0.0;
                for (int j = 0; j < len; ++j) {
                    scores[move] += expectimaxScore(depth, weighted_subset[j].second, true) * weighted_subset[j].first;
                }
            }
        }
        if (display_level >= 3) {
            std::cout << "Heuristic score: " << Heuristics::get_h_score(game.state) << std::endl << "Move scores: ";
            for (auto const& score : scores) {
                std::cout << score.first << ": " << score.second << ", ";
            }
            std::cout << std::endl;
        }

        float max_score = -std::numeric_limits<float>::max();
        int max_choice = UP;
        for (auto const& score : scores) {      // Determine best move choice based on tabulated scores
            if (score.second > max_score) {
                max_score = score.second;
                max_choice = score.first;
            }
        }

        switch (max_choice) {
            case UP:
                game.up(false);
                break;
            case DOWN:
                game.down(false);
                break;
            case LEFT: 
                game.left(false);
                break;
            case RIGHT:
                game.right(false);
        }
        moves++;
        if (display_level >= 2) {
            std::cout << "Move " << moves << ": " << max_choice << std::endl;
        }
    }
    if (display_level <= 1) {
        // std::cout << "Done!" << (display_level == 0 ? "\n" : "");
    }
    if (display_level >= 1) {
        std::cout << std::endl << game << std::endl;
    }
    return std::tuple<int, int, int>(game.getHighestTile(), game.score, moves);
}

/**
 * Creates and completes n-many games using the Expectimax simulation function.
 * Tabulates data from game each in order to display results at completion.
 */
int expectimaxSolve(int n, int depth, int display_level, 
    std::vector<int> scores, std::vector<int> highest_tiles, std::vector<int> moves) {

    int successes = 0;
    for (int i = 0; i < n; ++i) {
        std::tuple<int, int, int> result = expectimaxSearch(depth, display_level, Game());
        if (std::get<0>(result) >= WIN) {
            successes++;
        }
        highest_tiles.push_back(std::get<0>(result));
        scores.push_back(std::get<1>(result));
        moves.push_back(std::get<2>(result));
    }
    return successes;
}