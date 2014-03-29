sc-friendlist
=============

###### intro

  the problem initially seems like a graph traversal problem.  One approach is to use recursion where n is the depth of recursion (how far we traverse away from each node).  recursion lends itself to multi threading, but in order to scale across machines we will have to involve some protocol like a message bus for communication.  

  in order to take advantage of distributive power of hadoop we need to break down into separate data chunks that can be worked on in a pipeline.

  the problem can be mapped to represent a graph i.e. a node and its children nodes.  we can iterate on each children and each iteration can be broken down into a mapping job.  one way to marshal a node here for our purposes is like so

"curnameval	1stdegval1,1stdegval2,1stdegval3"

a tap separated pair of strings where the first string is a name and the second string is a csv string of names that are the first degree friends.  we also need to track continuous traversals, which can be done in the same line 

"curnameval	1stdegval1,1stdegval2	2nddegval1,2nddegval2,2nddegval3"

the first string is the current node value where k=0, and subsequent ++k string are csv of k degree values from the current node.

###### init_mapper

  input is a list of tab separated pair of names.  output is a list of marshalled nodes as described in the intro with only first degree friends.

  output can be read by a reducer.

  for correctness we will have to reduce before each mapping step

###### inner_mapper

  input is marshalled node listing up to k degree friends.  output is also a list of marshalled nodes but with one more tab separated csv string appended to it for the k+1 degree values.

  for each line, inner_mapper will split into an array of length k of tab separated strings input[].  input[0] is the name of the current person.  input[1] is a csv list of 1st degree friends.  input[k] is csv list of k degree friends.   

  as we go through each line we create a map between name and input[k] and input[1].  

  then we must traverse back through the map and for each name n, traverse again through each k degree friend f and compile f's 1st degree friends in a new list.  this list will then become the k+1 degree list for n.

  output can be read by an inner_reducer for another iteration of inner_mapper or just reducer for output.

###### inner_reducer

  input is a marshalled node listing up to k degree friends.  output is also a list of marshalled nodes of up to k degree friends.

  inner_reducer will accumulate all kth degree friends for each name.  it can just accumulate for the last k as this whole system relies on inner_reducer to be run before each subsequent map step to ensure correctness.

###### reducer

  input is a marshalled node listing up to k degree friends.  reducer will accumulate friend lists in several local output files so that each is a list of friends per one person.

###### final output

  the reducer accumulates the list of 1-k degree friends for each person in a separate global files.  to satisfy this problem, we can have a program to compile the final output in a ordered manner.

###### Analysis

  space complexity within each program is similar to BFS algorithm.

  time complexity within each program is also similar to that of a BFS algorithm.  that is, O(|V| + |E|) where |V| is the number of people nodes and |E| is the number of friendships.  

  correctness is ensured because of reduction between each map step.  