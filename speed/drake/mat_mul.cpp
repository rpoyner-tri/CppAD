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
$begin drake_mat_mul.cpp$$
$spell
    drake
$$

$section Drake Speed: Matrix Multiplication$$


$head Specifications$$
$cref link_mat_mul$$

$head Implementation$$
// a drake version of this test is not yet implemented
$srccode%cpp% */
# include <map>

extern std::map<std::string, bool> global_option;
// see comments in main program for this external
extern size_t global_cppad_thread_alloc_inuse;

#include <drake/common/autodiff.h>

# include <cppad/cppad.hpp>
# include <cppad/speed/mat_sum_sq.hpp>
# include <cppad/speed/uniform_01.hpp>
# include <cppad/example/atomic_three/mat_mul.hpp>

// list of possible options
extern std::map<std::string, bool> global_option;

bool link_mat_mul(
    size_t                           size     ,
    size_t                           repeat   ,
    CppAD::vector<double>&           x        ,
    CppAD::vector<double>&           z        ,
    CppAD::vector<double>&           dz       )
{   global_cppad_thread_alloc_inuse = 0;

    // --------------------------------------------------------------------
    // check global options
    const char* valid[] = { "memory", "onetape", "optimize", "atomic"};
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
    // optimization options: no conditional skips or compare operators
    std::string optimize_options =
        "no_conditional_skip no_compare_op no_print_for_op";
    // -----------------------------------------------------
    // setup
    typedef drake::AutoDiffXd           ADScalar;
    typedef CppAD::vector<ADScalar>     ADVector;

    size_t j;               // temporary index
    size_t m = 1;           // number of dependent variables
    size_t n = size * size; // number of independent variables
    ADVector   X(n);        // AD domain space vector
    ADVector   Y(n);        // Store product matrix
    ADVector   Z(m);        // AD range space vector
    CppAD::ADFun<double> f; // AD function object

    // ------------------------------------------------------
    while (repeat--)
    {   // get the next matrix
        CppAD::uniform_01(n, x);
        Eigen::DenseIndex deriv_num = 0;
        for( j = 0; j < n; j++)
            X[j] = ADScalar(x[j], n, deriv_num++);

        mat_sum_sq(size, X, Y, Z);
        z[0] = Z[0].value();
        for (int i = 0; i < n; i++) {
          dz[i] = Z[0].derivatives()(i);
        }
    }
    size_t thread                   = CppAD::thread_alloc::thread_num();
    global_cppad_thread_alloc_inuse = CppAD::thread_alloc::inuse(thread);
    // --------------------------------------------------------------------
    // Free temporary work space (any future atomic_mat_mul constructors
    // would create new temporary work space.)
    CppAD::user_atomic<double>::clear();
    // --------------------------------------------------------------------

    return true;
}
/* %$$
$end
*/
