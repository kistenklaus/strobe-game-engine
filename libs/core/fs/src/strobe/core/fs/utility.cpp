#include "./utility.hpp"
#include <cassert>

namespace strobe::fs::details {

std::size_t normalize_path_inplace(std::span<char> nonNullTerminatedPath) {
  const auto end = nonNullTerminatedPath.end();
  const auto begin = nonNullTerminatedPath.begin();
  auto read_it = begin;
  auto write_it = begin;

  enum class State {
    Empty,
    SingleDot,
    DoubleDot,
    Word,
  };

  State state = State::Empty;
  bool firstWord = true;
  char prev = '\0';
  while (read_it != end) {
    char c = *read_it++;
    State nextState;
    switch (state) {
    case State::Empty:
      if (c == '.') {
        nextState = State::SingleDot;
      } else if (!firstWord && c == '/') {
        nextState = State::Empty; // matches "***//" and ignores it.
      } else {
        *write_it++ = c;
        nextState = State::Word;
      }
      break;
    case State::SingleDot:
      if (c == '.') {
        nextState = State::DoubleDot;
      } else if (c == '/') { // matches "./"
        // we completely ignore this case.
        // unless the path started with ./ then keep it.
        if (firstWord == true) {
          *write_it++ = '.';
          *write_it++ = '/';
        }
        nextState = State::Empty;
      } else { // matches ".x" (i.e. dotfiles)
        *write_it++ = '.';
        *write_it++ = c;
        nextState = State::Word;
      }
      break;
    case State::DoubleDot:
      if (c == '/') { // matches "../"But if you have multiple leading slashes
                      // (like "////abc/xyz/"), they might get collapsed
                      // incorrectly or treated as segments.
        // backtrace to last parent directory.
        // unless we started with ../
        if (firstWord == true) {
          *write_it++ = '.';
          *write_it++ = '.';
          *write_it++ = '/';
        } else {
          // here we know that the begin() to write_it is already normalized.
          --write_it; // now points to the last char written.
          assert(*write_it == '/');
          do {
            --write_it;
          } while (write_it != begin && *write_it != '/');
          if (write_it == begin) {
            *write_it++ = '.';
            *write_it++ = '.';
            *write_it++ = '/';
          } else {
            write_it++;
          }
        }
        nextState = State::Empty;
      } else {
        *write_it++ = '.';
        *write_it++ = '.';
        *write_it++ = c;
        nextState = State::Empty;
      }
      break;
    case State::Word:
      if (prev == '/' && c == '/') {
        break; // skip
      }
      *write_it++ = c;
      if (c == '/') {
        firstWord = false;
        nextState = State::Empty;
      } else {
        nextState = State::Word;
      }
      break;
    }
    state = nextState;
    prev = c;
  }
  if (state == State::DoubleDot) {
    if (firstWord == true) {
      *write_it++ = '.';
      *write_it++ = '.';
    } else {
      // here we know that the begin() to write_it is already normalized.
      --write_it; // now points to the last char written.
      assert(*write_it == '/');
      do {
        --write_it;
      } while (write_it != begin && *write_it != '/');
      if (write_it == begin) {
        *write_it++ = '.';
        *write_it++ = '/';
      } else {
        write_it++;
      }
    }
  }

  std::size_t newSize = std::distance(begin, write_it);
  assert(newSize <= nonNullTerminatedPath.size());
  return newSize;
}

} // namespace strobe::fs::details
