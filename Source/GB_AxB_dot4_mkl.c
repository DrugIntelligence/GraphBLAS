//------------------------------------------------------------------------------
// GB_AxB_dot4_mkl: compute c+=A*b where c and b are dense vectors
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This function is CSR/CSC agnostic, but the comments are writen as if
// all matrices are in CSR format, to match how MKL_graph views its matrices.

#define GB_DEBUG

#include "GB_mxm.h"
#include "GB_mkl.h"

#if GB_HAS_MKL_GRAPH

#define GB_MKL_FREE_WORK                            \
    GB_MKL_GRAPH_DESCRIPTOR_DESTROY (mkl_desc) ;    \
    GB_MKL_GRAPH_VECTOR_DESTROY (z_mkl) ;           \
    GB_MKL_GRAPH_MATRIX_DESTROY (A_mkl) ;           \
    GB_MKL_GRAPH_VECTOR_DESTROY (b_mkl) ;

#define GB_MKL_FREE_ALL                         \
    GB_MKL_FREE_WORK

GrB_Info GB_AxB_dot4_mkl            // c += A*b using MKL
(
    GrB_Vector c,                   // input/output vector (dense)
    const GrB_Matrix A,             // input matrix A
    const GrB_Vector B,             // input vector b (dense)
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;

    mkl_graph_descriptor_t mkl_desc = NULL ;
    mkl_graph_vector_t z_mkl = NULL ;
    mkl_graph_matrix_t A_mkl = NULL ;
    mkl_graph_vector_t b_mkl = NULL ;

    float *GB_RESTRICT Cx = (float *) c->x ;

    //--------------------------------------------------------------------------
    // get the semiring operators and types
    //--------------------------------------------------------------------------

    GrB_BinaryOp mult = semiring->multiply ;
    GrB_Monoid add = semiring->add ;
    ASSERT (mult->ztype == add->op->ztype) ;
    bool A_is_pattern, B_is_pattern ;
    GB_AxB_pattern (&A_is_pattern, &B_is_pattern, false, mult->opcode) ;

    GB_Opcode mult_opcode, add_opcode ;
    GB_Type_code xcode, ycode, zcode ;
    bool builtin_semiring = GB_AxB_semiring_builtin (A, A_is_pattern, B,
        B_is_pattern, semiring, false, &mult_opcode, &add_opcode, &xcode,
        &ycode, &zcode) ;
    ASSERT (builtin_semiring) ;

    ASSERT (xcode == GB_FP32_code) ;
    ASSERT (ycode == GB_FP32_code) ;
    ASSERT (zcode == GB_FP32_code) ;
    ASSERT (mult_opcode == GB_TIMES_opcode || mult_opcode == GB_SECOND_opcode) ;
    ASSERT (add_opcode == GB_PLUS_opcode) ;
    ASSERT (semiring == GrB_PLUS_TIMES_SEMIRING_FP32 ||
            semiring == GxB_PLUS_SECOND_FP32) ;

    //--------------------------------------------------------------------------
    // determine the MKL_graph semiring
    //--------------------------------------------------------------------------

    mkl_graph_semiring_t mkl_semiring = MKL_GRAPH_SEMIRING_PLUS_TIMES_FP32 ;

    //--------------------------------------------------------------------------
    // determine the # of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;

    //--------------------------------------------------------------------------
    // construct shallow copies of A and b
    //--------------------------------------------------------------------------

    int64_t n = B->vlen ;
    GB_MKL_OK (mkl_graph_vector_create (&b_mkl)) ;
    GB_MKL_OK (mkl_graph_vector_set_dense (b_mkl, n,
        B->x, MKL_GRAPH_TYPE_FP32)) ;

    GB_MKL_OK (mkl_graph_matrix_create (&A_mkl)) ;
    GB_MKL_OK (mkl_graph_matrix_set_csr (A_mkl, A->vdim, A->vlen,
        A->p, MKL_GRAPH_TYPE_INT64,
        A->i, MKL_GRAPH_TYPE_INT64,
        A->x, A_is_pattern ? MKL_GRAPH_TYPE_BOOL : GB_type_mkl (A->type->code)));

    //--------------------------------------------------------------------------
    // z=A*b via MKL
    //--------------------------------------------------------------------------

    printf ("calling MKL here, semiring %d\n", mkl_semiring) ;

    printf ("calling MKL mxv %d: %s.%s.%s\n",
        mkl_semiring,
        semiring->add->op->name,
        semiring->multiply->name,
        semiring->multiply->xtype->name) ;

    if (mult_opcode == GB_SECOND_opcode)
    {
        GB_MKL_OK (mkl_graph_descriptor_create (&mkl_desc)) ;
        GB_MKL_OK (mkl_graph_descriptor_set_field (mkl_desc,
            MKL_GRAPH_MODIFIER_FIRST_INPUT, MKL_GRAPH_ONLY_STRUCTURE)) ;
    }

    GB_MKL_OK (mkl_graph_vector_create (&z_mkl)) ;

    printf ("created z\n") ;
    double t = omp_get_wtime ( ) ;
    GB_MKL_OK (mkl_graph_mxv (z_mkl, NULL, MKL_GRAPH_ACCUMULATOR_NONE,
        mkl_semiring, A_mkl, b_mkl, mkl_desc,
        MKL_GRAPH_REQUEST_COMPUTE_ALL, MKL_GRAPH_METHOD_AUTO)) ;
    t = omp_get_wtime ( ) - t ;
    printf ("MKL time: %g\n", t) ;

    printf ("MKL claims to be successful: %s.%s.%s\n",
        semiring->add->op->name,
        semiring->multiply->name,
        semiring->multiply->xtype->name) ;

    //--------------------------------------------------------------------------
    // get the contents of z
    //--------------------------------------------------------------------------

    int64_t znrows = 0 ;
    mkl_graph_type_t Zx_type = 0 ;
    const float *GB_RESTRICT Zx = NULL ;
    GB_MKL_OK (mkl_graph_vector_get_dense (z_mkl, &znrows, &Zx, &Zx_type)) ;

    printf ("Zx %p\n", Zx) ;
    if (Zx == NULL || znrows != n)
    {
        // bug in mkl_graph_mxm: returns MKL_GRAPH_STATUS_SUCCESS even if
        // the semiring is not supported
        printf ("Hey, Z is NULL or znrows != n!\n") ;
        GB_MKL_FREE_ALL ;
        return (GrB_NO_VALUE) ;
    }

    if (Zx_type != GB_type_mkl (c->type->code))
    {
        GB_MKL_FREE_ALL ;
        return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
            "MKL returned result with wrong type."
            "Expected [%d], got [%d]\n",
            GB_type_mkl (c->type->code), Zx_type))) ;
    }

    //--------------------------------------------------------------------------
    // c += z
    //--------------------------------------------------------------------------

    GB_cblas_saxpy (n, 1.0, Zx, Cx, nthreads_max) ;

    //--------------------------------------------------------------------------
    // free MKL matrices z, A, and b
    //--------------------------------------------------------------------------

    GB_MKL_FREE_WORK ;

    ASSERT_VECTOR_OK (c, "mkl mxv result", GB2) ;

    return (info) ;
}

#endif

