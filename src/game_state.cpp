#include "game_state.hpp"

#include <iterator>

namespace {

point predict_step(const point &start, direction dir) {
  switch (dir) {
    case north:
      return point{
          start.i - 1,
          start.j,
      };
    case south:
      return point{
          start.i + 1,
          start.j,
      };
    case east:
      return point{
          start.i,
          start.j + 1,
      };
    case west:
      return point{
          start.i,
          start.j - 1,
      };
  }
}
}  // namespace

std::optional<point> snake::walk(const point &p) {
  body_.push(p);

  if (pending_growth_ > 0) {
    pending_growth_--;
    return std::optional<point>();
  } else {
    const auto tail = body_.front();
    body_.pop();
    return tail;
  }
}

void snake::grow(uint32_t amount) { pending_growth_ += amount; }

point snake::head() const { return body_.back(); }

playing_field::playing_field(size_t width, size_t height)
    : width_(width), height_(height), data_(width * height, tile::empty) {
  for (size_t i = 0; i < width * height; ++i) {
    free_spots_.insert(i);
  }
}

point playing_field::to_coord(size_t idx) const {
  return point{
      idx / width_,
      idx % width_,
  };
}

size_t playing_field::from_coord(const point &p) const {
  return width_ * p.i + p.j;
}

tile playing_field::get(const point &p) const { return data_[from_coord(p)]; }

void playing_field::put(const point &p, tile t) {
  const size_t idx = from_coord(p);
  data_[idx] = t;
  if (t == tile::empty) {
    free_spots_.insert(idx);
  } else {
    free_spots_.erase(idx);
  }
}

bool playing_field::is_legal(const point &p) const {
  return p.j < width_ && p.i < height_;
}

bool playing_field::is_full() const { return free_spots_.empty(); }

std::optional<point> playing_field::find_empty(range_rng rng) const {
  if (is_full()) {
    return std::nullopt;
  }
  std::unordered_set<size_t>::const_iterator it(free_spots_.begin());
  std::advance(it, rng(0, free_spots_.size()));
  return to_coord(*it);
}

std::unique_ptr<game_state> game_state::create(size_t height, size_t width,
                                               point snake_origin,
                                               std::vector<point> fruits,
                                               uint32_t growth_rate,
                                               range_rng rng) {
  auto field = std::make_unique<playing_field>(width, height);
  for (const auto &p : fruits) {
    field->put(p, tile::fruit);
  }

  return std::unique_ptr<game_state>(
      new game_state(std::move(field), std::make_unique<snake>(snake_origin),
                     growth_rate, rng));
}

tile_updates game_state::step(direction d) {
  tile_updates updates;

  const point new_head = predict_step(snake_->head(), d);
  if (!field_->is_legal(new_head)) {
    return updates;
  }

  auto update_field = [&updates, this](const point &p, tile t) {
    field_->put(p, t);
    updates.push_back(p);
  };
  bool grow = false;

  switch (field_->get(new_head)) {
    case tile::body:
      return updates;
    case tile::fruit:
      snake_->grow(growth_rate_);
      grow = true;
      [[fallthrough]];
    case tile::empty:
      if (auto maybe_tail = snake_->walk(new_head); maybe_tail) {
        update_field(*maybe_tail, tile::empty);
      }
      update_field(new_head, tile::body);
  }

  if (grow) {
    if (auto maybe_fruit = field_->find_empty(rng_); maybe_fruit) {
      update_field(*maybe_fruit, tile::fruit);
    }
  }

  return updates;
}
