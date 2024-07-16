//ENV
//NAME, TAG, GROUP, PROPERTIES
#define HRS_ADD_TEST_ON_ENV(ENV, NAME, ...) \
[[maybe_unused]] int _HRS_TEST_LIB_FICTIVE_VARIABLE_##NAME = []() -> int \
{ \
	using ::hrs::operator|; \
	ENV.add_test(NAME, #NAME __VA_OPT__(,) __VA_ARGS__); \
	return 1; \
}();

#define HRS_ADD_TEST(NAME, ...) \
HRS_ADD_TEST_ON_ENV(::hrs::test::test_environment::get_global_environment(), NAME __VA_OPT__(,) __VA_ARGS__)

#define HRS_TEST_ON_ENV(ENV, NAME, ...) \
static void NAME(); \
namespace \
{ \
	HRS_ADD_TEST_ON_ENV(ENV, NAME __VA_OPT__(,) __VA_ARGS__) \
}; \
static void NAME()

#define HRS_TEST(NAME, ...) \
HRS_TEST_ON_ENV(::hrs::test::test_environment::get_global_environment(), NAME __VA_OPT__(,) __VA_ARGS__)

#define HRS_CLASS_TEST_ON_ENV_DECL(ENV, NAME, ...) \
public: \
	static void NAME(); \
private: \
	HRS_ADD_TEST_ON_ENV(ENV, NAME __VA_OPT__(,) __VA_ARGS__)

#define HRS_CLASS_TEST_DECL(NAME, ...) \
HRS_CLASS_TEST_ON_ENV_DECL(::hrs::test::test_environment::get_global_environment(), NAME __VA_OPT__(,) __VA_ARGS__)

#define HRS_CLASS_TEST_ON_ENV(ENV, NAME, ...) \
private: \
	HRS_ADD_TEST_ON_ENV(ENV, NAME __VA_OPT__(,) __VA_ARGS__) \
public: \
	static void NAME()

#define HRS_CLASS_TEST(NAME, ...) \
HRS_CLASS_TEST_ON_ENV(::hrs::test::test_environment::get_global_environment(), NAME __VA_OPT__(,) __VA_ARGS__)

#define HRS_CLASS_TEST_DEF(CLASS, NAME, ...) \
void CLASS::NAME()

#define HRS_SETUP_CLASS_TESTS(CLASS) \
namespace \
{ \
	CLASS _HRS_TEST_LIB_FICTIVE_TEST_CLASS_VARIABLE; \
};

#define HRS_ASSERT_TEST(...) \
if(!(__VA_ARGS__)) \
	throw ::hrs::test::test_assert_exception(#__VA_ARGS__)

#define HRS_FAIL_TEST(...) \
	throw ::hrs::test::test_assert_exception(#__VA_ARGS__)

#define HRS_ASSERT_EQUAL(X, Y) \
HRS_ASSERT_TEST(X == Y)

#define HRS_ASSERT_NOT_EQUAL(X, Y) \
HRS_ASSERT_TEST(X != Y)

#define HRS_ASSERT_LESS(X, Y) \
HRS_ASSERT_TEST(X < Y)

#define HRS_ASSERT_GREATER(X, Y) \
HRS_ASSERT_TEST(X > Y)

#define HRS_ASSERT_LESS_OR_EQUAL(X, Y) \
HRS_ASSERT_TEST(X <= Y)

#define HRS_ASSERT_GREATER_OR_EQUAL(X, Y) \
HRS_ASSERT_TEST(X >= Y)

//Output function
#define HRS_MAIN_TEST(OUTPUT_FUNC, END_OUTPUT_FUNC) \
int main(int argc, char **argv) \
{ \
	::hrs::test::test_environment::get_global_environment().set_output_function(OUTPUT_FUNC); \
	::hrs::test::test_environment::get_global_environment().set_end_output_function(OUTPUT_FUNC); \
	::hrs::test::test_environment::get_global_environment().run(); \
	return 0; \
}


