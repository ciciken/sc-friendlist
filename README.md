sc-friendlist
=============

First Pass Naive Method
========

Mapper
===

    k os a number <= n.  n will be controlled outside of this program
    by how many times this program is called in the pipeline.

    input is a list where first token is the namekey amd each subsequent token
    represents friends at the k degree level where k is the index of the tab
    separated line.  these friend tokens may be a csv string representing many
    friends at the k degree or a single string at the k degree
    representing one friend.

    for each namekey, determine maxk and map a list of friends at the maxk+1
    degree. maxk is determined as last tab separated token.

    output is a list that matches the rules of the input.

    output can be read by a reducer or this mapper again to find next degree friends.

    reducer can take care of sorting and duplicates.


