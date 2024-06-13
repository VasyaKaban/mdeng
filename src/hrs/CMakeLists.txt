cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(Hrs STATIC)

target_sources(
Hrs
    PRIVATE
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
		hint_cast_object.hpp
		enum_meta.hpp
		resource.hpp
		stacktrace.hpp
		demangle.hpp
		dynamic_library.hpp
		variadic.hpp
)

if(UNIX)
	target_sources(
	Hrs
	    PRIVATE
		    demangle_impl/ia64_abi/demangle.h
			demangle_impl/ia64_abi/demangle.cpp
			stacktrace_impl/unwind/frame.h
			stacktrace_impl/unwind/frame.cpp
			stacktrace_impl/unwind/stacktrace.h
			stacktrace_impl/unwind/stacktrace.cpp
			dynamic_library_impl/dl/dynamic_library.h
			dynamic_library_impl/dl/dynamic_library.cpp
	)
elseif(WIN32)
	target_sources(
	Hrs
	    PRIVATE
		    demangle_impl/winapi/demangle.h
			demangle_impl/winapi/demangle.cpp
			stacktrace_impl/winapi/frame.h
			stacktrace_impl/winapi/frame.cpp
			stacktrace_impl/winapi/stacktrace.h
			stacktrace_impl/winapi/stacktrace.cpp
			dynamic_library_impl/winapi/dynamic_library.h
			dynamic_library_impl/winapi/dynamic_library.cpp
	)
endif()


target_sources(
Hrs
    PRIVATE
	    math/math_common.hpp
		math/vector_common.hpp
		math/vector.hpp
		math/vector_view.hpp
		math/matrix_common.hpp
		math/matrix.hpp
		math/matrix_view.hpp
		math/quaternion.hpp)

set_target_properties(Hrs PROPERTIES LINKER_LANGUAGE CXX)
