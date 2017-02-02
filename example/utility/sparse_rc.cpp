/* --------------------------------------------------------------------------
CppAD: C++ Algorithmic Differentiation: Copyright (C) 2003-17 Bradley M. Bell

CppAD is distributed under multiple licenses. This distribution is under
the terms of the
                    Eclipse Public License Version 1.0.

A copy of this license is included in the COPYING file of this distribution.
Please visit http://www.coin-or.org/CppAD/ for information on other licenses.
-------------------------------------------------------------------------- */

/*
$begin sparse_rc.cpp$$
$spell
	rc
$$

$section sparse_rc: Example and Test$$


$code
$srcfile%example/utility/sparse_rc.cpp%0%// BEGIN C++%// END C++%1%$$
$$

$end
*/

// BEGIN C++
# include <cppad/utility/sparse_rc.hpp>
# include <vector>

bool sparse_rc(void)
{	bool ok = true;
	typedef std::vector<size_t> SizeVector;

	// sparsity pattern for a 5 by 5 diagonal matrix
	CppAD::sparse_rc<SizeVector> pattern;
	size_t nr  = 5;
	size_t nc  = 5;
	size_t nnz = 5;
	pattern.resize(nr, nc, nnz);
	for(size_t k = 0; k < nnz; k++)
	{	size_t r = nnz - k - 1; // reverse or row-major order
		size_t c = nnz - k - 1;
		pattern.set(k, r, c);
	}

	// row and column vectors corresponding to pattern
	const SizeVector& row( pattern.row() );
	const SizeVector& col( pattern.row() );
	SizeVector row_major = pattern.row_major();

	// check row and column
	for(size_t k = 0; k < nnz; k++)
	{	ok &= row[ row_major[k] ] == k;
		ok &= col[ row_major[k] ] == k;
	}

	return ok;
}

// END C++