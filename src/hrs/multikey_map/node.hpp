#pragma once

#include <array>
#include <cstdint>
#include <tuple>
#include <utility>

namespace hrs
{
    namespace detail
    {
        template<typename N>
        struct children_pair
        {
            N* left;
            N* right;

            constexpr children_pair() noexcept
                : left(nullptr),
                  right(nullptr)
            {}

            ~children_pair() = default;
            constexpr children_pair(const children_pair& pr) noexcept
                : left(pr.left),
                  right(pr.right)
            {}

            constexpr children_pair(children_pair&& pr) noexcept
                : left(std::exchange(pr.left, nullptr)),
                  right(std::exchange(pr.right, nullptr))
            {}

            constexpr children_pair& operator=(const children_pair& pr) noexcept
            {
                left = pr.left;
                right = pr.right;

                return *this;
            }

            constexpr children_pair& operator=(children_pair&& pr) noexcept
            {
                left = std::exchange(pr.left, nullptr);
                right = std::exchange(pr.right, nullptr);

                return *this;
            }
        };

        template<typename V, typename... Keys>
        struct node;

        template<typename V, typename... Keys>
        struct node_base
        {
            std::array<node_base*, sizeof...(Keys)> parents;
            std::array<children_pair<node_base<V, Keys...>>, sizeof...(Keys)> children;

            node_base() = default;
            ~node_base() = default;
            node_base(const node_base&) = default;
            node_base(node_base&&) = default;
            node_base& operator=(const node_base&) = default;
            node_base& operator=(node_base&& nb) = default;

            constexpr void change_parent_child(node_base* old_child,
                                               node_base* new_child,
                                               std::size_t index) noexcept
            {
                if(children[index].left == old_child)
                    children[index].left = new_child;
                else
                {
                    assert(children[index].right == old_child);
                    children[index].right = new_child;
                }
            }

            constexpr node<V, Keys...>* to_node() noexcept
            {
                return static_cast<node<V, Keys...>*>(this);
            }

            constexpr const node<V, Keys...>* to_node() const noexcept
            {
                return static_cast<const node<V, Keys...>*>(this);
            }
        };

        template<typename V, typename... Keys>
        struct node : node_base<V, Keys...>
        {
            using self_t = node<V, Keys...>;
            using base_t = node_base<V, Keys...>;

            std::tuple<std::add_const_t<Keys>...> keys;
            V data;
            std::array<std::uint_least8_t, sizeof...(Keys)> heights;

            template<typename T, typename... TKeys>
            requires(sizeof...(TKeys) == sizeof...(Keys)) &&
                        std::constructible_from<std::tuple<Keys...>, TKeys...> &&
                        std::constructible_from<V, T>
            constexpr node(T&& _data, TKeys&&... _keys) noexcept(
                std::is_nothrow_constructible_v<std::tuple<Keys...>, TKeys...> &&
                std::is_nothrow_constructible_v<V, T>)
                : keys(std::forward<TKeys>(_keys)...),
                  data(std::forward<T>(_data))
            {
                for(std::size_t i = 0; i < sizeof...(Keys); i++)
                    heights[i] = 1;
            }

            template<typename T, typename... TKeys>
            requires(sizeof...(TKeys) == sizeof...(Keys)) &&
                        std::constructible_from<std::tuple<Keys...>, std::tuple<TKeys...>> &&
                        std::constructible_from<V, T>
            constexpr node(T&& _data, std::tuple<TKeys...>&& _keys) noexcept(
                std::is_nothrow_constructible_v<std::tuple<Keys...>, std::tuple<TKeys...>> &&
                std::is_nothrow_constructible_v<V, T>)
                : keys(std::move(_keys)),
                  data(std::forward<T>(_data))
            {
                for(std::size_t i = 0; i < sizeof...(Keys); i++)
                    heights[i] = 1;
            }

            ~node() = default;
            node(const node&) = default;
            node(node&&) = default;
            node& operator=(const node&) = default;
            node& operator=(node&&) = default;

            constexpr node* small_left_rotation(std::size_t index) noexcept
            {
                /*
				 r
				 |
				 |
				 t----> |height| > 1
				/ \
			   /   \
			  /     \
			 с3	0    c 2
					/ \
				   /   \
				  /	    \
				 с2	1	c1 1


				   ||c3.height| - |c.height|| > 1
				   c2.height <= c1.height

					  fix_height(r)
					   */

                node* t = this;
                node* c = t->children[index].right->to_node();
                node* c2 = c->children[index].left->to_node();
                base_t* r = t->parents[index];

                t->children[index].right = c2;

                if(c2)
                    c2->parents[index] = t;

                c->children[index].left = t;
                t->parents[index] = c;

                c->parents[index] = r;
                if(r)
                    r->change_parent_child(t, c, index);

                t->heights[index] = calculate_height(t, index);
                c->heights[index] = calculate_height(c, index);
                return c;
                //return r;
            }

            constexpr node* small_right_rotation(std::size_t index) noexcept
            {
                /*
						 r
						 |
						 |
						 t----> |height| > 1
						/ \
					   /   \
					  /     \
					 с 2    c3 0
					/ \
				   /   \
				  /		\
				 с1	1	 с2	1


				   ||c3.height| - |c.height|| > 1
				   c2.height <= c1.height

					  fix_height(r)
					   */

                node* t = this;
                node* c = t->children[index].left->to_node();
                node* c2 = c->children[index].right->to_node();
                base_t* r = t->parents[index];

                t->children[index].left = c2;
                if(c2)
                    c2->parents[index] = t;

                c->children[index].right = t;
                t->parents[index] = c;

                c->parents[index] = r;
                if(r)
                    r->change_parent_child(t, c, index);

                t->heights[index] = calculate_height(t, index);
                c->heights[index] = calculate_height(c, index);
                return c;
                //return r;
            }

            constexpr node* big_left_rotation(std::size_t index) noexcept
            {
                /*
				 r
				 |
				 |
				 t----> |height| > 1
				/ \
			   /   \
			  /     \
			 с3	1    c 3
					/ \
				   /   \
				  /	    \
				 с2	2	c1 1
				/ \
			   /   \
			  /     \
			 c4 1   c5 1

				   ||c3.height| - |c.height|| > 1
				   c2.height > c1.height

					  fix_height(r)
					   */

                node* t = this;
                node* c = t->children[index].right->to_node();
                node* c2 = c->children[index].left->to_node();
                node* c4 = c2->children[index].left->to_node();
                node* c5 = c2->children[index].right->to_node();
                base_t* r = t->parents[index];

                t->children[index].right = c4;
                if(c4)
                    c4->parents[index] = t;

                c->children[index].left = c5;
                if(c5)
                    c5->parents[index] = c;

                c2->children[index].left = t;
                c2->children[index].right = c;
                t->parents[index] = c2;
                c->parents[index] = c2;
                c2->parents[index] = r;
                if(r)
                    r->change_parent_child(t, c2, index);

                t->heights[index] = calculate_height(t, index);
                c->heights[index] = calculate_height(c, index);
                c2->heights[index] = calculate_height(c2, index);
                return c2;
                //return r;
            }

            constexpr node* big_right_rotation(std::size_t index) noexcept
            {
                /*
				 r
				 |
				 |
				 t----> |height| > 1
				/ \
			   /   \
			  /     \
			 с 3     c3 1
			/ \
		   /   \
		  /		\
		 с1 1    с2	2
				/ \
			   /   \
			  /     \
			 c5 1   c4 1

				   ||c3.height| - |c.height|| > 1
				   c2.height > c1.height

					  fix_height(r)
					   */

                node* t = this;
                node* c = t->children[index].left->to_node();
                node* c2 = c->children[index].right->to_node();
                node* c4 = c2->children[index].right->to_node();
                node* c5 = c2->children[index].left->to_node();
                base_t* r = t->parents[index];

                t->children[index].left = c4;
                if(c4)
                    c4->parents[index] = t;

                c->children[index].right = c5;
                if(c5)
                    c5->parents[index] = c;

                c2->children[index].right = t;
                c2->children[index].left = c;
                t->parents[index] = c2;
                c->parents[index] = c2;
                c2->parents[index] = r;
                if(r)
                    r->change_parent_child(t, c2, index);

                t->heights[index] = calculate_height(t, index);
                c->heights[index] = calculate_height(c, index);
                c2->heights[index] = calculate_height(c2, index);
                return c2;
                //return r;
            }
        };

        template<typename V, typename... Keys>
        std::uint8_t get_node_height(const node<V, Keys...>* node, std::size_t index) noexcept
        {
            if(!node)
                return 0;

            return node->heights[index];
        }

        template<typename V, typename... Keys>
        std::uint8_t calculate_height(const node<V, Keys...>* node, std::size_t index) noexcept
        {
            if(!node)
                return 0;

            std::size_t left_height = get_node_height(node->children[index].left->to_node(), index);
            std::size_t right_height =
                get_node_height(node->children[index].right->to_node(), index);

            return (left_height > right_height ? left_height : right_height) + 1;
        }

        enum class rotation_type
        {
            none,
            small_left,
            big_left,
            small_right,
            big_right,
        };

        template<typename V, typename... Keys>
        rotation_type select_rotation(const node<V, Keys...>* node, std::size_t index) noexcept
        {
            if(!node)
                return rotation_type::none;

            std::uint8_t left_height =
                get_node_height(node->children[index].left->to_node(), index);
            std::uint8_t right_height =
                get_node_height(node->children[index].right->to_node(), index);

            if(left_height == right_height)
                return rotation_type::none;

            if(left_height > right_height)
            {
                std::uint8_t diff = left_height - right_height;
                if(diff < 2)
                    return rotation_type::none;

                const auto* left_node = node->children[index].left;

                std::uint8_t left_height =
                    get_node_height(left_node->children[index].left->to_node(), index);
                std::uint8_t right_height =
                    get_node_height(left_node->children[index].right->to_node(), index);

                if(right_height > left_height)
                    return rotation_type::big_right;

                return rotation_type::small_right;
            }
            else
            {
                std::uint8_t diff = right_height - left_height;
                if(diff < 2)
                    return rotation_type::none;

                const auto* right_node = node->children[index].right;

                std::uint8_t left_height =
                    get_node_height(right_node->children[index].left->to_node(), index);
                std::uint8_t right_height =
                    get_node_height(right_node->children[index].right->to_node(), index);

                if(left_height > right_height)
                    return rotation_type::big_left;

                return rotation_type::small_left;
            }
        }

    };
};
