FUN add(x, y) x + y; (* simple functions*)
FUN math(x, y, z) add(add(x, y), z);
(* goal expression *)
?math (READ_NUMBER(), READ_NUMBER(), READ_NUMBER())