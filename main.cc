#include <algorithm>
#include <iostream>
#include <vector>

#include "bool_sat.hh"

using namespace std;

Expression* sha256_expression(Bool_SAT& sat, string target_hash);
Expression* add_expression(Bool_SAT& sat, int input1, int output);

int main(int argc, char** argv) {
    // if (argc < 2) {
    //     cout << "You must enter a target hash." << endl;
    //     return 1;
    // }

    // Bool_SAT sat;
    // Expression* expr = sha256_expression(sat, argv[1]);
    // string result = sat.find_expr_inverse(expr);

    // if (result.length() > 0)
    //     cout << sat.binary_to_hex(result) << endl;
    // else
    //     cout << "No solution!" << endl;

    Bool_SAT sat;
    Expression* expr = add_expression(sat, 1, 2);
    string result = sat.find_expr_inverse(expr);

    if (result.length() > 0)
        cout << result << endl;
    else
        cout << "No solution!" << endl;

    return 0;
}

Expression* sha256_expression(Bool_SAT& sat, string target_hash) {
    target_hash = sat.hex_to_binary(target_hash);

    int num_bits = target_hash.length();
    vector<Node*> vars = sat.make_vars(num_bits);

    /*
    Initialize hash values:
    (first 32 bits of the fractional parts of the square roots of the first 8
    primes 2..19):
    */

    vector<Node*> h0 = sat.decimal_to_node_vec(0x6a09e667, 32);
    vector<Node*> h1 = sat.decimal_to_node_vec(0xbb67ae85, 32);
    vector<Node*> h2 = sat.decimal_to_node_vec(0x3c6ef372, 32);
    vector<Node*> h3 = sat.decimal_to_node_vec(0xa54ff53a, 32);
    vector<Node*> h4 = sat.decimal_to_node_vec(0x510e527f, 32);
    vector<Node*> h5 = sat.decimal_to_node_vec(0x9b05688c, 32);
    vector<Node*> h6 = sat.decimal_to_node_vec(0x1f83d9ab, 32);
    vector<Node*> h7 = sat.decimal_to_node_vec(0x5be0cd19, 32);

    /*
    Initialize array of round constants:
    (first 32 bits of the fractional parts of the cube roots of the first 64
    primes 2..311):
    */

    unsigned int k_decimal[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
        0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
        0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
        0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
        0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
        0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
        0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
        0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
        0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
    vector<Node*> k[64];

    for (int i = 0; i < 64; i++)
        k[i] = sat.decimal_to_node_vec(k_decimal[i], 32);

    /*
    Pre-processing (Padding):
    begin with the original message of length L bits
    append a single '1' bit
    append K '0' bits, where K is the minimum number >= 0 such that L + 1 + K +
    64 is a multiple of 512 append L as a 64-bit big-endian integer, making the
    total post-processed length a multiple of 512 bits
    */

    vector<Node*> inputs;

    for (int i = 0; i < num_bits; i++) {
        inputs.push_back(vars[i]);
    }

    inputs.push_back(&sat.one_node);

    for (int i = 0; i < 512 - 256 - 1 - 64; i++) {
        inputs.push_back(&sat.zero_node);
    }

    vector<Node*> message_bit_length = sat.decimal_to_node_vec(256, 64);

    for (int i = 0; i < 64; i++) {
        inputs.push_back(message_bit_length[i]);
    }

    /*
        Create a 64-entry message schedule array w[0..63] of 32-bit words
        (The initial values in w[0..63] don't matter, so many implementations
        zero them here) copy chunk into first 16 words w[0..15] of the message
        schedule array
    */

    vector<Node*> w[64];

    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 32; j++) {
            w[i].push_back(inputs[i * 32 + j]);
        }
    }

    /*
    Extend the first 16 words into the remaining 48 words w[16..63] of the
    message schedule array:
    */

    for (int i = 16; i < 64; i++) {
        vector<Node*> s0 = sat.abstract_bitwise_op(
            XOR, sat.abstract_r_rotate(w[i - 15], 7),
            sat.abstract_bitwise_op(XOR, sat.abstract_r_rotate(w[i - 15], 18),
                                    sat.abstract_r_shift(w[i - 15], 3)));
        vector<Node*> s1 = sat.abstract_bitwise_op(
            XOR, sat.abstract_r_rotate(w[i - 2], 17),
            sat.abstract_bitwise_op(XOR, sat.abstract_r_rotate(w[i - 2], 19),
                                    sat.abstract_r_shift(w[i - 2], 10)));
        w[i] = sat.abstract_add(
            w[i - 16], sat.abstract_add(s0, sat.abstract_add(w[i - 7], s1)));
    }

    /*
        Initialize working variables to current hash value:
    */

    vector<Node*> a = h0;
    vector<Node*> b = h1;
    vector<Node*> c = h2;
    vector<Node*> d = h3;
    vector<Node*> e = h4;
    vector<Node*> f = h5;
    vector<Node*> g = h6;
    vector<Node*> h = h7;

    /*
        Compression function main loop:
    */

    for (int i = 0; i < 64; i++) {
        vector<Node*> S1 = sat.abstract_bitwise_op(
            XOR, sat.abstract_r_rotate(e, 6),
            sat.abstract_bitwise_op(XOR, sat.abstract_r_rotate(e, 11),
                                    sat.abstract_r_rotate(e, 25)));
        vector<Node*> ch = sat.abstract_bitwise_op(
            XOR, sat.abstract_bitwise_op(AND, e, f),
            sat.abstract_bitwise_op(AND, sat.abstract_not(e), g));
        vector<Node*> temp1 = sat.abstract_add(
            h, sat.abstract_add(
                   S1, sat.abstract_add(ch, sat.abstract_add(k[i], w[i]))));
        vector<Node*> S0 = sat.abstract_bitwise_op(
            XOR, sat.abstract_r_rotate(a, 2),
            sat.abstract_bitwise_op(XOR, sat.abstract_r_rotate(a, 13),
                                    sat.abstract_r_rotate(a, 22)));
        vector<Node*> maj = sat.abstract_bitwise_op(
            XOR, sat.abstract_bitwise_op(AND, a, b),
            sat.abstract_bitwise_op(XOR, sat.abstract_bitwise_op(AND, a, c),
                                    sat.abstract_bitwise_op(AND, b, c)));
        vector<Node*> temp2 = sat.abstract_add(S0, maj);

        h = g;
        g = f;
        f = e;
        e = sat.abstract_add(d, temp1);
        d = c;
        c = b;
        b = a;
        a = sat.abstract_add(temp1, temp2);
    }

    h0 = sat.abstract_add(h0, a);
    h1 = sat.abstract_add(h1, b);
    h2 = sat.abstract_add(h2, c);
    h3 = sat.abstract_add(h3, d);
    h4 = sat.abstract_add(h4, e);
    h5 = sat.abstract_add(h5, f);
    h6 = sat.abstract_add(h6, g);
    h7 = sat.abstract_add(h7, h);

    h0.insert(h0.end(), h1.begin(), h1.end());
    h0.insert(h0.end(), h2.begin(), h2.end());
    h0.insert(h0.end(), h3.begin(), h3.end());
    h0.insert(h0.end(), h4.begin(), h4.end());
    h0.insert(h0.end(), h5.begin(), h5.end());
    h0.insert(h0.end(), h6.begin(), h6.end());
    h0.insert(h0.end(), h7.begin(), h7.end());

    Expression* expr = new Expression;
    expr->head = sat.combine_outputs_to_expr(h0, target_hash);
    expr->vars = vars;

    cout << "Calculated expression" << endl;
    return expr;
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