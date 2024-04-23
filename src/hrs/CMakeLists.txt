cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HRS_SOURCES
	expected.hpp
	block.hpp
	mem_req.hpp
	debug.hpp
	scoped_call.hpp
	flags.hpp
	non_creatable.hpp
	iterator_for_each.hpp
	error.hpp
	instantiation.hpp
	free_block_chain_base.hpp
	sized_free_block_chain.hpp
	unsized_free_block_chain.hpp
	hint_cast_object.hpp)

set(HRS_MATH_SOURCES
	math/math_common.hpp
	math/vector_common.hpp
	math/vector.hpp
	math/vector_view.hpp
	math/matrix_common.hpp
	math/matrix.hpp
	math/matrix_view.hpp
	math/quaternion.hpp)

add_library(
	Hrs STATIC
		${HRS_SOURCES}
		${HRS_MATH_SOURCES})

set_target_properties(Hrs PROPERTIES LINKER_LANGUAGE CXX)
