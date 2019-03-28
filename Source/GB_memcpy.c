//------------------------------------------------------------------------------
// GB_memcpy: parallel memcpy
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// PARALLEL: break up a single large memcpy into smaller ones to do in parallel

#include "GB.h"

#define GB_CHUNK (1024*1024)

void GB_memcpy                  // parallel memcpy
(
    void *dest,                 // destination
    const void *src,            // source
    size_t n,                   // # of bytes to copy
    int nthreads                // # of threads to use
)
{

    if (nthreads <= 1 || n <= GB_CHUNK)
    {
        memcpy (dest, src, n) ;
    }
    else
    {
        nthreads = GB_IMIN (nthreads, n / GB_CHUNK) ;
        size_t nchunks = 1 + (n / GB_CHUNK) ;

        #pragma omp parallel for num_threads (nthreads) schedule (dynamic,1)
        for (size_t k = 0 ; k < nchunks ; k++)
        {
            size_t start = k * GB_CHUNK ;
            if (start < n)
            {
                size_t chunk = GB_IMIN (n - start, GB_CHUNK) ;
                memcpy (dest + start, src + start, chunk) ;
            }
        }
    }
}
