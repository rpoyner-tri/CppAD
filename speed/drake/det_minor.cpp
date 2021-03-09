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
$begin drake_det_minor.cpp$$
$spell
    drake
$$

$section Drake Speed: Gradient of Determinant by Minor Expansion$$


$head Specifications$$
$cref link_det_minor$$

$head Implementation$$
// a drake version of this test is not yet implemented
$srccode%cpp% */
# include <map>

extern std::map<std::string, bool> global_option;
// see comments in main program for this external
extern size_t global_cppad_thread_alloc_inuse;

#include <drake/common/autodiff.h>

# include <cppad/speed/det_by_minor.hpp>
# include <cppad/speed/uniform_01.hpp>
# include <cppad/utility/vector.hpp>
# include <cppad/cppad.hpp>

// list of possible options
extern std::map<std::string, bool> global_option;

namespace {
    // typedefs
    typedef drake::AutoDiffXd       a_double;
    typedef CppAD::vector<a_double> a_vector;
    //
    // setup
    void setup(
        // inputs
        size_t size             ,
        // outputs
        CppAD::ADFun<double>& f )
    {
        // object for computing determinant
        CppAD::det_by_minor<a_double>   a_det(size);
        //
        // number of independent variables
        size_t nx = size * size;
        //
        // choose a matrix
        CppAD::vector<double> matrix(nx);
        CppAD::uniform_01(nx, matrix);
        //
        // copy to independent variables
        a_vector   a_A(nx);
        for(size_t j = 0; j < nx; ++j)
            a_A[j] = matrix[j];
        //
        // AD computation of the determinant
        a_vector a_detA(1);
        a_detA[0] = a_det(a_A);
    }

}

bool link_det_minor(
    const std::string&         job      ,
    size_t                     size     ,
    size_t                     repeat   ,
    CppAD::vector<double>     &matrix   ,
    CppAD::vector<double>     &gradient )
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
    // ---------------------------------------------------------------------
    //
    // AD function mapping matrix to determinant
    static CppAD::ADFun<double> static_f;
    //
    // size corresponding to static_f
    static size_t static_size = 0;
    //
    // number of independent variables
    size_t nx = size * size;
    using ADScalar = drake::AutoDiffXd;
    using ADVector = CppAD::vector<ADScalar>;
    ADVector   A(nx);        // AD domain space vector
    CppAD::det_by_minor<ADScalar> Det(size);
    //
    // CPPAD_ASSERT_UNKNOWN( job == "run" );
    while(repeat--)
    {   // get the next matrix
        CppAD::uniform_01(nx, matrix);
        // TODO: could do initializeAutoDiff here
        Eigen::DenseIndex deriv_num = 0;
        for( int i = 0; i < nx; i++)
          A[i] = ADScalar(matrix[i], nx, deriv_num++);

        ADScalar result = Det(A);
        if (result.derivatives().size() != nx) {
          throw std::runtime_error("bad derivatives size");
        }
        for (int i = 0; i < nx; i++) {
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
