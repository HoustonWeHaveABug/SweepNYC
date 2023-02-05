# SweepNYC
Solver for the Chinese Postman Problem (visit all edges in a mixed graph) and the New York Street Sweeper Problem (visit all arcs in a mixed graph).

Implements the solution described in page 5 of below document for both problems:

http://testoptimal.com/ref/GraphTheory%20Techniques%20In%20Model-Based%20Testing.pdf

For each node that has positive polarity, a BFS is performed to find the nearest node that has negative polarity and create the additional arcs between both nodes.

Once all nodes have a polarity equal to 0, a DFS is performed to visit all edges/arcs from selected starting node and complete a circuit (the search is exhaustive).

Posted as a solution to the Reddit Dailyprogrammer Hard Challenge #243: https://www.reddit.com/r/dailyprogrammer/comments/3vey01/20151204_challenge_243_hard_new_york_street/.

Sample input files are also provided - 'o' represents a node or a blocked edge. After the city data, two parameters must be specified:
- Type of problem to solve (0 for Chinese Postman Problem, 1 for New York Street Sweeper Problem)
- Number of choices at each node for DFS (1-4, 4 will perform a full search, otherwise the branching factor will be limited to the value specified)
