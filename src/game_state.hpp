#ifndef GAME_STATE_H
#define GAME_STATE_H
#include <cstddef>
#include <functional>
#include <experimental/optional>
#include <memory>
#include <queue>
#include <set>

using std::experimental::optional;
using std::size_t;
using std::unique_ptr;

enum direction { north, south, east, west };

enum tile { empty = 0, fruit, body };

struct point {
  const size_t i;
  const size_t j;
};
  
class snake {
public:
  explicit snake(point start) 
    : pending_growth_(0) {
    body_.push(start);
  }

  optional<point> walk(const point &p);
  void grow(unsigned int amount);
  point head() const;

private:
  unsigned int pending_growth_;
  std::queue<point> body_;
};

typedef std::function<size_t(size_t, size_t)> range_rng;
typedef std::vector<point> tile_updates;

class playing_field {
public:
  playing_field(size_t width, size_t height);

  tile get(const point &p) const;
  void put(const point &p, tile t);
  bool is_legal(const point &p) const;
  bool is_full() const;

  optional<point> find_empty(range_rng rng) const;

private:
  point to_coord(size_t idx) const;
  size_t from_coord(const point &p) const;

  const size_t width_;
  const size_t height_;
  std::vector<tile> data_;
  std::set<size_t> free_spots_;
};

class game_state {
public:
  static unique_ptr<game_state> create(size_t height, size_t width,
                                       point snake_origin,
                                       std::vector<point> fruits,
                                       unsigned int growth_rate,
                                       range_rng rng);

  tile_updates step(direction d);
  tile get_tile(point p) const { return field_->get(p); }

private:  
  game_state(unique_ptr<playing_field> field,
             unique_ptr<snake> s,
             unsigned int growth_rate,
             range_rng rng)
    : field_(std::move(field)), snake_(std::move(s)),
      growth_rate_(growth_rate), rng_(rng) {}
  
  const unique_ptr<playing_field> field_;
  const unique_ptr<snake> snake_;
  const unsigned int growth_rate_;
  const range_rng rng_;
};

#endif //GAME_STATE_H
