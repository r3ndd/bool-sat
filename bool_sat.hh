#include <map>
#include <string>
#include <vector>

using namespace std;

enum BoolOp { AND = 0, OR, NOT, XOR, NAND, NOR, XNOR };
enum NodeType { BRANCH = 0, LEAF };

struct Node {
    NodeType type;
    union {
        BoolOp op;
        int value;
    };
    Node* left;
    Node* right;
};

struct Expression {
    Node* head;
    vector<Node*> vars;
};

class Bool_SAT {
   public:
    Node zero_node = {LEAF, AND, NULL, NULL};
    Node one_node = {LEAF, OR, NULL, NULL};

    string decimal_to_binary(unsigned int d, int num_bits);
    string hex_to_binary(string h);
    string binary_to_hex(string b);
    vector<Node*> bin_str_to_node_vec(string bin_str);
    vector<Node*> decimal_to_node_vec(unsigned int d, int num_bits);
    Node* do_op(BoolOp op, Node* node1, Node* node2 = NULL);
    vector<Node*>& make_vars(int num);
    Node* constant_char_to_node(char constant);
    vector<Node*> abstract_bitwise_op(BoolOp op, vector<Node*> a, Node* b);
    vector<Node*> abstract_bitwise_op(BoolOp op, vector<Node*> a,
                                      vector<Node*> b);
    vector<Node*> abstract_not(vector<Node*> a);
    vector<Node*> abstract_l_shift(vector<Node*> a, int shamt);
    vector<Node*> abstract_r_shift(vector<Node*> a, int shamt);
    vector<Node*> abstract_l_rotate(vector<Node*> a, int shamt);
    vector<Node*> abstract_r_rotate(vector<Node*> a, int shamt);
    vector<Node*> abstract_add(vector<Node*> a, vector<Node*> b);
    vector<Node*> abstract_multiply(vector<Node*> a, vector<Node*> b);
    Node* combine_outputs_to_expr(vector<Node*> outputs, string target_output);
    bool bool_sat(Expression* expr);
    string find_expr_inverse(Expression* expr);
    void print_tree_as_bool_expr(Node* head);

   private:
    int next_var_id = 2;
    bool any_const_simps;
    map<Node*, bool> is_affected_by_var;
    map<Node*, Node*> already_copied;
    map<Node*, bool> already_substituted;
    map<Node*, bool> already_simplified;
    map<Node*, bool> already_counted;
    map<Node*, bool> already_searched;

    Node* make_var();
    vector<Node*> zeros(int num);
    Node* simplify_constant(BoolOp op, Node* constant_node, Node* other_node);
    bool affected_by_var(Node* node, int var_id);
    Node* deep_copy_node(Node* node);
    Node* full_deep_copy_node(Node* node);
    Node* substitute_constant(Node* node, int var_id, Node* constant);
    void simplify_constants(Node* node);
    int find_highest_var(Node* node, vector<Node*> vars);
    void clear_maps();
    int count_nodes(Node* head);
    int count_dup_nodes(Node* head);
    string tree_to_bool_expr(Node* head);
};