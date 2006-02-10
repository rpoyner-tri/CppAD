# ifndef CppADValueIncluded
# define CppADValueIncluded

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

/*
$begin Value$$
$spell
	inline
	const
$$

$index Value$$

$index Base, from AD$$
$index AD, convert to Base$$
$index convert, AD to Base$$

$section Convert From AD to its Base Type$$

$table
$bold Syntax$$ $cnext 
$syntax%%b% = Value(%x%)%$$
$tend

$fend 20$$

$head Purpose$$
Returns the 
$xref/glossary/Base Type/base type/$$ value corresponding to $italic x$$.

$head x$$
The argument $italic x$$ has prototype
$syntax%
	const AD<%Base%> &%x%
%$$

$head b$$
The return value $italic b$$ has prototype
$syntax%
	%Base% %b%
%$$

$head Restrictions$$
This function cannot be called while the tape that records
$syntax%AD<%Base%>%$$ is in the Recording
$xref/glossary/Tape State/state/$$.

$head Example$$
$children%
	Example/Value.cpp
%$$
The file
$xref/Value.cpp/$$
contains an example and a test of this operation.

$end
-------------------------------------------------------------------------------
*/

//  BEGIN CppAD namespace
namespace CppAD {

template <class Base>
inline Base Value(const AD<Base> &x) 
{	Base result;

	CppADUsageError(
		AD<Base>::Tape()->State() != Recording,
		"cannot use Value while the tape is recording"
	);
		

	result = x.value;

	return result;
}

}
//  END CppAD namespace


# endif
