(* Argument order and recursion: subtract reads a then b, left to right.
   countdown recurses with a READ_NUMBER inside its argument list. *)
FUN subtract(a, b) a - b;
FUN countdown(n, acc) IF n = 0 THEN acc ELSE countdown(n - 1, acc + subtract(READ_NUMBER(), READ_NUMBER())) FI;
? countdown(2, 0)
