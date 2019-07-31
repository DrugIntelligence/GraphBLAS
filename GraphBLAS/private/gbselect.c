//------------------------------------------------------------------------------
// gbselect: select entries from a GraphBLAS matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// gbselect is an interface to GxB_select.

// Usage:

// Cout = gbselect (op, A, desc)
// Cout = gbselect (op, A, thunk, desc)

// Cout = gbselect (Cin, accum, op, A, desc)
// Cout = gbselect (Cin, accum, op, A, thunk, desc)

// Cout = gbselect (Cin, M, op, A, desc)
// Cout = gbselect (Cin, M, op, A, thunk, desc)

// Cout = gbselect (Cin, M, accum, op, A, desc)
// Cout = gbselect (Cin, M, accum, op, A, thunk, desc)

// If Cin is not present or is an empty matrix (Cin = [ ]) then it is
// implicitly a matrix with no entries, of the right size (which depends on A,
// and the descriptor).  If the accum operator is present, its type is
// the ztype of the accum.  Otherwise, C has the same time as A.

#include "gb_matlab.h"

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    gb_usage (nargin >= 3 && nargin <= 7 && nargout <= 1,
        "usage: Cout = gbselect (Cin, M, accum, op, A, thunk, desc)") ;

    //--------------------------------------------------------------------------
    // find the arguments
    //--------------------------------------------------------------------------

    GrB_Matrix C = NULL, M = NULL, A, thunk = NULL ;
    GrB_BinaryOp accum = NULL ;
    GxB_SelectOp op = NULL ;
    GrB_Type ctype ;
    bool kind_is_object = false ;
    GrB_Descriptor desc = 
        gb_mxarray_to_descriptor (pargin [nargin-1], &kind_is_object) ;

    if (mxIsChar (pargin [0]))
    {

        //----------------------------------------------------------------------
        // Cout = gbselect (op, A, desc)
        // Cout = gbselect (op, A, thunk, desc)
        //----------------------------------------------------------------------

        gb_usage (nargin == 3 || nargin == 4,
            "usage: Cout = gbselect (op, A, thunk, desc)") ;

        op = gb_mxstring_to_selectop (pargin [0]) ;
        A = gb_get_shallow (pargin [1]) ;
        thunk = (nargin > 3) ? (GxB_Scalar) gb_get_shallow (pargin [2]) : NULL ;

    }
    else if (mxIsChar (pargin [1]) && mxIsChar (pargin [2]))
    {
        GB_HERE ;

        //----------------------------------------------------------------------
        // Cout = gbselect (Cin, accum, op, A, desc)
        // Cout = gbselect (Cin, accum, op, A, thunk, desc)
        //----------------------------------------------------------------------

        gb_usage (nargin == 5 || nargin == 6,
            "usage: Cout = gbselect (Cin, accum, op, A, thunk, desc)") ;

        C = gb_get_deep (pargin [0], NULL) ;
        OK (GxB_Matrix_type (&ctype, C)) ;
        accum = gb_mxstring_to_binop (pargin [1], ctype) ;
        op = gb_mxstring_to_selectop (pargin [2]) ;
        A = gb_get_shallow (pargin [3]) ;
        thunk = (nargin > 5) ? (GxB_Scalar) gb_get_shallow (pargin [4]) : NULL ;

    }
    else if (mxIsChar (pargin [2]) && !mxIsChar (pargin [3]))
    {
        GB_HERE ;

        //----------------------------------------------------------------------
        // Cout = gbselect (Cin, M, op, A, desc)
        // Cout = gbselect (Cin, M, op, A, thunk, desc)
        //----------------------------------------------------------------------

        gb_usage (nargin == 5 || nargin == 6,
            "usage: Cout = gbselect (Cin, M, op, A, thunk, desc)") ;

        C = gb_get_deep (pargin [0], NULL) ;
        M = gb_get_shallow (pargin [1]) ;
        op = gb_mxstring_to_selectop (pargin [2]) ;
        A = gb_get_shallow (pargin [3]) ;
        thunk = (nargin > 5) ? (GxB_Scalar) gb_get_shallow (pargin [4]) : NULL ;

    }
    else if (mxIsChar (pargin [2]) && mxIsChar (pargin [3]))
    {
        GB_HERE ;

        //----------------------------------------------------------------------
        // Cout = gbselect (Cin, M, accum, op, A, desc)
        // Cout = gbselect (Cin, M, accum, op, A, thunk, desc)
        //----------------------------------------------------------------------

        gb_usage (nargin == 6 || nargin == 7,
            "usage: Cout = gbselect (Cin, M, accum, op, A, thunk, desc)") ;

        C = gb_get_deep (pargin [0], NULL) ;
        OK (GxB_Matrix_type (&ctype, C)) ;
        M = gb_get_shallow (pargin [1]) ;
        accum = gb_mxstring_to_binop (pargin [2], ctype) ;
        op = gb_mxstring_to_selectop (pargin [3]) ;
        A = gb_get_shallow (pargin [4]) ;
        thunk = (nargin > 6) ? (GxB_Scalar) gb_get_shallow (pargin [5]) : NULL ;

    }
    else
    {
        GB_HERE ;
        USAGE ("Cout = gbselect (Cin, M, accum, op, A, thunk, desc)") ;
    }

    //--------------------------------------------------------------------------
    // construct C if not present on input
    //--------------------------------------------------------------------------

    // If C is NULL, then it is either not present on input, or present but
    // passed in as Cin = [ ].  Construct C of the right size and type.

    if (C == NULL)
    {

        // get the descriptor contents to determine if A is transposed
        GrB_Desc_Value in0 ;
        OK (GxB_get (desc, GrB_INP0, &in0)) ;
        bool A_transpose = (in0 == GrB_TRAN) ;

        // get the size of A
        GrB_Index anrows, ancols ;
        OK (GrB_Matrix_nrows (&anrows, A)) ;
        OK (GrB_Matrix_ncols (&ancols, A)) ;

        // determine the size of C
        GrB_Index cnrows = (A_transpose) ? ancols : anrows ;
        GrB_Index cncols = (A_transpose) ? anrows : ancols ;

        // determine the type of C
        if (accum != NULL)
        {
            // if accum is present, use its ztype to determine the type of C
            OK (GxB_BinaryOp_ztype (&ctype, accum)) ;
        }
        else
        {
            // otherwise, C has the same type as A
            OK (GxB_Matrix_type (&ctype, A)) ;
        }

        // TODO all the user to determine the CSR/CSC format
        OK (GrB_Matrix_new (&C, ctype, cnrows, cncols)) ;
    }

    //--------------------------------------------------------------------------
    // compute C<M> += select (A, thunk)
    //--------------------------------------------------------------------------

    OK (GxB_select (C, M, accum, op, A, thunk, desc)) ;

    //--------------------------------------------------------------------------
    // free shallow copies
    //--------------------------------------------------------------------------

    OK (GrB_free (&M)) ;
    OK (GrB_free (&A)) ;
    OK (GrB_free (&thunk)) ;
    OK (GrB_free (&desc)) ;

    //--------------------------------------------------------------------------
    // export the output matrix C back to MATLAB
    //--------------------------------------------------------------------------

    // OK (GxB_Matrix_fprint (C, "C selected to export", 3, stdout)) ;
    pargout [0] = gb_export (&C, kind_is_object) ;
}

