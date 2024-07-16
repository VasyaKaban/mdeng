#pragma once

#include <memory>
#include "entry.hpp"

namespace hrs
{
	template<typename V, typename A, type_instantiation<key> CKey, type_instantiation<key> ...CKeys>
	class basic_multikey_map
	{
	public:

		template<std::size_t Index, typename M>
		requires
			hrs::type_instantiation<std::remove_cv_t<M>, basic_multikey_map>
		friend class ::hrs::detail::entry;

		using compound_keys_t = hrs::variadic<CKey, CKeys...>;
		using value_t = V;

		using node_base_t = detail::node_base<V, typename CKey::key_t, typename CKeys::key_t...>;
		using node_t = detail::node<V, typename CKey::key_t, typename CKeys::key_t...>;

		template<std::size_t Index>
		using entry_t = detail::entry<Index, basic_multikey_map>;

		template<std::size_t Index>
		using const_entry_t = detail::entry<Index, const basic_multikey_map>;

		using allocator_t = std::allocator_traits<A>::template rebind_alloc<node_t>;
		using comparators_t = std::tuple<typename CKey::comparator_t, typename CKeys::comparator_t...>;

		basic_multikey_map()
			noexcept(std::is_nothrow_default_constructible_v<allocator_t> &&
					 std::is_nothrow_default_constructible_v<comparators_t>)
			requires std::is_default_constructible_v<allocator_t> &&
					 std::is_default_constructible_v<comparators_t>
			: size(0) {}

		template<typename ...Comps>
			requires std::is_default_constructible_v<allocator_t> &&
					 std::constructible_from<comparators_t, Comps...>
		basic_multikey_map(Comps &&...comps)
			noexcept(std::is_nothrow_default_constructible_v<allocator_t> &&
					 std::is_nothrow_constructible_v<comparators_t, Comps...>)
			: comparators(std::forward<Comps>(comps)...),
			  size(0) {}

		template<typename Alloc>
			requires std::constructible_from<allocator_t, Alloc>
		basic_multikey_map(Alloc &&_allocator)
			noexcept(std::is_nothrow_constructible_v<allocator_t, Alloc> &&
					 std::is_nothrow_default_constructible_v<comparators_t>)
			: allocator(std::forward<Alloc>(_allocator)),
			  size(0) {}

		template<typename Alloc, typename ...Comps>
			requires std::constructible_from<allocator_t, Alloc> &&
					 std::constructible_from<comparators_t, Comps...>
		basic_multikey_map(Alloc &&_allocator, Comps &&...comps)
			noexcept(std::is_nothrow_constructible_v<allocator_t, Alloc> &&
					 std::is_nothrow_constructible_v<comparators_t, Comps...>)
			: comparators(std::forward<Comps>(comps)...),
			  allocator(std::forward<Alloc>(_allocator)),
			  size(0) {}

		~basic_multikey_map()
		{
			destroy();
		}

		basic_multikey_map(const basic_multikey_map &mkm)
			requires std::copy_constructible<comparators_t>
			: comparators(mkm.comparators),
			  allocator(std::allocator_traits<allocator_t>::select_on_container_copy_construction(mkm.allocator)),
			  size(0)
		{
			auto _entry = mkm.get_entry<0>();
			for(auto it = _entry.begin(); it != _entry.end(); it++)
			{
				const node_t *node = it.n->to_node();
				insert(node->data, node->keys);
			}
		}

		basic_multikey_map(basic_multikey_map &&mkm)
			noexcept(std::is_nothrow_move_constructible_v<allocator_t> &&
					 (std::is_nothrow_move_constructible_v<comparators_t> ||
					  std::is_nothrow_copy_constructible_v<comparators_t>))
			requires std::move_constructible<allocator_t> &&
					 (std::move_constructible<comparators_t> || std::copy_constructible<comparators_t>)
			: comparators(std::move_if_noexcept(mkm.comparators)),
			  allocator(std::move(mkm.allocator)),
			  root(std::exchange(mkm.root, {})),
			  size(std::exchange(mkm.size, 0)) {}

		basic_multikey_map & operator=(const basic_multikey_map &mkm)
			requires std::is_copy_assignable_v<comparators_t>
		{
			clear();
			if constexpr(std::allocator_traits<allocator_t>::propagate_on_container_copy_assignment::value)
				allocator = mkm.allocator;

			comparators = mkm.comparators;
			auto _entry = mkm.get_entry<0>();
			for(auto it = _entry.begin(); it != _entry.end(); it++)
			{
				const node_t *node = it.n->to_node();
				insert(node->data, node->keys);
			}

			return *this;
		}

		basic_multikey_map & operator=(basic_multikey_map &&mkm)
			noexcept(std::allocator_traits<allocator_t>::propagate_on_container_move_assignment::value &&
					 std::is_nothrow_move_assignable_v<allocator_t> &&
					 (std::is_nothrow_move_assignable_v<comparators_t> ||
					  std::is_nothrow_copy_assignable_v<comparators_t>))
			requires std::move_constructible<comparators_t> || std::copy_constructible<comparators_t>
		{
			clear();
			comparators = std::move_if_noexcept(mkm.comparators);

			if constexpr(std::allocator_traits<allocator_t>::propagate_on_container_move_assignment::value)
			{
				allocator = std::move(mkm.allocator);
				root = std::exchange(mkm.root, {});
				size = std::exchange(mkm.size, 0);
			}
			else
			{
				if(allocator == mkm.allocator)
				{
					root = std::exchange(mkm.root, {});
					size = std::exchange(mkm.size, 0);
				}
				else
				{
					//move each value/key
					for(auto it = mkm.get_entry<0>().begin(); it != mkm.get_entry<0>().end(); it++)
					{
						std::apply([it, this](auto &..._keys)
						{
							insert(std::move_if_noexcept(it.value()), std::move_if_noexcept(_keys)...);
						}, it.keys());
					}

					mkm.clear();
				}
			}

			return *this;
		}

		std::size_t get_size() const noexcept
		{
			return size;
		}

		bool is_empty() const noexcept
		{
			return size == 0;
		}

		void clear() noexcept
		{
			destroy_next(root.children[0].left->to_node());
			root = {};
			size = 0;
		}

		template<std::size_t Index>
		entry_t<Index> get_entry() noexcept
		{
			return entry_t<Index>(this);
		}

		template<std::size_t Index>
		const_entry_t<Index> get_entry() const noexcept
		{
			return const_entry_t<Index>(this);
		}

		template<std::size_t Index, typename M>
			requires std::same_as<std::remove_cv_t<M>, basic_multikey_map>
		void erase(const detail::iterator<Index, M> it) noexcept
		{
			if(it == get_entry<Index>().end())
				return;

			assert(size != 0);

			for(std::size_t i = 0; i < sizeof...(CKeys) + 1; i++)
				virtual_erase(const_cast<node_t *>(it.n->to_node()), i);

			std::allocator_traits<allocator_t>::destroy(allocator, it.n->to_node());
			std::allocator_traits<allocator_t>::deallocate(allocator, it.n->to_node(), 1);
			size--;
		}

		//if inserted:
		//	iterator to new node, false, any
		//else
		//	iterator to existed node, true, index of already existed key(only for unique mapping)
		template<typename T, typename ...TKeys>
			requires
				(sizeof...(TKeys) == sizeof...(CKeys) + 1) &&
				std::constructible_from<node_t, T, TKeys...>
		std::tuple<typename entry_t<0>::iterator, bool, std::size_t> insert(T &&value, TKeys &&...keys)
		{
			auto [node, inserted, index] = virtual_insert<0>(std::forward<T>(value),
															 std::forward<TKeys>(keys)...);

			if(inserted)
			{
				size++;
				return
					std::tuple<typename entry_t<0>::iterator, bool, std::size_t>
					{
						typename entry_t<0>::iterator{node},
						inserted,
						index
					};
			}
			else
				return
					std::tuple<typename entry_t<0>::iterator, bool, std::size_t>
					{
						get_entry<0>().end(),
						inserted,
						index
					};
		}

		template<std::size_t Index, typename K>
			requires
				(Index < sizeof...(CKeys) + 1) &&
				requires(K &&key,
						 compound_keys_t::template nth_t<Index>::comparator_t &comp,
						 compound_keys_t::template nth_t<Index>::key_t &key_comp)
				{
					{comp(std::forward<K>(key), key_comp)} -> std::same_as<bool>;
					{comp(key_comp, std::forward<K>(key))} -> std::same_as<bool>;
				}
		entry_t<Index>::iterator find(K &&key)
		{
			auto *node = root.children[Index].left->to_node();
			auto &comp = std::get<Index>(comparators);
			while(node)
			{
				auto &node_key = std::get<Index>(node->keys);
				bool cmp1 = comp(std::forward<K>(key), node_key);

				if(cmp1)// -> go left
					node = node->children[Index].left->to_node();
				else
				{
					bool cmp2 = comp(node_key, std::forward<K>(key));
					if(!cmp2)// -> equal
						return node;
						//return {node, false};
					else// -> go right
						node = node->children[Index].right->to_node();
				}
			}

			return get_entry<Index>().end();
		}

		template<std::size_t Index, typename K>
			requires
				(Index < sizeof...(CKeys) + 1) &&
				requires(K &&key,
						 compound_keys_t::template nth_t<Index>::comparator_t &comp,
						 compound_keys_t::template nth_t<Index>::key_t &key_comp)
				{
					{comp(std::forward<K>(key), key_comp)} -> std::same_as<bool>;
					{comp(key_comp, std::forward<K>(key))} -> std::same_as<bool>;
				}
		const_entry_t<Index>::iterator find(K &&key) const
		{
			auto *node = root.children[Index].left->to_node();
			auto &comp = std::get<Index>(comparators);
			while(node)
			{
				auto &node_key = std::get<Index>(node->keys);
				bool cmp1 = comp(std::forward<K>(key), node_key);

				if(cmp1)// -> go left
					node = node->children[Index].left->to_node();
				else
				{
					bool cmp2 = comp(node_key, std::forward<K>(key));
					if(!cmp2)// -> equal
						return node;
					//return {node, false};
					else// -> go right
						node = node->children[Index].right->to_node();
				}
			}

			return get_entry<Index>().end();
		}

		template<std::size_t Index, typename K>
			requires
				(Index < sizeof...(CKeys) + 1) &&
				requires(K &&key,
						 compound_keys_t::template nth_t<Index>::comparator_t &comp,
						 compound_keys_t::template nth_t<Index>::key_t &key_comp)
				{
					{comp(std::forward<K>(key), key_comp)} noexcept -> std::same_as<bool>;
					{comp(key_comp, std::forward<K>(key))} noexcept -> std::same_as<bool>;
				}
		bool contains(K &&key) const noexcept
		{
			return find<Index>(key) != get_entry<Index>().end();
		}

		allocator_t & get_allocator() noexcept
		{
			return allocator;
		}

		const allocator_t & get_allocator() const noexcept
		{
			return allocator;
		}

	private:
		void destroy_next(node_t *node) noexcept
		{
			if(!node)
				return;

			destroy_next(node->children[0].left->to_node());

			auto *right = node->children[0].right->to_node();
			delete node;

			destroy_next(right);
		}

		void destroy() noexcept
		{
			destroy_next(root.children[0].left->to_node());
		}

		void virtual_erase(node_t *node, std::size_t index) noexcept
		{
			if(node->children[index].left == nullptr && node->children[index].right == nullptr)
			{
				auto *parent = node->parents[index];
				//if(parent)
				//{
				parent->change_parent_child(node, nullptr, index);
					//#error WHAT ABOUT node->height recalculation???
				fix_map(parent, index);
				//}
				//else
				//	roots[index] = nullptr;
			}
			else if(node->children[index].left == nullptr && node->children[index].right != nullptr)
			{
				auto *parent = node->parents[index];
				auto *new_node = node->children[index].right;

				new_node->parents[index] = parent;
				//if(parent)
				parent->change_parent_child(node, new_node, index);
				//else
				//	roots[index] = new_node;

				fix_map(parent, index);
			}
			else if(node->children[index].left != nullptr && node->children[index].right == nullptr)
			{
				auto *parent = node->parents[index];
				auto *new_node = node->children[index].left;

				new_node->parents[index] = parent;
				//if(parent)
				parent->change_parent_child(node, new_node, index);
				//else
				//	roots[index] = new_node;

				fix_map(parent, index);
			}
			else
			{
				auto *right_node = node->children[index].right;
				auto *right_left_node = right_node->children[index].left;
				if(!right_left_node)
				{
					auto *parent = node->parents[index];
					right_node->parents[index] = parent;
					//if(parent)
					parent->change_parent_child(node, right_node, index);
					//else
					//	roots[index] = right_node->children[index].right;

					right_node->children[index].left = node->children[index].left;
					node->children[index].left->parents[index] = right_node;
					fix_map(right_node, index);
					//fix_map(parent, index);
				}
				else
				{
					auto *ll_node = right_node->children[index].left;
					while(true)
					{
						if(!ll_node->children[index].left)
							break;

						ll_node = ll_node->children[index].left;
					}

					auto *n_parent = node->parents[index];
					auto *ll_parent = ll_node->parents[index];
					std::swap(node->parents[index], ll_node->parents[index]);
					//if(n_parent)
					n_parent->change_parent_child(node, ll_node, index);
					//else
					//	roots[index] = ll_node;

						   //no check for ll_parent, because ll_node always has a parent!
					assert(ll_parent);
					ll_parent->change_parent_child(ll_node, node, index);

					auto *n_left = node->children[index].left;
					auto *ll_left = ll_node->children[index].left;
					std::swap(node->children[index].left, ll_node->children[index].left);
					if(n_left)
						n_left->parents[index] = ll_node;

					if(ll_left)
						ll_left->parents[index] = node;


					auto *n_right = node->children[index].right;
					auto *ll_right = ll_node->children[index].right;
					std::swap(node->children[index].right, ll_node->children[index].right);
					if(n_right)
						n_right->parents[index] = ll_node;

					if(ll_right)
						ll_right->parents[index] = node;

					virtual_erase(node, index);
				}
			}
		}

		template<std::size_t Index, typename T, typename ...TKeys>
		std::tuple<node_t *, bool, std::size_t>
		virtual_insert(T &&value, TKeys &&...keys)
		{
			using out_t = std::tuple<node_t *, bool, std::size_t>;
			auto *node = root.children[Index].left->to_node();
			if(!node)
			{
				if constexpr(Index == sizeof...(CKeys))
				{
					auto *new_node = create_node(std::forward<T>(value), std::forward<TKeys>(keys)...);
					root.children[Index].left = new_node;
					new_node->parents[Index] = &root;
					return out_t(new_node, true, 0);
				}
				else
				{
					auto tpl = virtual_insert<Index + 1>(std::forward<T>(value),
														 std::forward<TKeys>(keys)...);

					if(std::get<1>(tpl))//inserted?
					{
						root.children[Index].left = std::get<0>(tpl);
						std::get<0>(tpl)->parents[Index] = &root;
					}

					return tpl;
				}
			}
			else
			{
				auto &&key = nth_argument<Index>(std::forward<TKeys>(keys)...);
				auto &comp = std::get<Index>(comparators);
				while(node)
				{
					bool cmp1 = comp(std::forward<decltype(key)>(key), std::get<Index>(node->keys));
					if(cmp1)// -> go left
					{
						if(node->children[Index].left)
							node = node->children[Index].left->to_node();
						else
						{
							node_t *new_node;
							if constexpr(Index == sizeof...(CKeys))
							{
								new_node = create_node(std::forward<T>(value), std::forward<TKeys>(keys)...);

								node->children[Index].left = new_node;
								new_node->parents[Index] = node;
								fix_map(node, Index);

								return out_t(new_node, true, 0);
							}
							else
							{
								auto tpl = virtual_insert<Index + 1>(std::forward<T>(value),
																	 std::forward<TKeys>(keys)...);

								new_node = std::get<0>(tpl);
								if(std::get<1>(tpl))//inserted?
								{
									node->children[Index].left = new_node;
									new_node->parents[Index] = node;
									fix_map(node, Index);
								}

								return tpl;
							}
						}
					}
					else
					{
						bool cmp2 = comp(std::get<Index>(node->keys), std::forward<decltype(key)>(key));
						if(!cmp2)// equal
						{
							if constexpr(std::same_as<typename compound_keys_t::template
													  nth_t<Index>::key_map_t, map_unique_key>)
							{
								return out_t(node, false, Index);
							}
						}

						// -> go right
						if(node->children[Index].right)
							node = node->children[Index].right->to_node();
						else
						{
							node_t *new_node;
							if constexpr(Index == sizeof...(CKeys))
							{
								new_node = create_node(std::forward<T>(value), std::forward<TKeys>(keys)...);

								node->children[Index].right = new_node;
								new_node->parents[Index] = node;
								fix_map(node, Index);

								return out_t(new_node, true, 0);
							}
							else
							{
								auto tpl = virtual_insert<Index + 1>(std::forward<T>(value),
																	 std::forward<TKeys>(keys)...);

								new_node = std::get<0>(tpl);
								if(std::get<1>(tpl))//inserted?
								{
									node->children[Index].right = new_node;
									new_node->parents[Index] = node;
									fix_map(node, Index);
								}

								return tpl;
							}
						}
					}
				}

				assert(false);
			}
		}

		void fix_map(node_base_t *bnode, std::size_t index) noexcept
		{
			using enum detail::rotation_type;
			const node_base_t *end_node = &root;
			while(bnode != end_node)
			{
				node_t *node = bnode->to_node();
				node->heights[index] = calculate_height(node, index);
				node_t *prev_node = node;
				detail::rotation_type rot = select_rotation(node, index);
				switch(rot)
				{
					case none:
						//
						break;
					case small_left:
						node = node->small_left_rotation(index);
						break;
					case big_left:
						node = node->big_left_rotation(index);
						break;
					case small_right:
						node = node->small_right_rotation(index);
						break;
					case big_right:
						node = node->big_right_rotation(index);
						break;
				}

				if(rot != none)
				{
					if(root.children[index].left == prev_node)
						root.children[index].left = node;
				}

				bnode = node->parents[index];
			};
		}

		template<typename TV, typename ...TKeys>
		node_t * create_node(TV &&value, TKeys &&...keys)
		{
			auto *ptr = std::allocator_traits<allocator_t>::allocate(allocator, 1);
			if constexpr(std::is_nothrow_constructible_v<node_t, TV, TKeys...>)
			{
				std::allocator_traits<allocator_t>::construct(allocator,
															  ptr,
															  std::forward<TV>(value),
															  std::forward<TKeys>(keys)...);
			}
			else
			{
				try
				{
					std::allocator_traits<allocator_t>::construct(allocator,
																  ptr,
																  std::forward<TV>(value),
																  std::forward<TKeys>(keys)...);
				}
				catch(...)
				{
					std::allocator_traits<allocator_t>::deallocate(allocator, ptr, 1);
					throw;
				}
			}
			return ptr;
		}

	private:
		comparators_t comparators;
		[[no_unique_address]] allocator_t allocator;
		node_base_t root;
		std::size_t size;
	};
};
