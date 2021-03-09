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
$begin drake_ode.cpp$$
$spell
    drake
$$

$section Drake Speed: Ode$$

$head Specifications$$
$cref link_ode$$

$head Implementation$$
// a drake version of this test is not yet implemented
$srccode%cpp% */
# include <map>

extern std::map<std::string, bool> global_option;
// see comments in main program for this external
extern size_t global_cppad_thread_alloc_inuse;

#include <drake/common/autodiff.h>
# include <cppad/utility/vector.hpp>
# include <cppad/cppad.hpp>
# include <cppad/speed/ode_evaluate.hpp>
# include <cppad/speed/uniform_01.hpp>
# include <cassert>


typedef drake::AutoDiffXd       ADScalar;
typedef CppAD::vector<ADScalar> ADVector;
namespace drake {

ADScalar fabs(ADScalar input) {
  return max(-input, input);
}
}

// list of possible options
extern std::map<std::string, bool> global_option;

bool link_ode(
    size_t                     size       ,
    size_t                     repeat     ,
    CppAD::vector<double>      &x         ,
    CppAD::vector<double>      &jacobian  )
{   global_cppad_thread_alloc_inuse = 0;

    // --------------------------------------------------------------------
    // check global options
    const char* valid[] = { "memory", "onetape", "optimize"};
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
    // --------------------------------------------------------------------
    // setup
    assert( x.size() == size );
    assert( jacobian.size() == size * size );

    size_t j;
    size_t p = 0;              // use ode to calculate function values
    size_t n = size;           // number of independent variables
    size_t m = n;              // number of dependent variables
    ADVector  X(n), Y(m);      // independent and dependent variables

    // -------------------------------------------------------------
    while(repeat--)
    {   // choose next x value
        uniform_01(n, x);
        Eigen::DenseIndex deriv_num = 0;
        for(j = 0; j < n; j++)
            X[j] = ADScalar(x[j], n, deriv_num++);

        // evaluate function
        CppAD::ode_evaluate(X, p, Y);

        // Copy out jacobian.
        for (int i = 0; i < size * size; i++) {
          int row = i / size;
          int col = i % size;
          jacobian[i] = Y[row].derivatives()(col);
        }
    }
    size_t thread                   = CppAD::thread_alloc::thread_num();
    global_cppad_thread_alloc_inuse = CppAD::thread_alloc::inuse(thread);
    return true;
}
/* %$$
$end
*/
