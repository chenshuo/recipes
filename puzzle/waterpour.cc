#include <set>
#include <vector>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

struct State
{
  int big;
  int small;

  State(int b = 0, int s = 0)
    : big(b), small(s)
  {
  }

  bool operator==(const State& rhs) const
  {
    return big == rhs.big && small == rhs.small;
  }

  bool operator!=(const State& rhs) const
  {
    return !(*this == rhs);
  }

  bool operator<(const State& rhs) const
  {
    return big < rhs.big
        || (big == rhs.big && small < rhs.small);
  }
};

struct Step
{
  State st;
  int parent;
  Step(const State& s, int p)
    : st(s), parent(p)
  { }
};

// not used anymore
bool exist(const std::vector<Step>& steps, int curr, State next)
{
  while (curr >= 0)
  {
    assert(curr < steps.size());
    if (next == steps[curr].st)
      return true;
    else
      curr = steps[curr].parent;
  }
  return false;
}

void print(const std::vector<Step>& steps, int curr)
{
  while (curr >= 0)
  {
    printf("%d %d\n", steps[curr].st.big, steps[curr].st.small);
    curr = steps[curr].parent;
  }
}

int main(int argc, char* argv[])
{
  if (argc == 4)
  {
    const int kBig = atoi(argv[1]);
    const int kSmall = atoi(argv[2]);
    const State kFinal(atoi(argv[3]), 0);
    std::vector<Step> steps;
    std::set<State> seen;
    steps.push_back(Step(State(), -1));

    int curr = 0;
    while (steps[curr].st != kFinal)
    {
      assert(curr < steps.size());
      const int big = steps[curr].st.big;
      const int small = steps[curr].st.small;
      // printf("%d: %d %d\n", curr, big, small);

      assert(steps[curr].parent != curr);
      if (seen.insert(steps[curr].st).second)
      {
        if (big > 0)
        {
          State next(0, small);
          assert(steps[curr].st != next);
          // if (!exist(steps, curr, next))
          if (seen.find(next) == seen.end())
            steps.push_back(Step(next, curr));
        }

        if (small > 0)
        {
          State next(big, 0);
          assert(steps[curr].st != next);
          // if (!exist(steps, curr, next))
          if (seen.find(next) == seen.end())
            steps.push_back(Step(next, curr));
        }

        if (big < kBig)
        {
          State next(kBig, small);
          assert(steps[curr].st != next);
          // if (!exist(steps, curr, next))
          if (seen.find(next) == seen.end())
            steps.push_back(Step(next, curr));
        }

        if (small < kSmall)
        {
          State next(big, kSmall);
          assert(steps[curr].st != next);
          // if (!exist(steps, curr, next))
          if (seen.find(next) == seen.end())
            steps.push_back(Step(next, curr));
        }

        if (big > 0 && small < kSmall)
        {
          State next;
          int smallSpace = kSmall - small;
          if (big > smallSpace)
            next = State(big - smallSpace, small + smallSpace);
          else
            next = State(0, small + big);

          assert(big + small == next.big + next.small);
          assert(steps[curr].st != next);
          // if (!exist(steps, curr, next))
          if (seen.find(next) == seen.end())
            steps.push_back(Step(next, curr));
        }

        if (big < kBig && small > 0)
        {
          State next;
          int bigSpace = kBig - big;
          if (small > bigSpace)
            next = State(kBig, small - bigSpace);
          else
            next = State(small + big, 0);

          assert(big + small == next.big + next.small);
          assert(steps[curr].st != next);
          // if (!exist(steps, curr, next))
          if (seen.find(next) == seen.end())
            steps.push_back(Step(next, curr));
        }
      }
      else
      {
        // printf("seen\n");
      }

      if (++curr >= steps.size())
        break;
    }

    if (steps[curr].st == kFinal)
    {
      printf("Found! %zd %zd\n", steps.size(), seen.size());
      print(steps, curr);
    }
    else
    {
      printf("Not Found! %zd %zd\n", steps.size(), seen.size());
    }
  }
  else
  {
    printf("Usage: %s volumeA volumeB target\n", argv[0]);
  }
}
