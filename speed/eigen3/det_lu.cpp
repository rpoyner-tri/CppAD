/* --------------------------------------------------------------------------
CppAD: C++ Algorithmic Differentiation: Copyright (C) 2003-20 Bradley M. Bell

CppAD is distributed under the terms of the
             Eclipse Public License Version 2.0.

This Source Code may also be made available under the following
Secondary License when the conditions for such availability set forth
in the Eclipse Public License, Version 2.0 are satisfied:
      GNU General Public License, Version 2.0 or later.
---------------------------------------------------------------------------- */
/*
$begin eigen3_det_lu.cpp$$
$spell
    eigen3
    Lu
$$

$section Eigen3 Speed: Gradient of Determinant Using Lu Factorization$$


$head Specifications$$
$cref link_det_lu$$

$head Implementation$$
A eigen3 version of this test is not yet implemented
$srccode%cpp% */
# include <map>

extern std::map<std::string, bool> global_option;
// see comments in main program for this external
extern size_t global_cppad_thread_alloc_inuse;

#include <eigen3//Eigen/Core>
#include <eigen3/unsupported/Eigen/AutoDiff>

using AutoDiffXd = Eigen::AutoDiffScalar<Eigen::VectorXd>;

# include <cppad/speed/det_by_lu.hpp>
# include <cppad/speed/uniform_01.hpp>
# include <cppad/utility/vector.hpp>
# include <cppad/cppad.hpp>

// list of possible options
extern std::map<std::string, bool> global_option;

bool link_det_lu(
    size_t                     size     ,
    size_t                     repeat   ,
    CppAD::vector<double>     &matrix   ,
    CppAD::vector<double>     &gradient )
{   global_cppad_thread_alloc_inuse = 0;

    // --------------------------------------------------------------------
    // check global options
    const char* valid[] = { "memory", "optimize"};
    size_t n_valid = sizeof(valid) / sizeof(valid[0]);
    typedef std::map<std::string, bool>::iterator iterator;
    //
    for(iterator itr=global_option.begin(); itr!=global_option.end(); ++itr)
    {   if( itr->second )
        {   bool ok = false;
            for(size_t i = 0; i < n_valid; i++)
                ok |= itr->first == valid[i];
            if( ! ok )
                return false;
        }
    }
    // --------------------------------------------------------------------
    // optimization options:
    std::string optimize_options =
        "no_conditional_skip no_compare_op no_print_for_op";
    // -----------------------------------------------------
    // setup
    using ADScalar = AutoDiffXd;
    using ADVector = CppAD::vector<ADScalar>;
    CppAD::det_by_lu<ADScalar> Det(size);

    size_t i;               // temporary index
    size_t m = 1;           // number of dependent variables
    size_t n = size * size; // number of independent variables
    ADVector   A(n);        // AD domain space vector

    // ------------------------------------------------------
    while(repeat--)
    {   // get the next matrix
        CppAD::uniform_01(n, matrix);
        Eigen::DenseIndex deriv_num = 0;
        for( i = 0; i < n; i++)
          A[i] = ADScalar(matrix[i], n, deriv_num++);

        // AD computation of the determinant
        ADScalar result = Det(A);
        if (result.derivatives().size() != n) {
          throw std::runtime_error("bad derivatives size");
        }
        for (i = 0; i < n; i++) {
          gradient[i] = result.derivatives()(i);
        }
    }
    size_t thread                   = CppAD::thread_alloc::thread_num();
    global_cppad_thread_alloc_inuse = CppAD::thread_alloc::inuse(thread);
    return true;
}
/* %$$
$end
*/
