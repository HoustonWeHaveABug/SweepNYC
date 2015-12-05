# SweepNYC
New York Street Sweeper Problem solver (visit all edges in a directed graph).

Implements the solution described in page 5 of below document:

http://testoptimal.com/ref/GraphTheory%20Techniques%20In%20Model-Based%20Testing.pdf

For each node that has positive polarity, a BFS is performed to find the nearest node that has negative polarity and create the additional edges between both nodes.

Once all nodes have a polarity equal to 0, a DFS is performed to visit all edges from selected starting node and complete a cycle.

Posted as solution to the Reddit Dailyprogrammer Hard Challenge #243 (though the problem is lightly different):

https://www.reddit.com/r/dailyprogrammer/comments/3vey01/20151204_challenge_243_hard_new_york_street/

Sample input files also provided ('o' represents a node or a blocked edge).
