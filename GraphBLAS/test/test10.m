clear all

rng ('default') ;

n = 6 ;
A = 100 * sprand (n, n, 0.5) ;
AT = A' ;
M = sparse (rand (n)) > 0.5 ;
Cin = sprand (n, n, 0.5) ;


Cout = gb.assign (Cin, A) ;
assert (isequal (A, sparse (Cout))) ;

Cout = gb.assign (Cin, A, { }, { }) ;
assert (isequal (A, sparse (Cout))) ;

Cout = gb.assign (Cin, M, A) ;
C2 = Cin ;
C2 (M) = A (M) ;
assert (isequal (C2, sparse (Cout))) ;

Cout = gb.assign (Cin, '+', A) ;
C2 = Cin + A ;
assert (isequal (C2, sparse (Cout))) ;

d.in0 = 'transpose' ;
Cout = gb.assign (Cin, M, A, d) ;
C2 = Cin ;
C2 (M) = AT (M) ;
assert (isequal (C2, sparse (Cout))) ;

Cout = gb.assign (Cin, '+', A, d) ;
C2 = Cin + AT ;
assert (isequal (C2, sparse (Cout))) ;

d.mask = 'complement' ;
Cout = gb.assign (Cin, M, A, d) ;
C2 = Cin ;
C2 (~M) = AT (~M) ;
assert (isequal (C2, sparse (Cout))) ;

I = [2 1 5] ;
J = [3 3 1 2] ;
B = sprandn (length (I), length (J), 0.5) ;
Cout = gb.assign (Cin, B, {I}, {J}) ;
C2 = Cin ;
C2 (I,J) = B ;
assert (isequal (C2, sparse (Cout))) ;

fprintf ('test10: all tests passed\n') ;
