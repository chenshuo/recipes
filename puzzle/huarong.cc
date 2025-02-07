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

//int board[5][4] = {
//  //0  1  2  3
//  { 1, 2, 2, 3, },   // 0
//  { 1, 2, 2, 3, },   // 1
//  { 4, 5, 5, 6, },   // 2
//  { 4, 7, 8, 6, },   // 3
//  { 9, 0, 0, 10 } }; // 4

int hrd[5][4] = {{0}};

//int board[5][4] = {
// // 0  1  2  3
//  {11, 1, 1, 3},   // 0
//  {12, 1, 1, 3},   // 1
//  { 4, 2, 2,13},   // 2
//  { 4, 6, 6,14},   // 3
//  { 0, 5, 5, 0} }; // 4

//华容道第三关
int board[5][4] = {
   // 0  1  2  3
    { 1, 2, 2, 3},   // 0
    { 4, 2, 2, 3},   // 1
    { 5, 6, 6, 7},   // 2
    { 5, 8, 8, 9},   // 3
    { 0,10,10, 0} }; // 4



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

    int temp_parents = this->counts;

    for (int i = 0; i < kBlocks; ++i)
    {
      Block b = blocks_[i];

      // move up
      if (b.top > 0 && mask.empty(b.top-1, b.left)
                    && mask.empty(b.top-1, b.right()))
      {
        State next = *this;
        next.parents = temp_parents;
        next.step++;
        next.blocks_[i].top--;
        func(next);
      }

      // move down
      if (b.bottom() < kRows-1 && mask.empty(b.bottom()+1, b.left)
                               && mask.empty(b.bottom()+1, b.right()))
      {
        State next = *this;
        next.parents = temp_parents;
        next.step++;
        next.blocks_[i].top++;
        func(next);
      }

      // move left
      if (b.left > 0 && mask.empty(b.top,      b.left-1)
                     && mask.empty(b.bottom(), b.left-1))
      {
        State next = *this;
        next.parents = temp_parents;
        next.step++;
        next.blocks_[i].left--;
        func(next);
      }

      // move right
      if (b.right() < kColumns-1 && mask.empty(b.top,      b.right()+1)
                                 && mask.empty(b.bottom(), b.right()+1))
      {
        State next = *this;
        next.parents = temp_parents;
        next.step++;
        next.blocks_[i].left++;
        func(next);
      }
    }
  }

  // std::vector<State> moves() const;

  Block blocks_[kBlocks];

public:
  int step = 0;

  int parents = 0;
  int counts = 0;
};

int test_main(int list_count_all_parent[512000], int pcount[], int pstep[])
{
    printf("sizeof(Mask) = %zd, sizeof(State) = %zd\n", sizeof(Mask), sizeof(State));
    std::unordered_set<Mask> seen;
    std::deque<State> queue;

    State initial;

    // int board[5][4] = {
    // //0  1  2  3
    // { 1, 2, 2, 3, },   // 0
    // { 1, 2, 2, 3, },   // 1
    // { 4, 5, 5, 6, },   // 2
    // { 4, 7, 8, 6, },   // 3
    // { 9, 0, 0, 10 } }; // 4
    // initial.blocks_[0] = Block(Shape::kVertical, 0, 0);
    // initial.blocks_[1] = Block(Shape::kSquare,   1, 0);
    // initial.blocks_[2] = Block(Shape::kVertical, 3, 0);
    // initial.blocks_[3] = Block(Shape::kVertical, 0, 2);
    // initial.blocks_[4] = Block(Shape::kHorizon,  1, 2);
    // initial.blocks_[5] = Block(Shape::kVertical, 3, 2);
    // initial.blocks_[6] = Block(Shape::kSingle,   1, 3);
    // initial.blocks_[7] = Block(Shape::kSingle,   2, 3);
    // initial.blocks_[8] = Block(Shape::kSingle,   0, 4);
    // initial.blocks_[9] = Block(Shape::kSingle,   3, 4);

    // int board[5][4] = {
    // //0  1  2  3
    // { 1, 2, 2, 3},   // 0
    // { 4, 2, 2, 3},   // 1
    // { 5, 6, 6, 7},   // 2
    // { 5, 8, 8, 9},   // 3
    // { 0,10,10, 0} }; // 4
    initial.blocks_[0] = Block(Shape::kSingle,   0, 0);
    initial.blocks_[1] = Block(Shape::kSquare,   1, 0);
    initial.blocks_[2] = Block(Shape::kVertical, 3, 0);
    initial.blocks_[3] = Block(Shape::kSingle,   0, 1);
    initial.blocks_[4] = Block(Shape::kVertical, 0, 2);
    initial.blocks_[5] = Block(Shape::kHorizon,  1, 2);
    initial.blocks_[6] = Block(Shape::kSingle,   3, 2);
    initial.blocks_[7] = Block(Shape::kHorizon,  1, 3);
    initial.blocks_[8] = Block(Shape::kSingle,   3, 3);
    initial.blocks_[9] = Block(Shape::kHorizon,  1, 4);

    queue.push_back(initial);
    seen.insert(initial.toMask());

    int i = 0;
    int count = 1;
    int temp_count = 0;

    while (!queue.empty())
    {
        State curr = queue.front();
        queue.pop_front();

        curr.counts = count;

        list_count_all_parent[curr.counts] = curr.parents;

        if (curr.isSolved())
        {

          temp_count = curr.counts;
          for(i=0; i<curr.step; i++)
          {
              printf("curr.counts = %d curr.parents = %d\r\n",
                    temp_count, list_count_all_parent[temp_count]);
              temp_count = list_count_all_parent[temp_count];
          }
          pcount[0] = curr.counts;
          pstep[0]  = curr.step;
          printf("found solution with %d steps counts = %d\r\n", curr.step, curr.counts);

          break;
        }
        else if (curr.step > 500)
        {
          printf("too many steps.\n");
          break;
        }

        count = count + 1;
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



int test_main_show(int list_count_sort[500])
{
    printf("sizeof(Mask) = %zd, sizeof(State) = %zd\n", sizeof(Mask), sizeof(State));
    std::unordered_set<Mask> seen;
    std::deque<State> queue;

    State initial;

    // int board[5][4] = {
    // //0  1  2  3
    // { 1, 2, 2, 3, },   // 0
    // { 1, 2, 2, 3, },   // 1
    // { 4, 5, 5, 6, },   // 2
    // { 4, 7, 8, 6, },   // 3
    // { 9, 0, 0, 10 } }; // 4
    // initial.blocks_[0] = Block(Shape::kVertical, 0, 0);
    // initial.blocks_[1] = Block(Shape::kSquare,   1, 0);
    // initial.blocks_[2] = Block(Shape::kVertical, 3, 0);
    // initial.blocks_[3] = Block(Shape::kVertical, 0, 2);
    // initial.blocks_[4] = Block(Shape::kHorizon,  1, 2);
    // initial.blocks_[5] = Block(Shape::kVertical, 3, 2);
    // initial.blocks_[6] = Block(Shape::kSingle,   1, 3);
    // initial.blocks_[7] = Block(Shape::kSingle,   2, 3);
    // initial.blocks_[8] = Block(Shape::kSingle,   0, 4);
    // initial.blocks_[9] = Block(Shape::kSingle,   3, 4);

    // int board[5][4] = {
    // //0  1  2  3
    // { 1, 2, 2, 3},   // 0
    // { 4, 2, 2, 3},   // 1
    // { 5, 6, 6, 7},   // 2
    // { 5, 8, 8, 9},   // 3
    // { 0,10,10, 0} }; // 4
    initial.blocks_[0] = Block(Shape::kSingle,   0, 0);
    initial.blocks_[1] = Block(Shape::kSquare,   1, 0);
    initial.blocks_[2] = Block(Shape::kVertical, 3, 0);
    initial.blocks_[3] = Block(Shape::kSingle,   0, 1);
    initial.blocks_[4] = Block(Shape::kVertical, 0, 2);
    initial.blocks_[5] = Block(Shape::kHorizon,  1, 2);
    initial.blocks_[6] = Block(Shape::kSingle,   3, 2);
    initial.blocks_[7] = Block(Shape::kHorizon,  1, 3);
    initial.blocks_[8] = Block(Shape::kSingle,   3, 3);
    initial.blocks_[9] = Block(Shape::kHorizon,  1, 4);

    queue.push_back(initial);
    seen.insert(initial.toMask());

    int i = 0;
    int j = 0;
    int count = 1;

    while (!queue.empty())
    {
        State curr = queue.front();
        queue.pop_front();

        curr.counts = count;

        for(i=0; i<5; i++)
        {
            for(j=0; j<4; j++)
            {
                hrd[i][j] = 0;
            }
        }
        for(i=0; i<10; i++)
        {
            //printf("%d %2d %2d \r\n",curr.blocks_[i].shape, curr.blocks_[i].left, curr.blocks_[i].top);
            hrd[curr.blocks_[i].top][curr.blocks_[i].left] = (int)curr.blocks_[i].shape;
            // kInvalid,
            // kSingle,
            // kHorizon,
            // kVertical,
            // kSquare,
            if(curr.blocks_[i].shape == Shape::kSingle)
            {
                hrd[curr.blocks_[i].top][curr.blocks_[i].left] = (int)Shape::kSingle;
            }
            if(curr.blocks_[i].shape == Shape::kHorizon)
            {
                hrd[curr.blocks_[i].top][curr.blocks_[i].left+1] = (int)Shape::kHorizon;
            }
            if(curr.blocks_[i].shape == Shape::kVertical)
            {
                hrd[curr.blocks_[i].top+1][curr.blocks_[i].left] = (int)Shape::kVertical;
            }
            if(curr.blocks_[i].shape == Shape::kSquare)
            {
                hrd[curr.blocks_[i].top][curr.blocks_[i].left+1] = (int)Shape::kSquare;
                hrd[curr.blocks_[i].top+1][curr.blocks_[i].left] = (int)Shape::kSquare;
                hrd[curr.blocks_[i].top+1][curr.blocks_[i].left+1] = (int)Shape::kSquare;
            }

        }
        // printf("curr.step = %d count = %d\r\n", curr.step, count);
        if(list_count_sort[curr.step] == curr.counts)
        {
            printf("curr.step = %d curr.counts = %d\r\n", curr.step, curr.counts);
            for(i=0; i<5; i++)
            {
                for(j=0; j<4; j++)
                {
                    printf("%2d ",hrd[i][j]);
                }
                printf("\r\n");
            }
            printf("\r\n");
        }

        if (curr.isSolved())
        {
          printf("found solution with %d steps count = %d\r\n", curr.step, count);

          break;
        }
        else if (curr.step > 500)
        {
          printf("too many steps.\n");
          break;
        }

        count = count + 1;
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

int test_list_sort(int list_count_all_parent[512000], int mcount, int mstep, int list_count_sort[500])
{
    int temp_sort = 0;
    int i = 0;
    int j = 0;

    printf("mcount = %d\r\n", mcount);
    temp_sort = mcount;
    list_count_sort[mstep] = temp_sort;
    for(j=mstep - 1; j>=0; j--)
    {
        //printf("temp_sort = %d\r\n", temp_sort);
        list_count_sort[j] = list_count_all_parent[temp_sort];
        temp_sort = list_count_all_parent[temp_sort];
    }

    printf("list_count_sort:\r\n");
    for(i = 0; i<mstep; i++)
    {
        printf("list_count_sort[%d] = %d\r\n", i, list_count_sort[i]);
    }

}

int main()
{
    int list_count_all_parent[512000] = {0};
    int list_count_sort[500] = {0};

    int mcount = 0;
    int mstep = 0;

    test_main(list_count_all_parent, &mcount, &mstep);

    test_list_sort(list_count_all_parent, mcount, mstep, list_count_sort);

    test_main_show(list_count_sort);
}




















