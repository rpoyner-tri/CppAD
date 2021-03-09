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
$begin drake_poly.cpp$$
$spell
    drake
$$

$section Drake Speed: Second Derivative of a Polynomial$$


$head Specifications$$
$cref link_poly$$

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
# include <cppad/utility/vector.hpp>

// list of possible options
extern std::map<std::string, bool> global_option;

bool link_poly(
    size_t                     size     ,
    size_t                     repeat   ,
    CppAD::vector<double>     &a        ,  // coefficients of polynomial
    CppAD::vector<double>     &z        ,  // polynomial argument value
    CppAD::vector<double>     &ddp      )  // second derivative w.r.t z
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
    // -----------------------------------------------------
    // setup
    typedef drake::AutoDiffXd       ADScalar;
    typedef CppAD::vector<ADScalar> ADVector;
    typedef Eigen::AutoDiffScalar<
      Eigen::Matrix<drake::AutoDiffXd, 1, 1>> AD2Scalar;
    typedef CppAD::vector<AD2Scalar> AD2Vector;

    size_t i;      // temporary index
    size_t m = 1;  // number of dependent variables
    size_t n = 1;  // number of independent variables
    ADVector Z(n); // AD domain space vector
    ADVector P(m); // AD range space vector
    AD2Vector Z2(n); // AD domain space vector
    AD2Vector P2(m); // AD range space vector

    // choose the polynomial coefficients
    CppAD::uniform_01(size, a);
    // for (i = 0; i < size; i++) {
    //   a[i] = 0;
    //   if (i == 2) {
    //     a[i] = 1;
    //   }
    // }

    // AD copy of the polynomial coefficients
    Eigen::DenseIndex deriv_num = 0;
    ADVector A(size);
    AD2Vector A2(size);
    for(i = 0; i < size; i++) {
      A[i] = ADScalar(a[i]);
      A2[i] = AD2Scalar(A[i]);
    }

    // --------------------------------------------------------------------
    while(repeat--)
    {
        // choose an argument value
        CppAD::uniform_01(1, z);
        Z[0] = ADScalar(z[0], n, deriv_num);
        assert(Z[0].derivatives().size() == n);
        // Z2[0] = ??
        Z2[0] = AD2Scalar(Z[0], n, deriv_num);
        // Z2[0].derivatives()(0).derivatives() = Z[0].derivatives();
        // assert(Z2[0].derivatives().size() == n);
        // assert(Z2[0].value().derivatives().size() == n);
        // assert(Z2[0].derivatives()(0).derivatives().size() == n);
        Z2[0].derivatives()(0).value() = 1;
        Z2[0].value().derivatives()(0) = 1;

        // AD computation of the function value
        // P[0] = CppAD::Poly(0, A, Z[0]);
        P2[0] = CppAD::Poly(0, A2, Z2[0]);
        assert(P2[0].derivatives().size() == n);
        if (P2[0].derivatives()(0).derivatives().size()) {
          ddp[0] = P2[0].derivatives()(0).derivatives()(0);
        } else {
          ddp[0] = 0.;
        }

        // TODO: port second derivative

        // // create function object f : A -> detA
        // f.Dependent(Z, P);

        // if( global_option["optimize"] )
        //     f.optimize(optimize_options);

        // // skip comparison operators
        // f.compare_change_count(0);

        // // pre-allocate memory for three forward mode calculations
        // f.capacity_order(3);

        // // evaluate the polynomial
        // p = f.Forward(0, z);

        // // evaluate first order Taylor coefficient
        // dp = f.Forward(1, dz);

        // // second derivative is twice second order Taylor coef
        // ddp     = f.Forward(2, ddz);
        // ddp[0] *= 2.;
    }
    size_t thread                   = CppAD::thread_alloc::thread_num();
    global_cppad_thread_alloc_inuse = CppAD::thread_alloc::inuse(thread);
    return true;
}
/* %$$
$end
*/
