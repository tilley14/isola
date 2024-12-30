#pragma once

/*
    clang-format off

    Isola is a two-player abstract stragegy game played on a 7 by 7 board.
    The board is initally filled with free spaces, except for the player's starting
    position. Each player has one piece that is placed in the middle row closest to his/her
    side of the board. The goal of the game is to isolate your opponent.
    The players take turns moving. Each move is made up of two subsequent actions.
        1. Moving one's piece either horizontally, vertically, or diagonally to a free space
            (killing the space that the player was moving from)
        2. Shooting an "Arrow" and removing on free space from the board.
    The first player to not be able to move at the beginning of their turn is the loser.

    clang-format on
*/

#include <cassert>
#include <charconv>
#include <cstdio>
#include <iostream>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

enum class Direction {
  DownLeft = 1,
  Down = 2,
  DownRight = 3,
  Left = 4,
  Right = 6,
  UpLeft = 7,
  Up = 8,
  UpRight = 9
};

struct Player {
  std::string avitar;
  int row;
  int col;

  void setCoordinates(int r, int c) {
    row = r;
    col = c;
  }
};

class Isola {

  const std::string EMPTY_SPOT;

  Player *activePlayer;
  Player p1;
  Player p2;

  std::vector<std::vector<std::string>> board;

public:
  Isola()
      : EMPTY_SPOT("+"), activePlayer(nullptr),
        p1{.avitar = "B", .row = 0, .col = 3},
        p2{.avitar = "W", .row = 6, .col = 3},
        board(7, std::vector<std::string>(7, EMPTY_SPOT)) {
    activePlayer = &p1;
    board[p1.row][p1.col] = p1.avitar;
    board[p2.row][p2.col] = p2.avitar;
  }

  void play() {
    displayRules();
    drawBoard();

    /*
       Each turn will consist of
       1. CheckHasValidMove()
       2. Move()
       3. FireArrow()
       4. Alernate Active Player
   */

    while (checkHasValidMove(activePlayer)) {
      move(activePlayer);
      fireArrow(activePlayer);
      activePlayer = activePlayer == &p1 ? &p2 : &p1;
    }

    // If active player cannot move, they lose
    std::cout << activePlayer->avitar << " is no longer able to move."
              << std::endl;
    std::cout << ((activePlayer == &p1) ? p2.avitar : p1.avitar)
              << " is the winner!" << std::endl;

    pause();
  }

  void move(Player *p) {
    int direction = 0;
    bool valid = false;

    // Continue asking for a move until the move is successful
    do {
      do {
        std::cout << "Turn: " << p->avitar
                  << "\nUse the number pad to move in a direction 1-9, but not "
                     "5 (see key): ";

        std::string input;
        std::cin >> input;

        std::string_view sv{input};
        auto [ptr, ec] = std::from_chars(sv.cbegin(), sv.cend(), direction);

        if (ec != std::errc{} || direction == 5 || direction < 1 ||
            direction > 9) {
          std::cout << "Invalid Input!" << std::endl;
        } else {
          valid = true;
        }
      } while (!valid);
    } while (!attemptMove(p, static_cast<Direction>(direction)));
  }

  bool attemptMove(Player *p, Direction direction) {
    assert(p != nullptr);
    assert(static_cast<int>(direction) != 5);
    assert(static_cast<int>(direction) >= 1);
    assert(static_cast<int>(direction) <= 9);

    int row = p->row;
    int col = p->col;

    switch (direction) {
    case Direction::Up: {
      row--;
    } break;
    case Direction::Down: {
      row++;
    } break;
    case Direction::Left: {
      col--;
    } break;
    case Direction::Right: {
      col++;
    } break;
    case Direction::UpLeft: {
      row--;
      col--;
    } break;
    case Direction::UpRight: {
      row--;
      col++;
    } break;
    case Direction::DownLeft: {
      row++;
      col--;
    } break;
    case Direction::DownRight: {
      row++;
      col++;
    } break;
    }

    bool isValidMove = true;

    if (row < 0 || row > rows() - 1 || col < 0 || col > cols() - 1) {
      isValidMove = false;
      std::cout << "Invalid move, please try again: " << std::endl;
    } else if (board[row][col] == "A") {
      isValidMove = false;
      std::cout << "That space is dead, please try again: " << std::endl;
    } else if (board[row][col] == p1.avitar || board[row][col] == p2.avitar) {
      isValidMove = false;
      std::cout << "That space is occupied by the opponent, please try again: "
                << std::endl;
    } else {
      isValidMove = true;
      std::cout << "Valid move" << std::endl;

      // Kill the old location of the player
      board[p->row][p->col] = "A";
      p->setCoordinates(row, col);
      board[p->row][p->col] = p->avitar;

      clearTerm();
      drawBoard();
    }

    return isValidMove;
  }

  void fireArrow(Player *p) {
    assert(p != nullptr);

    std::cout << p->avitar << " time to fire an arrow!" << std::endl;

    int row;
    int col;

    do {
      std::errc ec;

      do {
        std::cout << "Please select a row: ";
        int in_row;
        std::string input;
        std::cin >> input;
        std::string_view sv{input};
        auto [ptr, e] = std::from_chars(sv.cbegin(), sv.cend(), in_row);
        ec = e;

        row = in_row - 1;

        if (ec != std::errc{} || row < 0 || row > rows() - 1) {
          std::cout << "Invalid coordinate!" << std::endl;
        }

      } while (ec != std::errc{} || row < 0 || row > rows() - 1);

      do {
        std::cout << "Please select a column: ";
        int in_col;
        std::string input;
        std::cin >> input;
        std::string_view sv{input};
        auto [ptr, e] = std::from_chars(sv.cbegin(), sv.cend(), in_col);
        ec = e;

        col = in_col - 1;

        if (ec != std::errc{} || col < 0 || col > cols() - 1) {
          std::cout << "Invalid coordinate!" << std::endl;
        }
      } while (ec != std::errc{} || col < 0 || col > cols() - 1);

      if (board[row][col] != "+") {
        std::cout << "That location cannot be destroyed." << std::endl;
      }

    } while (board[row][col] != "+");

    board[row][col] = "A";
    clearTerm();
    drawBoard();
  }

  bool checkHasValidMove(Player *p) {
    assert(p != nullptr);

    bool hasValidMove = false;
    int row = p->row;
    int col = p->col;

    // Check to see if there is an open spot "+" around the player
    if (row - 1 >= 0 && col - 1 >= 0 && board[row - 1][col - 1] == "+") {
      hasValidMove = true;
    } else if (row - 1 >= 0 && board[row - 1][col] == "+") {
      hasValidMove = true;
    } else if (row - 1 >= 0 && col + 1 < cols() &&
               board[row - 1][col + 1] == "+") {
      hasValidMove = true;
    } else if (col - 1 >= 0 && board[row][col - 1] == "+") {
      hasValidMove = true;
    } else if (col + 1 < cols() && board[row][col + 1] == "+") {
      hasValidMove = true;
    } else if (row + 1 < rows() && col - 1 >= 0 &&
               board[row + 1][col - 1] == "+") {
      hasValidMove = true;
    } else if (row + 1 < rows() && board[row + 1][col] == "+") {
      hasValidMove = true;
    } else if (row + 1 < rows() && col + 1 < cols() &&
               board[row + 1][col + 1] == "+") {
      hasValidMove = true;
    }

    return hasValidMove;
  }

  void displayRules() {
    // clang-format off
    std::string str =
        "********** Isola Game **********"
        "\nEach player has one piece."
        "\nThe Board has 7 by 7 positions, which initially contain"
        "\nfree spaces ('+') except for the initial positions"
        "\nof the players. A Move consists of two subsequent actions:"
        "\n\n1. Moving one's piece to a neighboring (horizontally, vertically,"
        "\ndiagonally) field that contains a '+' but not the opponents piece."
        "\n\n2. Removing any '+' with no piece on it (Replacing it with an 'A')."
        "\n\nIf a player cannot move at the beginning of their turn, that player loses the game.";
    // clang-format on

    std::cout << str << std::endl;
    pause("Press any key to start...");
  }

  void drawBoard() {
    std::string str = "  "; // reserve space for row labels
    for (int col = 0; col < cols(); ++col) {
      str += std::to_string(col + 1);
    }
    str += "\n";

    for (int row = 0; row < rows(); ++row) {
      str += std::to_string(row + 1) + " ";
      for (int col = 0; col < cols(); ++col) {
        str += board[row][col];
      }
      str += "\n";
    }

    str += "\n7-8-9"
           "\n4---6"
           "\n1-2-3\n";

    clearTerm();
    std::cout << str << std::endl;
  }

  void clearTerm() {
    // system("cls");
    system("clear");
  }

  void pause(const std::string &msg = "Press enter to continue...") {
    std::cout << msg << std::endl;
    std::getchar();
  }

  int rows() const { return 7; }
  int cols() const { return 7; }
};
