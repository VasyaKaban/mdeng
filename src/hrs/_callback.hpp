#pragma once

#include <concepts>
#include <variant>
#include "function_traits.hpp"

namespace hrs
{
	template<typename O, typename F>
	concept object_callable_like_function =
		std::is_function_v<F> &&
		requires
		{
			{static_cast<F std::remove_reference_t<O>::*>(&std::remove_reference_t<O>::operator())};
		};

	template<typename F, bool is_variadic = function_has_varidic_arguments<F>, typename ...Args>
		requires
			std::is_function_v<F> &&
			(is_variadic == function_has_varidic_arguments<F>)
	struct callable_interface;

	template<typename F, typename ...Args>
	struct callable_interface<F, true, Args...>
	{
		constexpr virtual ~callable_interface() {}
		constexpr virtual function_return_type<F> operator()(Args..., ...)
			noexcept(is_function_noexcept_qualified<F>) = 0;
	};

	template<typename F, typename ...Args>
	struct callable_interface<F, false, Args...>
	{
		constexpr virtual ~callable_interface() {}
		constexpr virtual function_return_type<F> operator()(Args...)
			noexcept(is_function_noexcept_qualified<F>) = 0;
	};

	template<typename F, typename O, typename ...Args>
		requires std::is_function_v<F>
	class callable_realization : public callable_interface<F, function_has_varidic_arguments<F>, Args...>
	{
	public:
		constexpr callable_realization(const O &_obj)
			noexcept(std::is_nothrow_copy_constructible_v<O>)
			requires(std::is_copy_constructible_v<O>)
			: obj(_obj) {}

		constexpr callable_realization(const volatile O &_obj)
			noexcept(std::is_nothrow_copy_constructible_v<O>)
			requires(std::is_copy_constructible_v<O>)
			: obj(_obj) {}

		constexpr callable_realization(O &&_obj)
			noexcept(std::is_nothrow_move_constructible_v<O>)
			requires(std::is_move_constructible_v<O>)
			: obj(std::move(_obj)) {}

		constexpr callable_realization(volatile O &&_obj)
			noexcept(std::is_nothrow_copy_constructible_v<O>)
			requires(std::is_copy_constructible_v<O>)
			: obj(_obj) {}

		virtual ~callable_realization() override = default;

		constexpr virtual function_return_type<F> operator()(Args... args)
			noexcept(is_function_noexcept_qualified<F>) override
		{
			if constexpr(std::same_as<function_return_type<F>, void>)
				obj(args...);
			else
				return obj(args...);
		}
	private:
		O obj;
	};

	template<typename F>
		requires std::is_function_v<F>
	constexpr inline bool can_be_pointer_to_function =
				 (!is_function_const_qualified<F>) &&
				 (!is_function_volatile_qualified<F>) &&
				 (!is_function_lvalue_reference_qualified<F>) &&
				 (!is_function_rvalue_reference_qualified<F>);

	template<typename F>
		requires std::is_function_v<F>
	constexpr auto __get_callable_pointer_interface_type() noexcept
	{
		return []<typename ...Args>(const variadic<Args...>)
		{
			return static_cast<callable_interface<F, function_has_varidic_arguments<F>, Args...> *>(nullptr);
		}(function_arguments<F>());
	}

	template<typename F>
		requires std::is_function_v<F>
	using callable_interface_pointer_type = decltype(__get_callable_pointer_interface_type<F>());

	template<typename O, typename ...Args>
	concept invocable_object =
		std::is_pointer_v<O> &&
		requires(O &obj, Args &&...args)
		{
			{(*obj)(std::forward<Args>(args)...)};
		} ||
		requires(O &obj, Args &&...args)
		{
			{obj(std::forward<Args>(args)...)};
		};


	template<typename F, typename ...Funcs>
		requires
			std::is_function_v<F> &&
			(object_callable_like_function<Funcs, F> && ...)
	class callback
	{
	public:
		using inner_type =
			std::conditional_t<can_be_pointer_to_function<F>,
							   std::variant<std::add_pointer_t<F>, callable_interface_pointer_type<F>, Funcs...>,
							   std::variant<callable_interface_pointer_type<F>, Funcs...>>;
	private:
		inner_type func;

	public:
		constexpr callback(std::nullptr_t = {}) noexcept
			: func(static_cast<callable_interface_pointer_type<F>>(nullptr)) {}

		constexpr ~callback()
		{
			std::visit([this]<typename T>(T &inner)
			{
				if constexpr(std::same_as<callable_interface_pointer_type<F>, T>)
				{
					delete inner;
				}
				else if constexpr(std::is_destructible_v<T>)
					inner.~T();

			}, func);
		}

		constexpr callback(std::add_pointer_t<F> func_ptr) requires(can_be_pointer_to_function<F>)
			: func(func_ptr) {}

		template<object_callable_like_function<F> O>
			requires requires{func = std::forward<O>(std::declval<O>());}
		constexpr callback(O &&obj) : func(std::forward<O>(obj)) {}

		template<object_callable_like_function<F> O>
			requires (!requires{func = std::forward<O>(std::declval<O>());})
		constexpr callback(O &&obj)
			: func(allocate_callable_object(std::forward<O>(obj), function_arguments<F>{})) {}

		/*constexpr callback(const callback &cb) requires (std::is_copy_constructible_v<Funcs> && ...)
		:
		{

		}*/

		constexpr callback(callback &&cb)
			: func(std::move(cb.func))
		{
			cb.func = static_cast<callable_interface_pointer_type<F>>(nullptr);
		}

		/*constexpr callback & operator=(const callback &cb)
		{

		}*/

		constexpr callback & operator=(callback &&cb)
		{
			this->callback();
			func = std::move(cb.func);
			cb.func = static_cast<callable_interface_pointer_type<F>>(nullptr);
			return *this;
		}

		template<object_callable_like_function<F> O>
		constexpr callback & operator=(O &&obj)
		{
			this->~callback();
			if constexpr(requires{func = std::forward<O>(obj);})
				func = std::forward<O>(obj);
			else
				func = allocate_callable_object(std::forward<O>(obj), function_arguments<F>{});

			return *this;
		}

		constexpr void drop() noexcept
		{
			this->~callback();
			func = static_cast<callable_interface_pointer_type<F>>(nullptr);
		}

		explicit constexpr operator bool() const noexcept
		{
			return std::visit([this]<typename T>(T &inner)
			{
				if constexpr(std::is_pointer_v<T>)
					return inner != nullptr;
				else
					return false;
			}, func);
		}

		template<typename ...Args>
		constexpr auto operator()(Args &&...args) const noexcept(is_function_noexcept_qualified<F>)
		{
			if constexpr(std::same_as<function_return_type<F>, void>)
			{
				std::visit([&args...]<typename T>(T &inner) noexcept(is_function_noexcept_qualified<F>)
				{
					if constexpr(std::is_pointer_v<std::remove_cvref_t<T>>)
					{
						static_assert(requires{(*inner)(std::forward<Args>(args)...);},
							"Pointer to functional object cannot be invoked with target arguments!");
						(*inner)(std::forward<Args>(args)...);
					}
					else
					{
						static_assert(requires{inner(std::forward<Args>(args)...);},
							"Functional object cannot be invoked with target arguments!");
						inner(std::forward<Args>(args)...);
					}
				}, func);
			}
			else
			{
				return std::visit([&args...]<typename T>(T &inner) noexcept(is_function_noexcept_qualified<F>)
				{
					if constexpr(std::is_pointer_v<std::remove_cvref_t<T>>)
					{
						static_assert(requires
							{
								{(*inner)(std::forward<Args>(args)...)};// -> std::same_as<function_return_type<F>>;
							},
							"Pointer to functional object cannot be invoked with target arguments!");
						return (*inner)(std::forward<Args>(args)...);
					}
					else
					{

						static_assert(invocable_object<T, Args...>,
							"Functional object cannot be invoked with target arguments!");
						return inner(std::forward<Args>(args)...);
					}
				}, func);
			}
		}

	private:
		template<object_callable_like_function<F> O, typename ...Args>
		constexpr static callable_interface_pointer_type<F>
		allocate_callable_object(O &&obj, const variadic<Args...>)
		{
			return new callable_realization<F, O, Args...>(std::forward<O>(obj));
		}

		/*template<typename ...Args>
		constexpr auto operator()(Args &&...args) const noexcept(is_function_noexcept_qualified<F>)
		{
			if constexpr(std::same_as<function_return_type<F>, void>)
			{
				std::visit([&args...]<typename T>(const T &inner) noexcept(is_function_noexcept_qualified<F>)
				{
					if constexpr(std::is_pointer_v<std::remove_cvref_t<T>>)
					{
						static_assert(requires{(*inner)(std::forward<Args>(args)...);},
							"Pointer to functional object cannot be invoked with target arguments!");
						(*inner)(std::forward<Args>(args)...);
					}
					else
					{
						 static_assert(requires{inner(std::forward<Args>(args)...);},
							"Functional object cannot be invoked with target arguments!");
						inner(std::forward<Args>(args)...);
					}
				}, func);
			}
			else
			{
				return std::visit([&args...]<typename T>(const T &inner) noexcept(is_function_noexcept_qualified<F>)
				{
					if constexpr(std::is_pointer_v<std::remove_cvref_t<T>>)
					{
						static_assert(requires
						{
							{(*inner)(std::forward<Args>(args)...)};// -> std::same_as<function_return_type<F>>;
						},
							"Pointer to functional object cannot be invoked with target arguments!");
						return (*inner)(std::forward<Args>(args)...);
					}
					else
					{
						static_assert(invocable_object<T, Args...>,
							"Functional object cannot be invoked with target arguments!");
						return inner(std::forward<Args>(args)...);
					}
				}, func);
			}
		}*/
	};

	/*struct C
	{
		constexpr C() = default;
		constexpr C(const C &) = default;
		constexpr C(const volatile C &c)
		{

		}
		//constexpr C(const volatile C &) = default;

		constexpr int operator()(int a, float b) volatile
		{
			return a + b;
		}

		constexpr int operator()(int a, float b) const
		{
			return a + b;
		}
	};

	struct A
	{
		constexpr A() = default;

		constexpr int operator()(int a, float b)
		{
			return a + b;
		}

		constexpr int operator()(int a, float b) const
		{
			return a + b;
		}

		constexpr int operator()(int a, float b) volatile
		{
			return a + b;
		}
	};

	struct B
	{
		constexpr int operator()(int a, float b)
		{
			return a + b;
		}

		constexpr int operator()(int a, float b) volatile
		{
			return a + b;
		}
	};

	constexpr int foo(int a, float b)
	{
		return b - a;
	}

	auto bar()
	{
		constexpr callback<int (int, float), A> cb;

		constexpr callback<int (int, float), A> cback1(&foo);


		static_assert(invocable_object<decltype(&foo), int&&, float&&>);



		constexpr auto r1 = cback1(1, 3.14f);

		callback<int (int, float), A> cback2(A{});
		constexpr A a;
		constexpr callback<int (int, float), A> cback3(a);


		auto r2 = cback2(1, 3.14f);
		constexpr auto r3 = cback3(1, 3.14f);


		//constexpr callback<int (int, float), A> cback4(B{});
		//auto r4 = cback4(1, 3.14f);

		static_assert(object_callable_like_function<decltype([&r2](int a, float b) mutable -> int {return b * a;}), int (int, float)>);

		callback<int (int, float), A> cback5([&r2](int a, float b) mutable -> int {return b * a;});
		auto r5 = cback5(1, 3.14f);


		callback<int (int, float) volatile, A> cbc(A{});
		cbc(1, 3.14f);

	}*/
};
