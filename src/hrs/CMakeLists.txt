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
		error.hpp
		instantiation.hpp
		free_block_chain_base.hpp
		sized_free_block_chain.hpp
		unsized_free_block_chain.hpp
		stacktrace.hpp
		demangle.hpp
		dynamic_library.hpp
		variadic.hpp
		one_of.hpp
		distinct.hpp
		copy_traits.hpp
		on_thread_exit.hpp
		swap_back_pop.hpp
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
		math/quaternion.hpp
)

target_sources(
Hrs
    PRIVATE
	    test/tests.h
		test/tests_undef.h
		test/test_property.h
		test/test_config.h
		test/test_config.cpp
		test/test_data.h
		test/test_data.cpp
		test/environment.h
		test/environment.cpp
)

target_sources(
Hrs
    PRIVATE
	    meta/class_meta.hpp
		meta/class_meta_def.hpp
		meta/class_meta_undef.hpp
		meta/enum_meta.hpp
		meta/enum_meta_def.hpp
		meta/enum_meta_undef.hpp
		meta/split_namespace.hpp
		meta/meta_attributes.hpp
		meta/reflexpr.hpp
)

target_sources(
Hrs
    PRIVATE
	    static_string/static_string.hpp
		static_string/algorithm.hpp
)

target_sources(
Hrs
    PRIVATE
	    multikey_map/node.hpp
		multikey_map/iterator.hpp
		multikey_map/entry.hpp
		multikey_map/basic_multikey_map_fwd.hpp
		multikey_map/basic_multikey_map.hpp
		multikey_map/multikey_map.hpp
)

set_target_properties(Hrs PROPERTIES LINKER_LANGUAGE CXX)
