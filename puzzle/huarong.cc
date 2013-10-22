#include <boost/functional/hash/hash.hpp>

#include <deque>
#include <type_traits>
#include <unordered_set>
#include <vector>

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

int board[5][4] = {
//  0  1  2  3
  { 1, 2, 2, 3, },   // 0
  { 1, 2, 2, 3, },   // 1
  { 4, 5, 5, 6, },   // 2
  { 4, 7, 8, 6, },   // 3
  { 9, 0, 0, 10 } }; // 4

struct Mask;

const int kRows = 5;
const int kColumns = 4;
const int kBlocks = 10;

enum class Shape // : int8_t
{
  kInvalid,
  kSingle,
  kHorizon,
  kVertical,
  kSquare,
};

struct Block
{
  Shape shape;
  int left, top;  // int8_t

  Block()
    : shape(Shape::kInvalid),
      left(-1),
      top(-1)
  {
  }

  Block(Shape s, int left, int top)
    : shape(s),
      left(left),
      top(top)
  {
    assert(shape != Shape::kInvalid);
    assert(left >= 0 && left < kColumns);
    assert(top >= 0 && top < kRows);
  }

  int bottom() const
  {
    const static int delta[] = { 0, 0, 0, 1, 1, };
    assert(shape != Shape::kInvalid);
    return top + delta[static_cast<int>(shape)];
  }

  int right() const
  {
    const static int delta[] = { 0, 0, 1, 0, 1, };
    assert(shape != Shape::kInvalid);
    return left + delta[static_cast<int>(shape)];
  }

  void mask(int8_t value, Mask* mask) const;
};

struct Mask
{
  Mask()
  {
    bzero(board_, sizeof(board_));
  }

  bool operator==(const Mask& rhs) const
  {
    return memcmp(board_, rhs.board_, sizeof board_) == 0;
  }

  size_t hashValue() const
  {
    const int8_t* begin = board_[0];
    return boost::hash_range(begin, begin + sizeof(board_));
  }

  void print() const
  {
    for (int i = 0; i < kRows; ++i)
    {
      for (int j = 0; j < kColumns; ++j)
      {
        printf(" %c", board_[i][j] + '0');
      }
      printf("\n");
    }
  }

  void set(int8_t value, int y, int x)
  {
    assert(value > 0);
    assert(x >= 0 && x < kColumns);
    assert(y >= 0 && y < kRows);
    assert(board_[y][x] == 0);
    board_[y][x] = value;
  }

  bool empty(int y, int x) const
  {
    assert(x >= 0 && x < kColumns);
    assert(y >= 0 && y < kRows);
    return board_[y][x] == 0;
  }

 private:
  int8_t board_[kRows][kColumns];
};

namespace std
{
  template<> struct hash<Mask>
  {
    size_t operator()(const Mask& x) const
    {
      return x.hashValue();
    }
  };
}

inline void Block::mask(int8_t value, Mask* mask) const
{
  mask->set(value, top, left);
  switch (shape)
  {
    case Shape::kHorizon:
      mask->set(value, top, left+1);
      break;
    case Shape::kVertical:
      mask->set(value, top+1, left);
      break;
    case Shape::kSquare:
      mask->set(value, top, left+1);
      mask->set(value, top+1, left);
      mask->set(value, top+1, left+1);
      break;
    default:
      assert(shape == Shape::kSingle);
      ;
  }
}

struct State
{
  Mask toMask() const
  {
    Mask m;
    for (int i = 0; i < kBlocks; ++i)
    {
      Block b = blocks_[i];
      b.mask(static_cast<int>(b.shape), &m);
    }
    return m;
  }

  bool isSolved() const
  {
    // FIXME: magic number
    Block square = blocks_[1];
    assert(square.shape == Shape::kSquare);
    return (square.left == 1 && square.top == 3);
  }

  template<typename FUNC>
  void move(const FUNC& func) const
  {
    static_assert(std::is_convertible<FUNC, std::function<void(const State&)>>::value,
                  "func must be callable with a 'const State&' parameter.");
    const Mask mask = toMask();

    for (int i = 0; i < kBlocks; ++i)
    {
      Block b = blocks_[i];

      // move up
      if (b.top > 0 && mask.empty(b.top-1, b.left)
                    && mask.empty(b.top-1, b.right()))
      {
        State next = *this;
        next.step++;
        next.blocks_[i].top--;
        func(next);
      }

      // move down
      if (b.bottom() < kRows-1 && mask.empty(b.bottom()+1, b.left)
                               && mask.empty(b.bottom()+1, b.right()))
      {
        State next = *this;
        next.step++;
        next.blocks_[i].top++;
        func(next);
      }

      // move left
      if (b.left > 0 && mask.empty(b.top,      b.left-1)
                     && mask.empty(b.bottom(), b.left-1))
      {
        State next = *this;
        next.step++;
        next.blocks_[i].left--;
        func(next);
      }

      // move right
      if (b.right() < kColumns-1 && mask.empty(b.top,      b.right()+1)
                                 && mask.empty(b.bottom(), b.right()+1))
      {
        State next = *this;
        next.step++;
        next.blocks_[i].left++;
        func(next);
      }
    }
  }

  // std::vector<State> moves() const;

  Block blocks_[kBlocks];
  int step = 0;
};

int main()
{
  printf("sizeof(Mask) = %zd, sizeof(State) = %zd\n", sizeof(Mask), sizeof(State));
  std::unordered_set<Mask> seen;
  std::deque<State> queue;

  State initial;
  initial.blocks_[0] = Block(Shape::kVertical, 0, 0);
  initial.blocks_[1] = Block(Shape::kSquare,   1, 0);
  initial.blocks_[2] = Block(Shape::kVertical, 3, 0);
  initial.blocks_[3] = Block(Shape::kVertical, 0, 2);
  initial.blocks_[4] = Block(Shape::kHorizon,  1, 2);
  initial.blocks_[5] = Block(Shape::kVertical, 3, 2);
  initial.blocks_[6] = Block(Shape::kSingle, 1, 3);
  initial.blocks_[7] = Block(Shape::kSingle, 2, 3);
  initial.blocks_[8] = Block(Shape::kSingle, 0, 4);
  initial.blocks_[9] = Block(Shape::kSingle, 3, 4);

  queue.push_back(initial);
  seen.insert(initial.toMask());

  while (!queue.empty())
  {
    const State curr = queue.front();
    queue.pop_front();

    if (curr.isSolved())
    {
      printf("found solution with %d steps\n", curr.step);
      break;
    }
    else if (curr.step > 200)
    {
      printf("too many steps.\n");
      break;
    }

    curr.move([&seen, &queue](const State& next) {
      auto result = seen.insert(next.toMask());
      if (result.second)
        queue.push_back(next);
    });

    // for (const State& next : curr.moves())
    // {
    //   auto result = seen.insert(next.toMask());
    //   if (result.second)
    //     queue.push_back(next);
    // }
  }
}
