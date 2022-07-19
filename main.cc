#include <algorithm>
#include <iostream>
#include <vector>
#include <cstdlib>

#include "bool_sat.hh"

using namespace std;

Expression* add_expression(Bool_SAT& sat, int input1, int output);

int main(int argc, char** argv) {
    if (argc != 3) {
        cout << "Pass a value for 'a' and a value for 'c' in the expression 'a + b = c'. Example: ./sat 2 5" << endl;
        return 1;
    }

    // a + b = c
    int a = atoi(argv[1]);
    int c = atoi(argv[2]);

    Bool_SAT sat;
    Expression* expr = add_expression(sat, a, c);
    string result = sat.find_expr_inverse(expr);

    if (result.length() > 0) {
        cout << "===== Result =====" << endl;
        cout << "b = " << result << endl;
    }
    else
        cout << "No solution!" << endl;

    return 0;
}

Expression* add_expression(Bool_SAT& sat, int input1, int output) {
    int num_bits = 25;
    vector<Node*> input1_bin = sat.decimal_to_node_vec(input1, num_bits);
    string output_bin = sat.decimal_to_binary(output, num_bits);
    vector<Node*> vars = sat.make_vars(num_bits);

    vector<Node*> add_expressions = sat.abstract_add(input1_bin, vars);

    Expression* expr = new Expression;
    expr->head = sat.combine_outputs_to_expr(add_expressions, output_bin);
    expr->vars = vars;

    cout << "Calculated expression" << endl;
    return expr;
}