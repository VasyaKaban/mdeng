#pragma once

#include <cstddef>
#include <cassert>
#include "../variadic.hpp"
#include "../instantiation.hpp"
#include "../copy_traits.hpp"
#include "node.hpp"
#include "basic_multikey_map_fwd.hpp"

namespace hrs
{
	namespace detail
	{
		template<std::size_t Index, typename M>
			requires
				hrs::type_instantiation<std::remove_cv_t<M>, basic_multikey_map>
		class iterator
		{
		public:

			template<typename V, typename A, type_instantiation<key> CKey, type_instantiation<key> ...CKeys>
			friend class ::hrs::basic_multikey_map;

			using key_t = const typename M::compound_keys_t::template nth_t<Index>::key_t;
			using value_t = copy_cv_t<M, typename M::value_t>;
			using node_base_t = copy_cv_t<M, typename M::node_base_t>;
			using node_t = copy_cv_t<M, typename M::node_t>;

			//iterator traits
			using difference_type = std::ptrdiff_t;
			using value_type = std::pair<key_t &, value_t &>;
			using pointer = value_type *;
			using reference = value_type &;
			using iterator_category = std::bidirectional_iterator_tag;
			using iterator_concept = std::bidirectional_iterator_tag;

			constexpr iterator(node_base_t *_n = nullptr) noexcept
				: n(_n) {}

			~iterator() = default;
			iterator(const iterator &) = default;
			iterator(iterator &&) = default;
			iterator & operator=(const iterator &) = default;
			iterator & operator=(iterator &&) = default;

			constexpr auto operator++(int) noexcept
			{
				auto it = *this;
				++(*this);
				return it;
			}

			constexpr decltype(auto) operator++() noexcept
			{
				if(n->children[Index].right)
				{
					n = n->children[Index].right;
					while(true)
					{
						if(n->children[Index].left)
							n = n->children[Index].left;
						else
							break;
					}
				}
				else
				{
					while(true)
					{
						auto parent = n->parents[Index];
						if(!parent)
							return *this;
						//assert(parent);
						if(parent)
						{
							if(parent->children[Index].left == n)
							{
								n = parent;
								break;
							}
							else
								n = parent;
						}
					}
				}

				return *this;
			}

			constexpr auto operator--(int) noexcept
			{
				auto it = *this;
				--(*this);
				return it;
			}

			constexpr decltype(auto) operator--() noexcept
			{
				if(n->children[Index].left)
				{
					n = n->children[Index].left;
					while(true)
					{
						if(n->children[Index].right)
							n = n->children[Index].right;
						else
							break;
					}
				}
				else
				{
					while(true)
					{
						auto parent = n->parents[Index];
						if(!parent)
							return *this;
						//assert(parent);

						if(parent->children[Index].right == n)
						{
							n = parent;
							break;
						}
						else
							n = parent;
					}
				}

				return *this;
			}

			constexpr std::pair<key_t &, value_t &> operator*() noexcept
			{
				auto *node = n->to_node();
				return {std::get<Index>(node->keys), node->data};
			}

			constexpr std::pair<key_t &, value_t &> operator*() const noexcept
			{
				auto *node = n->to_node();
				return {std::get<Index>(node->keys), node->data};
			}


			constexpr key_t & key() const noexcept
			{
				auto *node = n->to_node();
				return std::get<Index>(node->keys);
			}

			constexpr decltype(auto) keys() const noexcept
			{
				return n->to_node()->keys;
			}

			constexpr value_t & value() noexcept
			{
				auto *node = static_cast<node_t *>(n);
				return node->data;
			}

			constexpr const value_t & value() const noexcept
			{
				auto *node = static_cast<node_t *>(n);
				return node->data;
			}

			constexpr bool operator==(const iterator &it) const noexcept
			{
				return n == it.n;
			}

			template<std::size_t RIndex>
				requires (RIndex < M::compound_keys_t::COUNT)
			constexpr iterator<RIndex, M> rebind() noexcept
			{
				return {n};
			}

			template<std::size_t RIndex>
				requires (RIndex < M::compound_keys_t::COUNT)
			constexpr const iterator<RIndex, M> rebind() const noexcept
			{
				return {n};
			}

		private:
			node_base_t *n;
		};
	};
};
