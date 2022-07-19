#include "bool_sat.hh"

#include <bitset>
#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <vector>

string Bool_SAT::decimal_to_binary(unsigned int d, int num_bits) {
    string b = bitset<1024>(d).to_string();
    return b.substr(b.length() - num_bits, b.length() - 1);
}

string Bool_SAT::hex_to_binary(string h) {
    string b = "";

    for (int i = 0; i < h.length(); i++) {
        string bits;

        switch (toupper(h[i])) {
            case '0':
                bits = "0000";
                break;
            case '1':
                bits = "0001";
                break;
            case '2':
                bits = "0010";
                break;
            case '3':
                bits = "0011";
                break;
            case '4':
                bits = "0100";
                break;
            case '5':
                bits = "0101";
                break;
            case '6':
                bits = "0110";
                break;
            case '7':
                bits = "0111";
                break;
            case '8':
                bits = "1000";
                break;
            case '9':
                bits = "1001";
                break;
            case 'A':
                bits = "1010";
                break;
            case 'B':
                bits = "1011";
                break;
            case 'C':
                bits = "1100";
                break;
            case 'D':
                bits = "1101";
                break;
            case 'E':
                bits = "1110";
                break;
            case 'F':
                bits = "1111";
                break;
        }

        b += bits;
    }

    return b;
}

string Bool_SAT::binary_to_hex(string b) {
    string h = "";

    while (b.length() % 4 != 0) {
        b = '0' + b;
    }

    for (int i = 0; i < b.length(); i += 4) {
        string chunk = b.substr(i, 4);
        string hex_char;

        if (chunk.compare("0000") == 0)
            hex_char = '0';
        else if (chunk.compare("0001") == 0)
            hex_char = '1';
        else if (chunk.compare("0010") == 0)
            hex_char = '2';
        else if (chunk.compare("0011") == 0)
            hex_char = '3';
        else if (chunk.compare("0100") == 0)
            hex_char = '4';
        else if (chunk.compare("0101") == 0)
            hex_char = '5';
        else if (chunk.compare("0110") == 0)
            hex_char = '6';
        else if (chunk.compare("0111") == 0)
            hex_char = '7';
        else if (chunk.compare("1000") == 0)
            hex_char = '8';
        else if (chunk.compare("1001") == 0)
            hex_char = '9';
        else if (chunk.compare("1010") == 0)
            hex_char = 'A';
        else if (chunk.compare("1011") == 0)
            hex_char = 'B';
        else if (chunk.compare("1100") == 0)
            hex_char = 'C';
        else if (chunk.compare("1101") == 0)
            hex_char = 'D';
        else if (chunk.compare("1110") == 0)
            hex_char = 'E';
        else if (chunk.compare("1111") == 0)
            hex_char = 'F';

        h += hex_char;
    }

    return h;
}

vector<Node*> Bool_SAT::bin_str_to_node_vec(string bin_str) {
    vector<Node*> vec;

    for (int i = 0; i < bin_str.length(); i++) {
        if (bin_str[i] == '0')
            vec.push_back(&zero_node);
        else if (bin_str[i] == '1')
            vec.push_back(&one_node);
    }

    return vec;
}

vector<Node*> Bool_SAT::decimal_to_node_vec(unsigned int d, int num_bits) {
    return bin_str_to_node_vec(decimal_to_binary(d, num_bits));
}

Node* Bool_SAT::do_op(BoolOp op, Node* node1, Node* node2 /*= NULL*/) {
    if (node1 == NULL)
        return node2;
    else if (node2 == NULL && op != NOT)
        return node1;

    if (node1->type == LEAF && (node1->value == 0 || node1->value == 1))
        return simplify_constant(op, node1, node2);
    else if (op != NOT && node2->type == LEAF &&
             (node2->value == 0 || node2->value == 1))
        return simplify_constant(op, node2, node1);
    else if (op == NOT && node1->type == BRANCH && node1->op == NOT)
        return node1->left;

    if (op == XOR)
        return do_op(OR, do_op(AND, node1, do_op(NOT, node2)),
                     do_op(AND, do_op(NOT, node1), node2));
    else if (op == NAND)
        return do_op(NOT, do_op(AND, node1, node2));
    else if (op == NOR)
        return do_op(NOT, do_op(OR, node1, node2));
    else if (op == XNOR)
        return do_op(NOT, do_op(XOR, node1, node2));

    // OR, AND, or NOT
    Node* new_node = new Node;
    new_node->type = BRANCH;
    new_node->op = op;
    new_node->left = node1;
    new_node->right = node2;

    return new_node;
}

Node* Bool_SAT::simplify_constant(BoolOp op, Node* constant_node,
                                  Node* other_node) {
    switch (op) {
        case AND:
            if (constant_node->value == 0)
                return &zero_node;
            else
                return other_node;
        case OR:
            if (constant_node->value == 0)
                return other_node;
            else
                return &one_node;
        case XOR:
            if (constant_node->value == 0)
                return other_node;
            else
                return do_op(NOT, other_node);
        case NOT:
            if (constant_node->value == 0)
                return &one_node;
            else
                return &zero_node;
    }
}

Node* Bool_SAT::make_var() {
    Node* input_node = new Node;
    input_node->type = LEAF;
    input_node->value = next_var_id++;

    return input_node;
}

vector<Node*>& Bool_SAT::make_vars(int num) {
    vector<Node*>* vars = new vector<Node*>;

    for (int i = 0; i < num; i++) {
        vars->push_back(make_var());
    }

    return *vars;
}

vector<Node*> Bool_SAT::zeros(int num) {
    vector<Node*> vec;

    for (int i = 0; i < num; i++) {
        vec.push_back(&zero_node);
    }

    return vec;
}

vector<Node*> Bool_SAT::abstract_bitwise_op(BoolOp op, vector<Node*> a,
                                            vector<Node*> b) {
    vector<Node*> result;

    for (int i = 0; i < a.size(); i++) {
        result.push_back(do_op(op, a[i], b[i]));
    }

    return result;
}

vector<Node*> Bool_SAT::abstract_bitwise_op(BoolOp op, vector<Node*> a,
                                            Node* b) {
    vector<Node*> result;

    for (int i = 0; i < a.size(); i++) {
        result.push_back(do_op(op, a[i], b));
    }

    return result;
}

vector<Node*> Bool_SAT::abstract_not(vector<Node*> a) {
    for (int i = 0; i < a.size(); i++) {
        a[i] = do_op(NOT, a[i]);
    }

    return a;
}

vector<Node*> Bool_SAT::abstract_l_shift(vector<Node*> a, int shamt) {
    for (int i = 0; i < shamt; i++) {
        a.erase(a.begin());
        a.push_back(&zero_node);
    }

    return a;
}

vector<Node*> Bool_SAT::abstract_r_shift(vector<Node*> a, int shamt) {
    for (int i = 0; i < shamt; i++) {
        a.pop_back();
        a.insert(a.begin(), &zero_node);
    }

    return a;
}

vector<Node*> Bool_SAT::abstract_l_rotate(vector<Node*> a, int shamt) {
    for (int i = 0; i < shamt; i++) {
        a.push_back(a[0]);
        a.erase(a.begin());
    }

    return a;
}

vector<Node*> Bool_SAT::abstract_r_rotate(vector<Node*> a, int shamt) {
    for (int i = 0; i < shamt; i++) {
        a.insert(a.begin(), a[a.size() - 1]);
        a.pop_back();
    }

    return a;
}

vector<Node*> Bool_SAT::abstract_add(vector<Node*> a, vector<Node*> b) {
    vector<Node*> sum;
    Node* carry = &zero_node;

    for (int i = a.size() - 1; i >= 0; i--) {
        Node* a_xor_b = do_op(XOR, a[i], b[i]);
        Node* output_bit = do_op(XOR, a_xor_b, carry);
        carry = do_op(OR, do_op(AND, a[i], b[i]), do_op(AND, a_xor_b, carry));

        sum.insert(sum.begin(), output_bit);
    }

    return sum;
}

vector<Node*> Bool_SAT::abstract_multiply(vector<Node*> a, vector<Node*> b) {
    vector<Node*> product = zeros(a.size());
    int shamt = 0;

    for (int i = a.size() - 1; i >= 0; i--) {
        vector<Node*> l_shifted_a;

        if (shamt > 0)
            l_shifted_a = abstract_l_shift(a, shamt);
        else
            l_shifted_a = a;

        vector<Node*> partial_prod = abstract_bitwise_op(AND, a, b[i]);
        product = abstract_add(product, partial_prod);
        shamt++;
    }

    return product;
}

Node* Bool_SAT::constant_char_to_node(char constant) {
    if (constant == '0')
        return &zero_node;
    else if (constant == '1')
        return &one_node;
}

Node* Bool_SAT::combine_outputs_to_expr(vector<Node*> outputs,
                                        string target_output) {
    Node* result = NULL;

    // for (int i = outputs.size() - 1; i >= 0; i--) {
    for (int i = 0; i < outputs.size(); i++) {
        if (target_output[i] == '1')
            result = do_op(AND, result, outputs[i]);
        else if (target_output[i] == '0')
            result = do_op(AND, result, do_op(NOT, outputs[i]));
    }

    return result;
}

// Recursive function that checks for satisfiability of the given formula
bool Bool_SAT::bool_sat(Expression* expr) {
    clear_maps();

    // Base case: if the formula is a constant
    if (expr->vars.size() == 0) {
        while (expr->head->type != LEAF) {
            clear_maps();
            simplify_constants(expr->head);
        }

        return expr->head->value;
    }

    // Get the highest variable in the tree
    int var_id_to_sub = find_highest_var(expr->head, expr->vars);
    expr->vars.erase(expr->vars.begin());

    // Store tree nodes affected by the variable
    affected_by_var(expr->head, var_id_to_sub);

    // Create two copies of the tree: One with the variable set to 0, and 1 
    Node* copy = deep_copy_node(expr->head);
    expr->head = substitute_constant(expr->head, var_id_to_sub, &zero_node);
    copy = substitute_constant(copy, var_id_to_sub, &one_node);

    // OR the two copies together
    Node* new_node = new Node;
    new_node->type = BRANCH;
    new_node->op = OR;
    new_node->left = expr->head;
    new_node->right = copy;

    affected_by_var(new_node, var_id_to_sub);

    //Simplify constants
    do {
        any_const_simps = false;
        simplify_constants(new_node);
    } while (any_const_simps);

    expr->head = new_node;
    return bool_sat(expr);
}

string Bool_SAT::find_expr_inverse(Expression* expr) {
    string input2 = "";
    Node* head = expr->head;
    vector<Node*> vars = expr->vars;

    clear_maps();

    expr->head = full_deep_copy_node(head);
    expr->vars = vars;

    cout << "Checking for solution..." << endl;

    if (!bool_sat(expr)) {
        return "";
    }

    while (vars.size() > 0) {
        cout << vars.size() << " variables left" << endl;

        int var_id_to_check = vars[0]->value;
        vars.erase(vars.begin());
        clear_maps();

        expr->head = substitute_constant(full_deep_copy_node(head),
                                         var_id_to_check, &zero_node);
        expr->vars = vars;

        if (bool_sat(expr)) {
            input2 += '0';
            clear_maps();
            head = substitute_constant(head, var_id_to_check, &zero_node);
        } else {
            input2 += '1';
            clear_maps();
            head = substitute_constant(head, var_id_to_check, &one_node);
        }
    }

    return input2;
}

bool Bool_SAT::affected_by_var(Node* node, int var_id) {
    if (is_affected_by_var.find(node) != is_affected_by_var.end()) {
        return is_affected_by_var[node];
    }

    if (node->type == LEAF) {
        is_affected_by_var[node] = node->value == var_id;
        return node->value == var_id;
    }

    bool left_affected = affected_by_var(node->left, var_id);
    bool right_affected =
        node->op != NOT && affected_by_var(node->right, var_id);

    is_affected_by_var[node] = left_affected || right_affected;
    return left_affected || right_affected;
}

Node* Bool_SAT::deep_copy_node(Node* node) {
    if (!is_affected_by_var[node]) {
        return node;
    }

    if (already_copied.find(node) != already_copied.end()) {
        return already_copied[node];
    }

    Node* new_left = NULL;
    Node* new_right = NULL;

    if (node->type == BRANCH) {
        new_left = deep_copy_node(node->left);

        if (node->op != NOT) {
            new_right = deep_copy_node(node->right);
        }
    }

    Node* new_node = new Node;
    new_node->type = node->type;
    new_node->op = node->op;
    new_node->left = new_left;
    new_node->right = new_right;

    already_copied[node] = new_node;
    is_affected_by_var[new_node] = true;
    return new_node;
}

Node* Bool_SAT::full_deep_copy_node(Node* node) {
    if (already_copied.find(node) != already_copied.end()) {
        return already_copied[node];
    }

    Node* new_left = NULL;
    Node* new_right = NULL;

    if (node->type == BRANCH) {
        new_left = full_deep_copy_node(node->left);

        if (node->op != NOT) {
            new_right = full_deep_copy_node(node->right);
        }
    }

    Node* new_node = new Node;
    new_node->type = node->type;
    new_node->op = node->op;
    new_node->left = new_left;
    new_node->right = new_right;

    already_copied[node] = new_node;
    return new_node;
}

Node* Bool_SAT::substitute_constant(Node* node, int var_id, Node* constant) {
    if (is_affected_by_var.find(node) != is_affected_by_var.end() &&
        !is_affected_by_var[node]) {
        return node;
    }

    if (already_substituted[node]) {
        return node;
    }

    if (node->type == LEAF) {
        if (node->value == var_id)
            return constant;
        else
            return node;
    }

    already_substituted[node] = true;
    node->left = substitute_constant(node->left, var_id, constant);

    if (node->op != NOT) {
        node->right = substitute_constant(node->right, var_id, constant);
    }

    return node;
}

void Bool_SAT::simplify_constants(Node* node) {
    if (node->type == LEAF || already_simplified[node]) {
        return;
    }

    already_simplified[node] = true;
    simplify_constants(node->left);

    if (node->op == NOT) {
        if (node->left->type == LEAF) {
            if (node->left->value == 0) {
                node->type = LEAF;
                node->value = 1;
                any_const_simps = true;
            } else if (node->left->value == 1) {
                node->type = LEAF;
                node->value = 0;
                any_const_simps = true;
            }
        }
    } else {
        simplify_constants(node->right);
        Node* new_node;

        if (node->left->type == LEAF &&
            (node->left->value == 0 || node->left->value == 1)) {
            new_node = simplify_constant(node->op, node->left, node->right);
            node->type = new_node->type;
            node->op = new_node->op;
            node->left = new_node->left;
            node->right = new_node->right;
            any_const_simps = true;
        } else if (node->right->type == LEAF &&
                   (node->right->value == 0 || node->right->value == 1)) {
            new_node = simplify_constant(node->op, node->right, node->left);
            node->type = new_node->type;
            node->op = new_node->op;
            node->left = new_node->left;
            node->right = new_node->right;
            any_const_simps = true;
        }
    }
}

int Bool_SAT::find_highest_var(Node* node, vector<Node*> vars) {
    queue<Node*> q;
    q.push(node);

    while (q.size() > 0) {
        node = q.front();
        q.pop();

        if (node->type == LEAF) {
            if (node->value > 1)
                return node->value;
            else
                continue;
        } else {
            q.push(node->left);

            if (node->op != NOT) {
                q.push(node->right);
            }
        }
    }

    return vars[0]->value;
}

void Bool_SAT::clear_maps() {
    is_affected_by_var.clear();
    already_copied.clear();
    already_substituted.clear();
    already_simplified.clear();
    already_counted.clear();
    already_searched.clear();
}

int Bool_SAT::count_nodes(Node* node) {
    if (already_counted[node]) {
        return 1;
    }

    if (node->type == LEAF) {
        return 1;
    }

    already_counted[node] = true;
    int count = count_nodes(node->left);

    if (node->op != NOT) {
        count += count_nodes(node->right);
    }

    return count;
}

int Bool_SAT::count_dup_nodes(Node* node) {
    if (already_counted[node]) {
        return 1;
    }

    if (node->type == LEAF) {
        return 0;
    }

    already_counted[node] = true;
    int count = count_dup_nodes(node->left);

    if (node->op != NOT) {
        count += count_dup_nodes(node->right);
    }

    return count;
}

string Bool_SAT::tree_to_bool_expr(Node* head) {
    if (head->type == LEAF) {
        return to_string(head->value);
    }

    switch (head->op) {
        case AND:
            return "(" + tree_to_bool_expr(head->left) + ")(" +
                   tree_to_bool_expr(head->right) + ")";
        case OR:
            return tree_to_bool_expr(head->left) + " + " +
                   tree_to_bool_expr(head->right);
        case NOT:
            return "!(" + tree_to_bool_expr(head->left) + ")";
    }
}

void Bool_SAT::print_tree_as_bool_expr(Node* head) {
    cout << tree_to_bool_expr(head) << endl;
}