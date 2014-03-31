sc-friendlist
=============

## Intro
  
  This is my solution to finding 1 to n degree friends given a list of relationships.  

  The problem initially seems like a graph traversal problem for which we can do BFS using recursion and multithreading.  In order to take advantage of Hadoop distribution processing across machines and horizontal scaling (instead of coming up with my own messaging protocol solution between machines ) I will go with the map reduce approach.

  The solution I came up with requires multiple map reduce jobs chained together for each iteration of n.  From my quick research it seems the hadoop streaming API requires additional framework to allow chaining together of map reduce jobs.  For simplification I will test using the command line pipe method as suggested in the initial problem spec. 

  My solution can be tested like so:

  for 1 degree friends (1 mapper and 1 reducer is called)

  cat input.txt | ./init_mapper | ./reducer | sort -k1,1

  for 3 degree friends (3 mappers and 3 reducers are called)

  cat input.txt | ./init_mapper | ./inner_reducer | ./inner_mapper | ./inner_reducer | ./inner_mapper | ./reducer | sort -k1,1

  For this method of testing, the reducers here will just cout instead of accumulating in a global file.  When testing on the command line using pipes, the tasks aren't actually distributed and all data will have passed through the reducers, thus properly accumulating.  This is in place of all distributed data passing through to a global accumulator.   

  I am not sure how the different solutions of chaining together map reduce jobs using hadoop streaming API expects input and output to be but I assume the intermediate / inner reducers need to output data that another mapper can take as input, otherwise the inner mappers will need to read from global accumulation files, perhaps from from HDFS.  

  Please let me know if my assumption is incorrect and if my solution is insufficiently portable to a proper Hadoop chained map reduce solution.

## Solution

  In order to take advantage of the distributive power of hadoop we need to break down into separate data chunks that can be worked on in a pipeline.

  The problem can be mapped to represent a graph i.e. a node and its children nodes.  One way to marshal a node here for our purposes is like so - 

"name	1stdegfrnd,1stdegfrnd,1stdegfrnd,"

 - a tap separated pair of strings where the first string is a name and the second string is a csv string of names that are the first degree friends, or the children of this node.  

  It is also useful to track previously traversed nodes, captured like so - 

"name	1stdegfrnd	2nddegfrnd,3rddegfrnd,	4rthdegfrnd,4rthdegfrnd,"

 - where the first string is the current node name, the second string is the list of 1st degree friends (the children of the current node), the third string is the list of 1 to k-1 degree friends (previously traversed nodes), and the fourth string is the list of current k degree friends.  It is from these k degree friends that the next iteration will require children friends (the k+1 degree friends).

#### init_mapper

  Input is a list of tab separated pair of names.  Output is a list of marshalled nodes as described in the intro with only first degree friends.  This means the list of first degree friends will equal the list of previously traversed nodes will equal the list of current k=1 degree friends.

  Output can be read by a reducer.  

#### inner_mapper

  Input is a list of marshalled node listing up to k degree friends.  Output is the same list but traversed list will have been updated to include the k degree friends and the new k degree list will be the k+1 degree friends.

  Inner_mapper will split each line into an array of tab separated strings of length k (say, input[]).

  As we go through each line we keep track for each name the input[3] (the k degree friends we last found), input[1] (the children of each node ) and input[2] (the already traversed friends).  

  Then we must iterate back through the map and for each name n, traverse again through each k degree friend f and compile f's 1st degree friends (f's children nodes) in a new list, ignoring the ones already traversed.  This list will then become the k+1 degree list for n.

  Output can be read by an inner_reducer for another iteration of inner_mapper or final_reducer for final output.

#### inner_reducer

  Input is a marshalled node listing up to k degree friends.  Output is also a list of marshalled nodes of up to k degree friends.

  Inner_reducer will accumulate all kth degree friends for each name.  This system relies on inner_reducer to be run before each subsequent map step to ensure correctness at each step, and thus correct accumulation of previously traversed friends up until this point.

  For easier testing on command line using pipes, the reducer here will just cout instead of accumulating in a global file as explained in the intro.  

  Output can be read by another iteration of inner_mapper.

#### final_reducer

  Input is a marshalled node listing up to k degree friends.  Output is a list of lines where each line is a tab separated list of names, the first one being the name key followed by the 1 to k degree friends of that name in lexicographical order.  

  Final_reducer will accumulate 1 to k degree friends for each name (both the k degree list and the previously traversed list), sorting them as well.

  For easier testing on command line using pipes, final_reducer here will just cout instead of accumulating in a global file as explained in the intro.

## Analysis

Assuming 
1. |V| is the number of people (vertices) 
2. |E| is the number of friendships between two people (edges).
3. k is the number iterations so far (degree of friends we have accumulated)

For each program, |V| and |E| may be the distributed sub count of the total input.

#### Space Complexity

  The input is represented by an adjacency list that we transform into the marshalled node, which is used for most of the system. 

  The marshalled node representation tracks not only children, but all previously traversed nodes up to n (in my implementation, more than that, since I track children separately so this may actually end up being numtraversed + c).

  The worst case scenario is that everyone is friends with each other or *eventually* friends with each other after N degrees, and at that point each node will have traversed through each other node, and the space complexity ends up being O(|V|^2) - much like an adjacency matrix.  

  This space complexity is for each run of the mapper programs.  Since worse case scenario, each node's children includes every other node, the reducers also have O(|V|^2). 

#### Time Complexity

  The init_mapper does one traversal through the input adjacency list to construct the marshalled map / matrix.   Each friendship is represented once, so the time complexity is O(|E|).

  The rest of the programs take input as the list of marshalled nodes, which number |V|.  Inner_mapper traverses through each node and then again through each k degree friend.  The reducers traverse through each node's k degree friends as well in order to accumulate.  Worst case scenario every person is or eventually is friends with everyone else after some iterations, and has |V| friends to traverse from then on, and thus time complexity is O(|V|^2)


## Correctness

Correctness is ensured because of reduction between each map step.

Without reducing between map iterations we may miss edges in friendships.  for example, if one job has

	mary -> bob susan
	bob -> steve

and a second job has

	bob -> joe

the output for second degree friends without reducing first would yield

	mary -> bob susan steve

and mary would never get bob's friend joe.  

As long as we reduce between each map, we guarantee at each reduction step each person is mapped to all of their accumulated k degree friends, and not a subset.
