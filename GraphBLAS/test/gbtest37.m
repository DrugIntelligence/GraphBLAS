function gbtest37
%GBTEST37 test istril, istriu, isbanded, isdiag, ishermitian, ...
% issymmetric, bandwith

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

nmax = 5 ;
for trial = 1:40
    fprintf ('.') ;

    for m = 1:nmax
        for n = 1:nmax
            A = sprand (m, n, 0.5) ;
            if (m == n)
                C = A+A' ;
            else
                C = A*A' ;
            end
            L = tril (A) ;
            U = triu (A) ;
            D = diag (diag (A)) ;

            GA = gb (A) ;
            GL = tril (GA) ;
            GU = triu (GA) ;
            GD = diag (diag (GA)) ;
            if (m == n)
                GC = GA + GA' ;
            else
                GC = GA * GA' ;
            end

            assert (isequal (A, double (GA))) ;
            assert (isequal (L, double (GL))) ;
            assert (isequal (U, double (GU))) ;
            assert (isequal (D, double (GD))) ;
            assert (isequal (C, double (GC))) ;

            assert (istril (A) == istril (GA)) ;
            assert (istril (L) == istril (GL)) ;
            assert (istril (U) == istril (GU)) ;
            assert (istril (D) == istril (GD)) ;
            assert (istril (C) == istril (GC)) ;

            assert (istriu (A) == istriu (GA)) ;
            assert (istriu (L) == istriu (GL)) ;
            assert (istriu (U) == istriu (GU)) ;
            assert (istriu (D) == istriu (GD)) ;
            assert (istriu (C) == istriu (GC)) ;

            assert (isdiag (A) == isdiag (GA)) ;
            assert (isdiag (L) == isdiag (GL)) ;
            assert (isdiag (U) == isdiag (GU)) ;
            assert (isdiag (D) == isdiag (GD)) ;
            assert (isdiag (C) == isdiag (GC)) ;

            assert (ishermitian (A) == ishermitian (GA)) ;
            assert (ishermitian (L) == ishermitian (GL)) ;
            assert (ishermitian (U) == ishermitian (GU)) ;
            assert (ishermitian (D) == ishermitian (GD)) ;
            assert (ishermitian (C) == ishermitian (GC)) ;

            assert (issymmetric (A) == issymmetric (GA)) ;
            assert (issymmetric (L) == issymmetric (GL)) ;
            assert (issymmetric (U) == issymmetric (GU)) ;
            assert (issymmetric (D) == issymmetric (GD)) ;
            assert (issymmetric (C) == issymmetric (GC)) ;

            assert (isequal (bandwidth (A), bandwidth (GA))) ;
            assert (isequal (bandwidth (L), bandwidth (GL))) ;
            assert (isequal (bandwidth (U), bandwidth (GU))) ;
            assert (isequal (bandwidth (D), bandwidth (GD))) ;
            assert (isequal (bandwidth (C), bandwidth (GC))) ;

            assert (bandwidth (A, 'lower') == bandwidth (GA, 'lower')) ;
            assert (bandwidth (L, 'lower') == bandwidth (GL, 'lower')) ;
            assert (bandwidth (U, 'lower') == bandwidth (GU, 'lower')) ;
            assert (bandwidth (D, 'lower') == bandwidth (GD, 'lower')) ;
            assert (bandwidth (C, 'lower') == bandwidth (GC, 'lower')) ;

            assert (bandwidth (A, 'upper') == bandwidth (GA, 'upper')) ;
            assert (bandwidth (L, 'upper') == bandwidth (GL, 'upper')) ;
            assert (bandwidth (U, 'upper') == bandwidth (GU, 'upper')) ;
            assert (bandwidth (D, 'upper') == bandwidth (GD, 'upper')) ;
            assert (bandwidth (C, 'upper') == bandwidth (GC, 'upper')) ;

            for lo = 0:nmax
                for hi = 0:nmax
                    assert (isbanded (A, lo, hi) == isbanded (GA, lo, hi)) ;
                    assert (isbanded (L, lo, hi) == isbanded (GL, lo, hi)) ;
                    assert (isbanded (U, lo, hi) == isbanded (GU, lo, hi)) ;
                    assert (isbanded (D, lo, hi) == isbanded (GD, lo, hi)) ;
                end
            end
        end
    end
end

fprintf ('\ngbtest37: all tests passed\n') ;
