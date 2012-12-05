#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include <algorithm>
#include <vector>
#include <string>

using namespace std;

struct Node;
typedef Node Column;
struct Node
{
    Node* left;
    Node* right;
    Node* up;
    Node* down;
    Column* col;
    int col_idx;
    const char* name;
    int size;
};

const int kMaxNodes = 25 * 2 * 5 * 2;
const int kMaxColumns = 5 * 2 * 5;

struct Dance
{
  Column* root_;
  int*    inout_;
  Column* columns_[100];
  vector<Node*> stack_;
  Node    nodes_[kMaxNodes];
  int     cur_node_;
  int     cnt_;

  Column* new_column(int n = 0)
  {
    assert(cur_node_ < kMaxNodes);
    Column* c = &nodes_[cur_node_++];
    memset(c, 0, sizeof(Column));
    c->left = c;
    c->right = c;
    c->up = c;
    c->down = c;
    c->col = c;
    c->col_idx = n;
    return c;
  }

  void append_column(int n)
  {
    assert(columns_[n] == NULL);

    Column* c = new_column(n);
    put_left(root_, c);
    columns_[n] = c;
  }

  Node* new_row(int col)
  {
    assert(columns_[col] != NULL);
    assert(cur_node_ < kMaxNodes);

    Node* r = &nodes_[cur_node_++];

    memset(r, 0, sizeof(Node));
    r->left = r;
    r->right = r;
    r->up = r;
    r->down = r;
    r->col_idx = col;
    r->col = columns_[col];
    put_up(r->col, r);
    return r;
  }

  Dance()
    : inout_(NULL),
    cur_node_(0),
    cnt_(0)
  {
    stack_.reserve(100);

    root_ = new_column();
    root_->left = root_->right = root_;
    memset(columns_, 0, sizeof(columns_));
    setup();
  }

  Column* get_min_column()
  {
    Column* c = root_->right;
    int min_size = c->size;
    if (min_size > 1) {
      for (Column* cc = c->right; cc != root_; cc = cc->right) {
        if (min_size > cc->size) {
          c = cc;
          min_size = cc->size;
          if (min_size <= 1)
            break;
        }
      }
    }
    return c;
  }

  void cover(Column* c)
  {
    c->right->left = c->left;
    c->left->right = c->right;
    for (Node* row = c->down; row != c; row = row->down) {
      for (Node* j = row->right; j != row; j = j->right) {
        j->down->up = j->up;
        j->up->down = j->down;
        j->col->size--;
      }
    }
  }

  void uncover(Column* c)
  {
    for (Node* row = c->up; row != c; row = row->up) {
      for (Node* j = row->left; j != row; j = j->left) {
        j->col->size++;
        j->down->up = j;
        j->up->down = j;
      }
    }
    c->right->left = c;
    c->left->right = c;
  }

  void put_left(Column* old, Column* nnew)
  {
    nnew->left = old->left;
    nnew->right = old;
    old->left->right = nnew;
    old->left = nnew;
  }

  void put_up(Column* old, Node* nnew)
  {
    nnew->up = old->up;
    nnew->down = old;
    old->up->down = nnew;
    old->up = nnew;
    old->size++;
    nnew->col = old;
  }

  bool solve()
  {
    if (root_->left == root_) {
      printf("found %d '0123456789|0123456789|0123456789|0123456789|0123456789|'\n", ++cnt_);

      for (size_t i = 0; i < stack_.size(); ++i) {
        Node* n = stack_[i];
        printf(" node = '%s'\n", n->name);
      }
      return true;
    }

    Column* const col = get_min_column();
    cover(col);
    for (Node* row = col->down; row != col; row = row->down) {
      stack_.push_back(row);
      for (Node* j = row->right; j != row; j = j->right) {
        cover(j->col);
      }
      if (solve()) {
        // return true;
      }
      stack_.pop_back();
      for (Node* j = row->left; j != row; j = j->left) {
        uncover(j->col);
      }
    }
    uncover(col);
    return false;
  }

  void setup()
  {
    for (int i = 0; i < 5 * 2 * 5; ++i)
    {
      append_column(i);
    }

    const char* image[] =
    {
    //  Nation   | Color    | Drink    | Pet      | Cigar
    //  5 - Eng    5 - Red    5 - Tea    5 - Dog    5 - Pall Mall
    //  6 - Swe    6 - Green  6 - Cof    6 - Bird   6 - Dunhill
    //  7 - Dan    7 - Yellow 7 - Milk   7 - Horse  7 - Blends
    //  8 - Nor    8 - Blue   8 - Beer   8 - Cat    8 - Blue Master
    //  9 - Gem    9 - White  9 - Water  9 -        9 - Prince
    // 0123456789|0123456789|0123456789|0123456789|0123456789|0123456789|
   // "1    1    |          |          |          |          |",
   // "1     1   |          |          |          |          |",
   // "1      1  |          |          |          |          |",
      "1       1 | 1      1 |          |          |          |",
   // "1        1|          |          |          |          |",
      " 1   1    | 1   1    |          |          |          |",
      " 1    1   |          |          | 1   1    |          |",
      " 1     1  |          | 1   1    |          |          |",
      " 1      1 |          |          |          |          |",
      " 1       1|          |          |          | 1       1|",
      "  1  1    |  1  1    |          |          |          |",
      "  1   1   |          |          |  1  1    |          |",
      "  1    1  |          |  1  1    |          |          |",
      "  1     1 |          |          |          |          |",
      "  1      1|          |          |          |  1      1|",
      "   1 1    |   1 1    |          |          |          |",
      "   1  1   |          |          |   1 1    |          |",
      "   1   1  |          |   1 1    |          |          |",
      "   1    1 |          |          |          |          |",
      "   1     1|          |          |          |   1     1|",
      "    11    |    11    |          |          |          |",
      "    1 1   |          |          |    11    |          |",
      "    1  1  |          |    11    |          |          |",
      "    1   1 |          |          |          |          |",
      "    1    1|          |          |          |    1    1|",
      "          |1    1    |          |          |          |",
      "          |11    1  1|1     1   |          |          |",
      "          |1      1  |          | 1     1  |1     1   |",
      "          |1       1 |          |          |          |",
      "          |1        1|          |          |          |",
      "          | 1   1    |          |          |          |",
      "          | 11   1  1| 1    1   |          |          |",
      "          | 1     1  |          |1         | 1    1   |",
      "          | 1     1  |          |  1    1  | 1    1   |",
      "          | 1      1 |          |          |          |",
      "          | 1       1|          |          |          |",
      "          |  1  1    |          |          |          |",
      "          |  11  1  1|  1   1   |          |          |",
      "          |  1    1  |          | 1     1  |  1   1   |",
      "          |  1    1  |          |   1   1  |  1   1   |",
      "          |  1     1 |          |          |          |",
      "          |  1      1|          |          |          |",
      "          |   1 1    |          |          |          |",
      "          |   11 1  1|   1  1   |          |          |",
      "          |   1   1  |          |  1    1  |   1  1   |",
      "          |   1   1  |          |    1  1  |   1  1   |",
      "          |   1    1 |          |          |          |",
      "          |   1     1|          |          |          |",
      "          |    11    |          |          |          |",
   // "          |    1 1   |    1 1   |          |          |",
      "          |    1  1  |          |   1   1  |    1 1   |",
      "          |    1   1 |          |          |          |",
      "          |    1    1|          |          |          |",
      "          |          |1    1    |          |          |",
      "          |          |1     1   |          |          |",
   // "          |          |1      1  |          |          |",
      "          |          |1       1 |          |1       1 |",
      "          |          |1        1|          |          |",
      "          |          | 1   1    |          |          |",
      "          |          | 1    1   |          |          |",
   // "          |          | 1     1  |          |          |",
      "          |          | 1      1 |          | 1      1 |",
      "          |          | 1       1|          |          |",
      "          |          |  1  1    |          |          |",
      "          |          |  1   1   |          |          |",
      "          |          |  1    1  |          |          |",
      "          |          |  1     1 |          |          |",
      "          |          |  1      1|          |          |",
      "          |          |   1 1    |          |          |",
      "          |          |   1  1   |          |          |",
   // "          |          |   1   1  |          |          |",
      "          |          |   1    1 |          |   1    1 |",
      "          |          |   1     1|          |          |",
      "          |          |    11    |          |          |",
      "          |          |    1 1   |          |          |",
   // "          |          |    1  1  |          |          |",
      "          |          |    1   1 |          |    1   1 |",
      "          |          |    1    1|          |          |",
      "          |          |          |1    1    |          |",
      "          |          |          |1     1   |1    1    |",
      "          |          |          |1      1  |          |",
      "          |          |          |1       1 | 1     1  |",
      "          |          |          |1        1|          |",
      "          |          |          | 1   1    |          |",
      "          |          |          | 1    1   | 1   1    |",
      "          |          |          | 1     1  |          |",
      "          |          |          | 1      1 |1      1  |",
      "          |          |          | 1      1 |  1    1  |",
      "          |          |          | 1       1|          |",
      "          |          |          |  1  1    |          |",
      "          |          |          |  1   1   |  1  1    |",
      "          |          |          |  1    1  |          |",
      "          |          |          |  1     1 | 1     1  |",
      "          |          |          |  1     1 |   1   1  |",
      "          |          |          |  1      1|          |",
      "          |          |          |   1 1    |          |",
      "          |          |          |   1  1   |   1 1    |",
      "          |          |          |   1   1  |          |",
      "          |          |          |   1    1 |  1    1  |",
      "          |          |          |   1    1 |    1  1  |",
      "          |          |          |   1     1|          |",
      "          |          |          |    11    |          |",
      "          |          |          |    1 1   |    11    |",
      "          |          |          |    1  1  |          |",
      "          |          |          |    1   1 |   1   1  |",
      "          |          |          |    1    1|          |",
      "          |          |          |          |1    1    |",
      "          |          |          |          |1     1   |",
      "          |          |          |          |1      1  |",
      "          |          |          |          |1       1 |",
      "          |          |          |          |1        1|",
      "          |          |          |          | 1   1    |",
      "          |          |          |          | 1    1   |",
      "          |          |          |          | 1     1  |",
      "          |          |          |          | 1      1 |",
      "          |          |          |          | 1       1|",
      "          |          |          |          |  1  1    |",
      "          |          |          |          |  1   1   |",
      "          |          |          |          |  1    1  |",
      "          |          |          |          |  1     1 |",
      "          |          |          |          |  1      1|",
      "          |          |          |          |   1 1    |",
      "          |          |          |          |   1  1   |",
      "          |          |          |          |   1   1  |",
      "          |          |          |          |   1    1 |",
      "          |          |          |          |   1     1|",
      "          |          |          |          |    11    |",
      "          |          |          |          |    1 1   |",
      "          |          |          |          |    1  1  |",
      "          |          |          |          |    1   1 |",
      "          |          |          |          |    1    1|",
      NULL
    };

    for (int line = 0; image[line] != NULL; ++line) {
      std::string thisLine(image[line]);
      std::string::iterator it = std::remove(thisLine.begin(), thisLine.end(), '|');
      thisLine.erase(it, thisLine.end());
      const char* row = thisLine.c_str();
      const char* first = strchr(row, '1');
      assert(first != NULL);
      Node* n0 = new_row(first - row);
      n0->name = image[line];
      printf("\n%d: %d", line, first - row);
      const char* next = strchr(first+1, '1');
      while (next) {
        Node* n = new_row(next - row);
        n->name = image[line];
        printf(", %d", next - row);
        put_left(n0, n);
        next = strchr(next+1, '1');
      }
    }
    printf("\n");
  }
};

int main()
{
  Dance d;
  d.solve();
}
