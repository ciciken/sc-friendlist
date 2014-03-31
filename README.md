sc-friendlist
=============

## Intro
  
  This is my solution to finding 1 to n degree friends given an adjacency list of friendships.  I used c++ which was the fastest / most convenient dev env at my disposal.

#### Approach

  The problem initially seems like a graph traversal problem for which we can do BFS using recursion and multithreading.  In order to take advantage of Hadoop distribution processing across machines and horizontal scaling (instead of coming up with my own messaging protocol solution between machines ) I will go with the map reduce approach to fit with Hadoop.

  I will break down the steps into four programs: init_mapper, inner_mapper, inner_reducer, and final_reducer.  Each program is intended to be able to run in a distributed manner (some caveats with the reducers and accumulation as explained later).

  init_mapper will read the adjacency list input and output a matrix map of unique nodes (names) to each node's first degree friends.  The final_reducer can be run at this point to display the desired results.  

  For any subsequent iterations of n, you can run the inner_reducer and then inner_mapper.  inner_reducer takes in distributed outputs of init_mapper or inner_mapper and accumulates list of friends for each unique node.  inner_mapper will expand each node's list of friends to the next degree.

  The final_reducer will accumulate same as inner_reducer, but output a sorted list of unique friends from 1 to the latest degree.

#### Running it

  The solution I came up with requires multiple map reduce jobs chained together for each iteration of n.  From my quick research it seems the hadoop streaming API requires additional framework to allow chaining together of map reduce jobs.  For simplification I will test using the command line pipe method as suggested in the initial problem spec. 

  My solution can be tested like so:

  for 1 degree friends (1 mapper and 1 reducer is called)

	cat input.txt | ./init_mapper | ./reducer | sort -k1,1

  for 3 degree friends (3 mappers and 3 reducers are called)

	cat input.txt | ./init_mapper | ./inner_reducer | ./inner_mapper | ./inner_reducer | ./inner_mapper | ./reducer | sort -k1,1

  For this method of testing, I am not sure how to simulate reducing to a global accumulation file for each name.  Instead, I have the reducers simply cout their local reductions.  When testing on the command line using pipes, the tasks aren't actually distributed and all data will have passed through the reducers, thus properly accumulating.  This is in place of all distributed data passing through reducers to global accumulators and subsequent mappers reading from these global accumulators.

  My reducers right now won't work in accumulating distributed input, but again, I'm not sure how to get around this using pipes.

  I am not sure how the different solutions of chaining together map reduce jobs using hadoop streaming API handles input and output, but I think the intermediate reducers write to a global accumulator in HDFS which is then distributed out again to subsequent mappers.  Hopefully my understanding of this is correct and sufficient to overlook my workaround.

## Solution

  The problem can be mapped to represent a graph i.e. a node and its children nodes.  One way to marshal a node here for our purposes is like so - 

	name	1stdegfrnd	2nddegfrnd,3rddegfrnd,	4rthdegfrnd,4rthdegfrnd,

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
