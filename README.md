A simple boolean satisfiability solver that implements a custom algorithm. 

The boolean expression is represented as a binary tree. For a given variable, the expression is duplicated for each possible value of the variable (0 or 1). These two expressions are OR'd back together into a single expression, constants are simplified, and duplicate branches are replaced with pointers to a single branch. This is repeated until all variables are replaced. Once this process is complete the final expression will reduce down to either 0 or 1, indicating if the boolean expression is satisfiable.

This also implements a basic inverse solver that can find all sastisfying inputs for a boolean expression using the SAT solver.
