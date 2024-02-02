/**
 * @author Phillip Rothenbeck (phillip.rothenbeck AT uni-jena.de)
 *
 * @section DESCRIPTION
 * Helper class for allocation
 **/
#ifndef TSUNAMI_LAB_ALLOCATOR_H
#define TSUNAMI_LAB_ALLOCATOR_H

#include <iostream>
#include <string>

#include "constants.h"

namespace tsunami_lab {
    t_real *aligned_alloc_real(t_idx i_size);
    t_idx *aligned_alloc_idx(t_idx i_size);
}  // namespace tsunami_lab

// allocator functions to help with aligned allocation (64 bit = 8 Byte cache line alligned)

tsunami_lab::t_real *tsunami_lab::aligned_alloc_real(t_idx i_size) {
    alignas(8) t_real *l_ptr = new t_real[i_size];
    return l_ptr;
}

tsunami_lab::t_idx *tsunami_lab::aligned_alloc_idx(t_idx i_size) {
    alignas(8) t_idx *l_ptr = new t_idx[i_size];
    return l_ptr;
}

#endif