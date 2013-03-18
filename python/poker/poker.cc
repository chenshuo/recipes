#include <algorithm>
#include <map>
#include <vector>

#include <assert.h>
#include <stdio.h>

// CAUTION: unreadable code

struct Card
{
  int rank; // 2 .. 14 : 2 .. 9, T, J, Q, K, A
  int suit; // 1 .. 4

  Card() : rank(0), suit(0)
  {
  }
};

struct String
{
  char str[16];
};

struct Hand
{
  Card cards[5];

  String toString() const
  {
    String result;
    int idx = 0;
    for (int i = 0; i < 5; ++i)
    {
      result.str[idx++] = "0123456789TJQKA"[cards[i].rank];
      result.str[idx++] = " CDHS"[cards[i].suit];
      result.str[idx++] = ' ';
    }
    assert(idx == 15);
    result.str[14] = '\0';
    return result;
  }
};

struct Score
{
  int score;
  int ranks[5];
  Hand hand;

  bool operator<(const Score& rhs) const
  {
    return score < rhs.score
      || (score == rhs.score
          && std::lexicographical_compare(ranks, ranks+5, rhs.ranks, rhs.ranks+5));
  }
};

struct Group
{
  int count;
  int rank;
  bool operator<(const Group& rhs) const
  {
    return count > rhs.count
      || (count == rhs.count && rank > rhs.rank);
  }
};

void fillGroups(const int ranks[], Group groups[], int* len)
{
  int idx = -1;
  int last_rank = 0;
  for (int i = 0; i < 5; ++i)
  {
    if (ranks[i] == last_rank)
    {
      ++groups[idx].count;
    }
    else
    {
      ++idx;
      ++groups[idx].count;
      groups[idx].rank = last_rank = ranks[i];
    }
  }
  *len = idx+1;
  std::sort(groups, groups+5);
}

Score getScore(const Hand& hand)
{
  int ranks[5] = { 0, };
  bool flush = true;
  int suit = hand.cards[0].suit;
  for (int i = 0; i < 5; ++i)
  {
    ranks[i] = hand.cards[i].rank;
    flush = flush && suit == hand.cards[i].suit;
  }
  std::sort(ranks, ranks+5);

  // 'A' is 1 for straight A, 2, 3, 4, 5
  if (ranks[0] == 2
      && ranks[1] == 3
      && ranks[2] == 4
      && ranks[3] == 5
      && ranks[4] == 14)
  {
    ranks[0] = 1;
    ranks[1] = 2;
    ranks[2] = 3;
    ranks[3] = 4;
    ranks[4] = 5;
  }

  Group groups[5] = { { 0, },  };
  int group_len = 0;
  fillGroups(ranks, groups, &group_len);
  assert(group_len <= 5);
  bool straight = group_len == 5 && ranks[4] - ranks[0] == 4;
  /*
  for (int i = 0; i < group_len; ++i)
    printf("%d %d, ", groups[i].count, groups[i].rank);
  printf("\n");
  */

  int score = 0;
  if (group_len == 1)
    score = 9;
  else if (straight && flush)
    score = 8;
  else if (group_len == 2 && groups[0].count == 4)
    score = 7;
  else if (group_len == 2 && groups[0].count == 3)
    score = 6;
  else if (flush)
    score = 5;
  else if (straight)
    score = 4;
  else if (group_len == 3 && groups[0].count == 3)
    score = 3;
  else if (group_len == 3 && groups[0].count == 2)
    score = 2;
  else if (group_len == 4)
    score = 1;
  else
    assert(group_len == 5);

  Score result = { 0, };
  result.score = score;
  int idx = 0;
  for (int i = 0; i < group_len; ++i)
    for (int j = 0; j < groups[i].count; ++j)
      result.ranks[idx++] = groups[i].rank;
  assert(idx == 5);
  result.hand = hand;

  return result;
}

Hand formHand(int choose[])
{
  Hand hand;
  int c = 0;
  for (int i = 0; i < 52; ++i)
  {
    if (choose[i])
    {
      hand.cards[c].rank = i / 4 + 2;
      hand.cards[c].suit = i % 4 + 1;
      ++c;
      if (c == 5)
        break;
    }
  }
  assert(c == 5);
  return hand;
}

int main()
{
  int choose[52] = { 1, 1, 1, 1, 1, 0, };
  int count = 0;
  std::vector<Score> scores;
  do {
    Hand hand(formHand(choose));
    //puts(hand.toString().str);
    Score score = getScore(hand);
    scores.push_back(score);
    ++count;
  } while (std::prev_permutation(choose, choose + 52));
  std::sort(scores.begin(), scores.end());
  for (auto it = scores.rbegin(); it != scores.rend(); ++it)
    printf("((%d, [%d, %d, %d, %d, %d]), '%s')\n", it->score,
        it->ranks[0], it->ranks[1], it->ranks[2], it->ranks[3], it->ranks[4],
        it->hand.toString().str);
}
