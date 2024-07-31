#define HRS_REFL_ENUM_BEGIN_EXISTED(NAME, ...) \
namespace hrs \
{ \
	template<> \
	struct enum_meta<NAME> \
		: decltype([]() \
		  { \
			  using enum NAME; \
			  return detail::enum_meta_base<NAME, #NAME, #__VA_ARGS__, __VA_ARGS__>{}; \
		  }())

#define HRS_REFL_ENUM_BEGIN(NAME, ...) \
enum class NAME \
{ \
	__VA_ARGS__ \
}; \
HRS_REFL_ENUM_BEGIN_EXISTED(NAME __VA_OPT__(,) __VA_ARGS__)

#define HRS_REFL_ENUM_TYPED_BEGIN(NAME, TYPE, ...) \
enum class NAME : TYPE \
{ \
	__VA_ARGS__ \
}; \
HRS_REFL_ENUM_BEGIN_EXISTED(NAME __VA_OPT__(,) __VA_ARGS__)

#define HRS_REFL_ENUM_NS_BEGIN(NAME, NS, ...) \
namespace NS \
{ \
	enum class NAME \
	{ \
		__VA_ARGS__ \
	}; \
}; \
HRS_REFL_ENUM_BEGIN_EXISTED(NS::NAME __VA_OPT__(,) __VA_ARGS__)

#define HRS_REFL_ENUM_NS_TYPED_BEGIN(NAME, NS, TYPE, ...) \
namespace NS \
{ \
	enum class NAME : TYPE \
	{ \
		__VA_ARGS__ \
	}; \
}; \
HRS_REFL_ENUM_BEGIN_EXISTED(NS::NAME __VA_OPT__(,) __VA_ARGS__)


#define HRS_REFL_ENUM_BODY(...) \
		  ,meta_attributes<__VA_ARGS__> \
	{

#define HRS_REFL_ENUM_END() \
	}; \
};

#include "enum_meta.hpp"
