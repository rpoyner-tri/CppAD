/* --------------------------------------------------------------------------
CppAD: C++ Algorithmic Differentiation: Copyright (C) 2003-21 Bradley M. Bell

CppAD is distributed under the terms of the
             Eclipse Public License Version 2.0.

This Source Code may also be made available under the following
Secondary License when the conditions for such availability set forth
in the Eclipse Public License, Version 2.0 are satisfied:
      GNU General Public License, Version 2.0 or later.
---------------------------------------------------------------------------- */
# include <cppad/core/llvm_ir.hpp>
//
# include <llvm/IR/DerivedTypes.h>
# include <llvm/IR/InstIterator.h>
# include <llvm/IR/IRBuilder.h>
# include <llvm/IR/LegacyPassManager.h>
# include <llvm/IR/Type.h>
# include <llvm/IR/Verifier.h>
//
# include <llvm/Support/FileSystem.h>
# include <llvm/Support/Host.h>
# include <llvm/Support/raw_os_ostream.h>
# include <llvm/Support/TargetRegistry.h>
# include <llvm/Support/TargetSelect.h>
//
# include <llvm/Target/TargetOptions.h>
# include <llvm/Target/TargetMachine.h>
//
//
# include <llvm/Transforms/InstCombine/InstCombine.h>
# include <llvm/Transforms/Scalar.h>
# include <llvm/Transforms/Scalar/GVN.h>
/*
---------------------------------------------------------------------------
$begin llvm_ir_from_graph$$
$spell
    llvm_ir
    obj
    vec
    op
$$

$section Convert a C++ AD Graph to LLVM Intermediate Representation$$

$head Syntax$$
$icode%msg% = %ir_obj%.from_graph(%graph_obj%$$

$head Prototype$$
$srcthisfile%0%// BEGIN_FROM_GRAPH%// END_FROM_GRAPH%1%$$

$head graph_obj$$
This a the $cref cpp_ad_graph$$ corresponding the function
that is converted to llvm intermediate representation.

$subhead Restrictions$$
The following limitations are placed on $icode graph_obj$$
(and expected to be removed in the future).
$list number$$
$cref/function_name/cpp_ad_graph/function_name/$$ must not be empty.
$lnext
$cref/discrete_name_vec/cpp_ad_graph/discrete_name_vec/$$ must be empty.
$lnext
$cref/atomic_name_vec/cpp_ad_graph/atomic_name_vec/$$ must be empty.
$lnext
$cref/print_text_vec/cpp_ad_graph/print_text_vec/$$ must be empty.
$lnext
Only the following operators my appear in
$cref/operator_vec/cpp_ad_graph/operator_vec/$$:
$code add_graph_op$$.
$lend

$head ir_obj$$
Its input value of this $cref/llvm_ir/llvm_ir_ctor/$$ object does not matter.
Upon return, it contains an LLVM Intermediate Representation (IR)
corresponding to the function in $icode graph_obj$$.

$head msg$$
If the return value $icode msg$$ is the empty string,
no error was detected.
Otherwise, $icode msg$$ describes the error and the return value
of $icode ir_obj$$ is unspecified.

$children%
    example/llvm/from_to_graph.cpp
%$$
$head Example$$
The file $cref llvm_from_to_graph.cpp$$ contains an example and test
of this operation.

$end
*/
namespace CppAD { // BEGIN_CPPAD_NAMESPACE

// BEGIN_FROM_GRAPH
std::string llvm_ir::from_graph(const CppAD::cpp_graph&  graph_obj)
// END_FROM_GRAPH
{   //
    // initialize return valuse as an identifier of this routines
    std::string msg = "llvm_ir::from_graph: ";
    using std::string;
    using CppAD::graph::graph_op_enum;
    //
    // Assumptions
    if( graph_obj.discrete_name_vec_size() != 0)
    {   msg += "graph_obj.discrete_name_vec_size() != 0";
        return msg;
    }
    if( graph_obj.atomic_name_vec_size() != 0)
    {   msg += "graph_obj.atomic_name_vec_size() != 0";
        return msg;
    }
    if( graph_obj.print_text_vec_size() != 0)
    {   msg += "graph_obj.print_text_vec_size() != 0";
        return msg;
    }
    //
    // scalar values
    n_dynamic_ind_     = graph_obj.n_dynamic_ind_get();
    n_variable_ind_    = graph_obj.n_variable_ind_get();
    n_variable_dep_    = graph_obj.dependent_vec_size();
    size_t n_constant  = graph_obj.constant_vec_size();
    size_t n_operator  = graph_obj.operator_vec_size();
    //
    // function_name_
    function_name_ = graph_obj.function_name_get();
    if( function_name_ == "" )
    {   msg += "graph_obj.function_name_get() is empty";
        return msg;
    }
    //
    // context_ir_
    context_ir_ = std::make_unique<llvm::LLVMContext>();
    //
    // module_ir_
    module_ir_ = std::make_unique<llvm::Module>("test", *context_ir_);
    //
    // llvm_double
    llvm::Type* llvm_double = llvm::Type::getDoubleTy(*context_ir_);
    //
    // llvm_double_ptr
    llvm::PointerType* llvm_double_ptr =
        llvm::PointerType::getUnqual(llvm_double);
    //
    // int_32_t
    llvm::Type* int_32_t = llvm::Type::getInt32Ty(*context_ir_);
    //
    // function_t
    // void (*function_t) (double *, double*)
    std::vector<llvm::Type*> param_types = {
        int_32_t, llvm_double_ptr, int_32_t, llvm_double_ptr
    };
    bool                     is_var_arg  = false;
    llvm::Type*              result_type = int_32_t;
    llvm::FunctionType*      function_t  = llvm::FunctionType::get(
            result_type, param_types, is_var_arg
    );
    //
    // function_ir
    // Create the IR function entry and insert this entry into the module
    auto            addr_space     = llvm::Function::ExternalLinkage;
    llvm::Function *function_ir    = llvm::Function::Create(
        function_t, addr_space, function_name_, module_ir_.get()
    );
    //
    // Make sure there are four arguments
    CPPAD_ASSERT_UNKNOWN(
        function_ir->arg_begin() + 4  == function_ir->arg_end()
    );
    //
    // len_input
    llvm::Argument* len_input  = function_ir->arg_begin() + 0;
    len_input->setName("len_input");
    //
    // input_ptr
    llvm::Argument* input_ptr  = function_ir->arg_begin() + 1;
    input_ptr->setName("input_ptr");
    //
    // len_output
    llvm::Argument* len_output  = function_ir->arg_begin() + 2;
    len_output->setName("len_output");
    //
    // output_ptr
    llvm::Argument* output_ptr  = function_ir->arg_begin() + 3;
    output_ptr->setName("output_ptr");
    //
    // Add a basic block at entry point to the function.
    llvm::BasicBlock* basic_block = llvm::BasicBlock::Create(
        *context_ir_, "EntryBlock", function_ir
    );
    //
    // Create a basic block builder with default parameters.  The builder will
    // automatically append instructions to the basic block `basic_block'.
    llvm::IRBuilder<> builder(basic_block);
    //
    // graph_ir
    // Initialize with nothing at index zero
    std::vector<llvm::Value*> graph_ir;
    graph_ir.push_back(nullptr);
    //
    // index_vector
    std::vector<llvm::Value*> index_vector(1);
    // ----------------------------------------------------------------------
    // check for error in len_input or len_output
    // ----------------------------------------------------------------------
    // error_len_input
    size_t n_input = n_dynamic_ind_ + n_variable_ind_;
    llvm::Value* expected_len_input = llvm::ConstantInt::get (
        *context_ir_, llvm::APInt(32, n_input, true)
    );
    llvm::Value* error_len_input = builder.CreateICmpNE(
        len_input, expected_len_input, "error_len_input"
    );
    // error_len_output
    llvm::Value* expected_len_output = llvm::ConstantInt::get (
        *context_ir_, llvm::APInt(32, n_variable_dep_, true)
    );
    llvm::Value* error_len_output = builder.CreateICmpNE(
        len_output, expected_len_output, "error_len_output"
    );
    // error_len
    llvm::Value* error_len = builder.CreateOr(
        error_len_input, error_len_output, "error_len"
    );
    // error_no
    // convert to boolean value error_len 32 bit signed integer
    llvm::Value* error_no = builder.CreateZExtOrTrunc(
            error_len, int_32_t, "error_no"
    );
    // error_bb, merge_bb
    llvm::BasicBlock* error_bb =
        llvm::BasicBlock::Create(*context_ir_, "error_bb");
    llvm::BasicBlock* merge_bb =
        llvm::BasicBlock::Create(*context_ir_, "merge_bb");
    //
    // if error_len, return error_no
    builder.CreateCondBr(error_len, error_bb, merge_bb);
    function_ir->getBasicBlockList().push_back(error_bb);
    builder.SetInsertPoint(error_bb);
    builder.CreateRet(error_no);
    function_ir->getBasicBlockList().push_back(merge_bb);
    builder.SetInsertPoint(merge_bb);
    // ------------------------------------------------------------------------
    // graph_ir
    // independent parameters
    for(size_t i = 0; i < n_dynamic_ind_; ++i)
    {   index_vector[0] = llvm::ConstantInt::get(
            *context_ir_, llvm::APInt(32, i, false)
        );
        string name        = "p_" + std::to_string(i);
        llvm::Value* ptr   = builder.CreateGEP(
            llvm_double, input_ptr, index_vector
        );
        llvm::Value* value = builder.CreateLoad(llvm_double, ptr, name);
        graph_ir.push_back(value);
    }
    //
    // graph_ir
    // independent variables
    for(size_t i = 0; i < n_variable_ind_; ++i)
    {   index_vector[0] = llvm::ConstantInt::get(
            *context_ir_, llvm::APInt(32, n_dynamic_ind_ + i, false)
        );
        string name        = "x_" + std::to_string(i);
        llvm::Value* ptr   = builder.CreateGEP(
            llvm_double, input_ptr, index_vector
        );
        llvm::Value* value = builder.CreateLoad(llvm_double, ptr, name);
        graph_ir.push_back(value);
    }
    //
    // graph_ir
    // constants
    for(size_t i = 0; i < n_constant; ++i)
    {   double c_i = graph_obj.constant_vec_get(i);
        llvm::Value* value = llvm::ConstantFP::get(
            *context_ir_, llvm::APFloat(c_i)
        );
        graph_ir.push_back(value);
    }
    // graph_ir
    // operators in the graph
    CppAD::cpp_graph::const_iterator itr;
    for(size_t iop = 0; iop < n_operator; ++iop)
    {   if( iop == 0 )
            itr = graph_obj.begin();
        else
            ++itr;
        CppAD::cpp_graph::const_iterator::value_type itr_value = *itr;
        //
        const CppAD::vector<size_t>&       arg( *itr_value.arg_node_ptr );
        graph_op_enum  op_enum  = itr_value.op_enum;
# ifndef NDEBUG
        const CppAD::vector<size_t>& str_index( *itr_value.str_index_ptr );
        size_t         n_result = itr_value.n_result;
        size_t         n_arg    = arg.size();
        size_t         n_str    = str_index.size();
# endif
        //
        llvm::Value* value;
        switch( op_enum )
        {
            case CppAD::graph::add_graph_op:
            CPPAD_ASSERT_UNKNOWN( n_arg == 2 );
            CPPAD_ASSERT_UNKNOWN( n_result == 1);
            CPPAD_ASSERT_UNKNOWN( n_str == 0 );
            value = builder.CreateFAdd( graph_ir[arg[0]], graph_ir[arg[1]] );
            graph_ir.push_back(value);
            break;

            default:
            msg += "graph_obj has following unsupported operator ";
            msg += local::graph::op_enum2name[op_enum];
            return msg;
        }
    }
    // set dependent variable values
    for(size_t i = 0; i < n_variable_dep_; ++i)
    {
        bool is_signed = false;
        index_vector[0] = llvm::ConstantInt::get(
            *context_ir_, llvm::APInt(32, i, is_signed)
        );
        string name        = "y_" + std::to_string(i);
        llvm::Value* ptr   = builder.CreateGEP(
            llvm_double, output_ptr, index_vector
        );
        size_t node_index  = graph_obj.dependent_vec_get(i);
        bool is_volatile   = false;
        builder.CreateStore(graph_ir[node_index], ptr, is_volatile);
        graph_ir[node_index]->setName(name);
    }
    // return zero for no error
    builder.CreateRet(error_no);
    //
    // check retreiving this function from this module
    CPPAD_ASSERT_UNKNOWN(
        function_ir == module_ir_->getFunction(function_name_)
    );
    // Validate the generated code, checking for consistency
    bool found_error = llvm::verifyFunction(*function_ir);
    if( found_error )
    {   msg += "error during verification of llvm_ir function";
        return msg;
    }
    //
    // No error
    msg = "";
    return msg;
}

} // END_CPPAD_NAMESPACE