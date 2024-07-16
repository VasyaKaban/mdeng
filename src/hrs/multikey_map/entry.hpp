#pragma once

#include "basic_multikey_map_fwd.hpp"
#include "iterator.hpp"

namespace hrs
{
	namespace detail
	{
		template<std::size_t Index, typename M>
			requires
				hrs::type_instantiation<std::remove_cv_t<M>, basic_multikey_map>
		class entry
		{
		public:
			using map_t = M;

			using key_t = typename M::compound_keys_t::template nth_t<Index>::key_t;
			using value_t = typename M::value_t;
			using node_base_t = copy_cv_t<M, typename M::node_base_t>;
			using node_t = copy_cv_t<M, typename M::node_t>;

			using iterator = detail::iterator<Index, M>;
			using const_iterator = detail::iterator<Index, M>;
			using reverse_iterator = std::reverse_iterator<iterator>;
			using const_reverse_iterator = std::reverse_iterator<const_iterator>;

			constexpr entry(map_t *_map_ptr = nullptr) noexcept
				: map_ptr(_map_ptr) {}

			~entry() = default;
			entry(const entry &) = default;

			auto begin() noexcept
			{
				auto *n = &map_ptr->root;
				while(true)
				{
					if(n->children[Index].left)
						n = n->children[Index].left;
					else
						break;
				}

				return iterator{n};
			}

			auto begin() const noexcept
			{
				auto *n = &map_ptr->root;
				while(true)
				{
					if(n->children[Index].left)
						n = n->children[Index].left;
					else
						break;
				}

				return const_iterator{n};
			}

			auto end() noexcept
			{
				return iterator{&map_ptr->root};
			}

			auto end() const noexcept
			{
				return const_iterator{&map_ptr->root};
			}

			auto rbegin() noexcept
			{
				return reverse_iterator{end()};
			}

			auto rbegin() const noexcept
			{
				return const_reverse_iterator{end()};
			}

			auto rend() noexcept
			{
				return reverse_iterator{begin()};
			}

			auto rend() const noexcept
			{
				return const_reverse_iterator{begin()};
			}

			constexpr bool is_valid() const noexcept
			{
				return map_ptr;
			}

			constexpr operator bool() const noexcept
			{
				return is_valid();
			}

			constexpr map_t * get_map() noexcept
			{
				return map_ptr;
			}

			constexpr const map_t * get_map() const noexcept
			{
				return map_ptr;
			}

			template<std::size_t RIndex>
				requires (RIndex <= M::compound_keys_t::COUNT)
			constexpr auto rebind() noexcept
			{
				return entry<RIndex, M>(map_ptr);
			}

		private:
			map_t *map_ptr;
		};
	};
};
