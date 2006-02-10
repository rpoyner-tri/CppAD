// BEGIN SHORT COPYRIGHT
/* -----------------------------------------------------------------------
CppAD: C++ Algorithmic Differentiation: Copyright (C) 2003-06 Bradley M. Bell

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
------------------------------------------------------------------------ */
// END SHORT COPYRIGHT

/*
$begin Value.cpp$$
$spell
	Cpp
	cstddef
$$

$section Convert From AD to its Base Type: Example and Test$$

$index Value$$
$index example, Value$$
$index test, Value$$
$index record, example$$

$code
$verbatim%Example/Value.cpp%0%// BEGIN PROGRAM%// END PROGRAM%1%$$
$$

$end
*/
// BEGIN PROGRAM

# include <CppAD/CppAD.h>

bool Value(void)
{	bool ok = true;
	using CppAD::AD;
	using CppAD::Value;

	// independent variable vector
	size_t n = 2;
	CppADvector< AD<double> > x(n);
	x[0] = 3.;
	x[1] = 4.;

	// check value before recording
	ok &= (Value(x[0]) == 3.);
	ok &= (Value(x[1]) == 4.);

	// start recording
	CppAD::Independent(x);
	// cannot call Value here (tape is recording)

	// dependent variable vector 
	size_t m = 1;
	CppADvector< AD<double> > y(m);
	y[0] = - x[1];

	// create f: x -> y 
	CppAD::ADFun<double> f(x, y);
	// can call Value here (tape is no longer recording)

	// check value after recording
	ok &= (Value(x[0]) ==  3.);
	ok &= (Value(x[1]) ==  4.);
	ok &= (Value(y[0]) == -4.);

	return ok;
}
// END PROGRAM
