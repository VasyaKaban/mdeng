#include "hrs/debug.hpp"
#include "hrs/stacktrace.hpp"

void foobar()
{
    hrs::assert_true(false, "Stacktrace test!");
}

void bar()
{
    foobar();
}

void foo_err()
{
    bar();
}

#include "LuaWay/VM.h"
#include "Renderer/Context/Context.h"
#include "Renderer/Vulkan/VkResultMeta.hpp"
#include "hrs/meta/reflexpr.hpp"
#include <cassert>
#include <iostream>

template<typename T>
struct A
{
    using type = T;
};

template<typename T>
using AT = A<T>::type;

constexpr auto script = R"(
	function foo(a, b)
		print("Received : ", a, b)
		local i = coroutine.yield(a + b)
		print("Received: ", i)
		coroutine.yield(2 + i)
		print('End!')
	end
)";

#warning Amogus!

int i = 0;

int& foo()
{
    return i;
}

int main(int argc, char** argv)
{
    auto vm_opt = LuaWay::VM::Open(true);
    assert(vm_opt);

    auto vm = std::move(*vm_opt);
    assert(vm.DoString(script, {}));
    auto foo = vm.GetGlobal("foo");
    auto co = vm.CreateThread(foo);
    assert(co.IsCreated());

    auto co_handle = co.As<LuaWay::Thread>().value();

    auto res_exp = co.Resume(1.0, 2.0);
    assert(res_exp);

    std::cout << "First: " << res_exp.value()[0].As<LuaWay::Number>().value() << std::endl;

    auto res_exp_2 = co.Resume(3.0);
    if(!res_exp_2)
        std::cerr << res_exp_2.error().message << std::endl;
    assert(res_exp_2);
    std::cout << "Second: " << res_exp_2.value()[0].As<LuaWay::Number>().value() << std::endl;
    co.Resume();
    co.Resume();

    // auto val = res_exp.value()[0].As<LuaWay::Number>().value();

    foo_err();
    auto ctx_exp = FireLand::Context::Init();
    assert(ctx_exp);

    auto ctx = std::move(*ctx_exp);
    auto version_exp = ctx.QueryVersion();
    if(!version_exp)
    {
        hrs::reflexpr<VkResult>::get_name(version_exp.error());
        std::cout << hrs::reflexpr<VkResult>::get_name(version_exp.error()) << std::endl;
        exit(-1);
    }
    else
    {
        std::cout << VK_VERSION_MAJOR(*version_exp) << "." << VK_VERSION_MINOR(*version_exp) << "."
                  << VK_VERSION_PATCH(*version_exp) << std::endl;
    }

    std::setlocale(LC_ALL, "ru");
    // foo();
    return 0;
}
