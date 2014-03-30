sc-friendlist
=============

## Approach

  the problem initially seems like a graph traversal problem.  One approach is to use recursion where n is the depth of recursion (how far we traverse away from each node).  recursion lends itself to multi threading, but in order to scale across machines we will have to involve some protocol like a message bus for communication.  

  in order to take advantage of distributive power of hadoop we need to break down into separate data chunks that can be worked on in a pipeline.

  the problem can be mapped to represent a graph i.e. a node and its children nodes.  one way to marshal a node here for our purposes is like so

"name	1stdegfrnd,1stdegfrnd,1stdegfrnd,"

a tap separated pair of strings where the first string is a name and the second string is a csv string of names that are the first degree friends.  

it would also be useful to track previously traversed nodes, captured like so

"name	1stdegfrnd	2nddegfrnd,3rddegfrnd,	4rthdegfrnd,4rthdegfrnd,"

the first string is the current node name, the second string the list of 1st degree friends (the children of the current node), the third string is the list of 1 to k-1 degree friends (previously traversed nodes), and the fourth string is the list of current k degree friends that we are trying to find the children of (the k+1 degree friends).

#### init_mapper

  input is a list of tab separated pair of names.  output is a list of marshalled nodes as described in the intro with only first degree friends.  the list of first degree friends will equal the list of previously traversed nodes will equal the list of current k=1 degree friends whose children will be for next iteration.

  output can be read by a reducer.  

#### inner_mapper

  input is a list of marshalled node listing up to k degree friends.  output is the same list but for each line it will have an additional tab separated csv string appended to it for the k+1 degree value

  for each line, inner_mapper will split into an array of tab separated strings input[] of length k.  input[0] is the name of the current person.  input[1] is a csv list of 1st degree friends.  input[k] is csv list of k degree friends.   

  as we go through each line we create a map between name and input[k] and input[1].  

  then we must traverse back through the map and for each name n, traverse again through each k degree friend f and compile f's 1st degree friends (f's children nodes) in a new list.  this list will then become the k+1 degree list for n (the nodes to potentially traverse)

  output can be read by an inner_reducer for another iteration of inner_mapper or just reducer for final output.

#### inner_reducer

  input is a marshalled node listing up to k degree friends.  output is also a list of marshalled nodes of up to k degree friends.

  inner_reducer will accumulate all kth degree friends for each name.  it can just accumulate for the last k as this whole system relies on inner_reducer to be run before each subsequent map step to ensure correctness.

  for easier testing on command line using pipes, the reducer here will just cout instead of accumulating in a global file - simulating all data will have passed through this reducer.  assuming hadoop streaming chaining multiple map reduce map reduce jobs is out of this scope atm.

#### reducer

  input is a marshalled node listing up to k degree friends.  reducer will accumulate friend lists in several local output files so that each is a list of friends per one person.

#### final output

  the reducer accumulates the list of 1-k degree friends for each person in a separate global files.  to satisfy this problem, we can have a program to compile the final output in a ordered manner.

## Analysis

  space complexity within each program is similar to BFS algorithm.

  time complexity within each program is also similar to that of a BFS algorithm.  that is, O(|V| + |E|) where |V| is the number of people nodes and |E| is the number of friendships.  

  correctness is ensured because of reduction between each map step.  


## Correctness

Without reducing between map iterations we may miss edges in friendships.  for example, of one job has

	mary -> bob susan
	bob -> steve

and a second job has
	bob -> joe

the output for second degree friends without reducing in between for mary would be
	mary -> bob susan steve

and mary would never get bob's friend joe.  

As long as we reduce between each map, the input for the map is guaranteed to have the full k degree friend list.

## Notes

Again, I'm assuming hadoop streaming chaining multiple map reduce map reduce jobs is out of this scope atm.  The reduce jobs are assumed to have all data passed through them and thus 'correctly' accumulates and couts instead of writing to a global file.  Please let me know if I'm assuming wrong here.