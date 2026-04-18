// ============================================================
// main.cpp — punkt startowy programu
//
// Cały program to dwie linie:
//   1. Stwórz obiekt Game (konstruktor ładuje zasoby, otwiera okno)
//   2. Uruchom pętlę gry (blokuje program dopóki okno jest otwarte)
//
// Gdy użytkownik zamknie okno, run() kończy działanie,
// destruktory klas SFML automatycznie zwalniają pamięć (RAII).
// ============================================================

#include "Game.h"

int main() {
    Game game;   // konstruktor: okno, font, dźwięki, tło, stan = MENU
    game.run();  // pętla: handleEvents → update → render, w kółko
    return 0;
}
