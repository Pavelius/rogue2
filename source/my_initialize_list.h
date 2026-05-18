///////////////////////////////////////////////////////////////////////////
//
//  Copyright 2026 by Pavel Chistyakov
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http ://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#pragma once

#ifdef __GNUC__
#include <initializer_list>
#else
namespace std {
template<class T>
class initializer_list {
	const T* first;
	const T* last;
public:
	typedef T value_type;
	typedef const T& reference;
	typedef const T& const_reference;
	typedef unsigned size_type;
	typedef const T* iterator;
	typedef const T* const_iterator;
	constexpr initializer_list() noexcept : first(0), last(0) {}
	constexpr initializer_list(const T* first_arg, const T* last_arg) noexcept : first(first_arg), last(last_arg) {}
	constexpr const T*	begin() const noexcept { return first; }
	constexpr const T*	end() const noexcept { return last; }
	constexpr unsigned	size() const noexcept { return last - first; }
};
}
#endif
