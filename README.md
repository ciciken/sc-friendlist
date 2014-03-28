sc-friendlist
=============

first pass naive method
========

init_mapper
====

  input is a list of tab separated pair of names.

  init_mapper will create a map of each unique name as key and a csv string of first degree friends as value.

  output is a list of tap separate pair of strings where first string is the namekey and the second string is a csv string of name key's first degree friends.

  output can be read by a reducer or inner_mapper to find next degree friends.

inner_mapper
====

  input is a list of tab separated pair of strings.  the first string is the name key and the second string is a friendlist represented by a csv string of k degree friends.

  inner_mapper will add k+1 degree friends to the friendlist for each name key.

  output follows the same rules as the input for k+1 degree friends.  it can be read by a reducer or inner_mapper again to find next degree friends.

reducer
====

  input is a list of tab separated pair of strings.  the first string is the name key and the second string is a friendlist represented by a csv string of k degree friends.

  reducer will accumulate friend lists in a local output file… need to look into best thread safe solution here.