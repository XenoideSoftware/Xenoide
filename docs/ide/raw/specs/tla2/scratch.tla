---- MODULE scratch ----
EXTENDS Integers, TLC, Sequences, FiniteSets

SecondsPerMinute == 60

(* This is an Operator that takes 1 parameters *)
MinutesToSeconds(m) == SecondsPerMinute * m

Abs(x) == IF x < 0 THEN -x ELSE x

(*
    TLA+ Doesn't support floats. To model systems 
    that work with them, they should be abstracted
 *)

(*  Booleans

= -> Test for Equality
# -> test for Inequality

Operators (Math symbols, not programming symbols)
A /\ B  -> And
A \/ B  -> Or
A ~ B   -> Not
A => B  -> A implies B (equivalent to ~A \/ B)


Bullet Point notation: It allows to rewrite expressions, from

A /\ (B \/ C) /\ (D \/ (E /\ F))

To

/\ A
/\ \/ B
   \/ C
/\ \/ D
   \/ /\ E
      /\ F


 *)
Xor(A, B) == A = ~B




(*
Sequences: The same as arrays in other languages, indexed (starting from 1)

S == << elem1, elem2, ..., elemM >>


- Tail, Head
- Len
- \o (merges two sequences)
Eval == S \o <<"b", "c">>
*)



(*
Sets: Holds a collection of unordered and unique values. They can't be indexed, and all of them must have the same data type

S == { elem1, elem2, ..., elemN}

Operators:
A \intersect B: 
A \union B
A \ B (difference)

Extending FiniteSets provide the Cardinality operator.

To check if a set is empty, do the following: S = {}

\X Is the operator for the Cartesian product operator.
*)


S == {0, 2, 1}

ClockType == (0..23) \X (0..59) \X (0..59)

(* These as well *)
Eval == S[1]
Init == PrintT(Eval)
Next == FALSE
====
