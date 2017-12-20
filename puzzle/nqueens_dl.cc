#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

// Dancing links algorithm by Donald E. Knuth
// www-cs-faculty.stanford.edu/~uno/papers/dancing-color.ps.gz

struct Node;
typedef Node Column;
struct Node
{
    Node* left;
    Node* right;
    Node* up;
    Node* down;
    Column* col;
    int name;
    int size;
};

const int kMaxQueens = 20;
const int kMaxColumns = kMaxQueens * 6;
const int kMaxNodes = 1 + kMaxColumns + kMaxQueens * kMaxQueens * 4;

class QueenSolver
{
 public:
  QueenSolver(int N)
    : root_(new_column())
  {
    assert(N <= kMaxQueens);
    root_->left = root_->right = root_;
    memset(columns_, 0, sizeof(columns_));

    // TODO: try organ pipe ordering
    for (int i = 0; i < N; ++i)
    {
      append_column(i);
      append_column(N+i);
    }
    for (int j = 0; j < 2*N; ++j)
    {
      int n = 2*N+j;
      assert(columns_[n] == NULL);
      Column* c = new_column(n);
      columns_[n] = c;
      n = 4*N+j;
      assert(columns_[n] == NULL);
      c = new_column(n);
      columns_[n] = c;
    }

    for (int col = 0; col < N; ++col)
    {
      for (int row = 0; row < N; ++row)
      {
        Node* n0 = new_row(col);
        Node* n1 = new_row(N+row);
        Node* n2 = new_row(2*N+row+col);
        Node* n3 = new_row(5*N+row-col);
        put_left(n0, n1);
        put_left(n0, n2);
        put_left(n0, n3);
      }
    }
  }

  void solve()
  {
    if (root_->left == root_) {
      ++count_;
      return;
    }
    Column* const col = get_min_column();
    cover(col);
    for (Node* row = col->down; row != col; row = row->down) {
      for (Node* j = row->right; j != row; j = j->right) {
        cover(j->col);
      }
      solve();
      for (Node* j = row->left; j != row; j = j->left) {
        uncover(j->col);
      }
    }
    uncover(col);
  }

  int64_t count() const { return count_; }

 private:
  int64_t count_ = 0;
  int     cur_node_ = 0;
  Column* root_;
  Column* columns_[kMaxColumns];
  Node    nodes_[kMaxNodes];

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
        c->name = n;
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
        r->name = col;
        r->col = columns_[col];
        put_up(r->col, r);
        return r;
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
};

int main(int argc, char* argv[])
{
  int N = argc > 1 ? atoi(argv[1]) : 8;
  QueenSolver solver(N);
  solver.solve();
  std::cout << solver.count() << '\n';
}
