#include <curses.h>
#include <time.h>

#include <chrono>
#include <memory>
#include <random>
#include <thread>

#include "game_state.hpp"

constexpr size_t border_h = 1;
constexpr size_t border_w = 1;

void init() {
  initscr();
  cbreak();  // disable char buffering
  noecho();  // disable char echo
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);
  curs_set(FALSE);
  refresh();  // some weird fuckery with stdscr clearing
}

void end() {
  endwin();
}

struct raw_point {
  raw_point(point p)
    : i(p.i + border_h),
      j(p.j + border_w) {}
  
  const size_t i;
  const size_t j;
};

void draw(WINDOW *win, const raw_point &p, tile t) {
  char cell;
  
  switch (t) {
  case tile::body:
    cell = 'o';
    break;
  case tile::fruit:
    cell = 'x';
    break;
  default:
    cell = ' ';
  }

  mvwaddch(win, p.i, p.j, cell);
}


point find_distinct_from(point taboo, size_t w, size_t h, range_rng rng) {
  size_t i, j;
  
  do {
    i = rng(0, h-1);
    j = rng(0, w-1);
  } while (i == taboo.i && j == taboo.j);
  return point{i, j};
}

range_rng init_rng() {
  srand(time(nullptr));
  return [](size_t lb, size_t ub) {
    return lb + (rand() % static_cast<int>(ub - lb + 1));
  };
}

int main(int argc, char **argv) {
  const auto rng = init_rng();
  const std::chrono::milliseconds game_loop_period(75);

  init();
  int screen_w, screen_h;
  getmaxyx(stdscr, screen_h, screen_w);
  WINDOW *win = newwin(screen_h, screen_w, 0, 0);
    
  const size_t w = screen_w - 2 * border_w;
  const size_t h = screen_h - 2 * border_h;

  const point snake_head {h / 2, w / 2};
  const point first_fruit = find_distinct_from(snake_head, w, h, rng);

  // prepare board
  wborder(win, 0, 0, 0, 0, 0, 0, 0, 0);
  draw(win, snake_head, tile::body);
  draw(win, first_fruit, tile::fruit);
  wrefresh(win);
  
  auto state = game_state::create(h, w, snake_head, {first_fruit}, 4, rng);
  optional<direction> d;
  bool running = true;

  std::thread input_loop([&d, &running]() {
      for (;;) {
        switch (getch()) {
        case KEY_UP:
        case 'w':
          d = direction::north;
          break;
        case KEY_LEFT:
        case 'a':
          d = direction::west;
          break;
        case KEY_DOWN:
        case 's':
          d = direction::south;
          break;
        case KEY_RIGHT:
        case 'd':
          d = direction::east;
          break;
        case 0x1b:  // ESC
        case 'q':
          running = false;
          break;
        default:
          break;
        }
      }
    });

  for (;;) {
    std::this_thread::sleep_for(game_loop_period);

    if (!running) break;
    if (!d) continue;

    std::vector<point> updates = state->step(*d);
    if (updates.empty()) break;
    for (const auto& p : updates) {
      draw(win, p, state->get_tile(p));
    }
    
    wrefresh(win);
  }

  mvwaddstr(win, 0, 0, "Game over. Press q to exit.");
  wrefresh(win);

  while (running);

  end();
  exit(EXIT_SUCCESS);
}
