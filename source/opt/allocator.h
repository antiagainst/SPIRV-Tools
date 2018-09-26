// Copyright (c) 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SOURCE_ALLOCATOR_H_
#define SOURCE_ALLOCATOR_H_

#include <cstddef>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace spvtools {

void* FallbackableCustomAllocate(std::size_t size);
void FallbackableCustomDeallocate(void* ptr, std::size_t size);

// A custom allocator implementing the allocator interface required by
// STL containers
template <typename T>
struct StlAllocator {
  // type definitions
  typedef T value_type;
  typedef T* pointer;
  typedef const T* const_pointer;
  typedef T& reference;
  typedef const T& const_reference;
  typedef size_t size_type;
  typedef size_t difference_type;

  // rebind allocator to type U
  template <class U>
  struct rebind {
    typedef StlAllocator<U> other;
  };

  StlAllocator() noexcept {}

  template <class U>
  StlAllocator(const StlAllocator<U>&) noexcept {}

  // allocates but does not initialize num elements of type T
  T* allocate(std::size_t n) {
    return static_cast<T*>(FallbackableCustomAllocate(sizeof(T) * n));
  }

  // deallocates storage p of deleted elements
  void deallocate(T* p, std::size_t size) noexcept {
    FallbackableCustomDeallocate(p, size);
  }

  // initializes object of allocated storage p with value val
  void construct(pointer p, const_reference val) { new (p) T(val); }

  // Constructs an object of Type U a the location given by P passing
  // through all other arguments to the constructor.
  template <typename U, typename... Args>
  void construct(U* p, Args&&... args) {
    ::new ((void*)p) U(std::forward<Args>(args)...);
  }

  // Deconstructs the object at p. It does not free the memory.
  void destroy(pointer p) { ((T*)p)->~T(); }

  // Deconstructs the object at p. It does not free the memory.
  template <typename U>
  void destroy(U* p) {
    p->~U();
  }

  // Returns the maximum theoretically possible number of T stored in
  // this allocator.
  size_type max_size() const {
    return std::numeric_limits<size_type>::max() / sizeof(value_type);
  }
};

template <class T>
struct CADeleter {
  CADeleter() {}

  // Allows converting from CAUniquePtr<Derived> to CAUniquePtr<Base>
  template <class U, typename = typename std::enable_if<
                         std::is_convertible<U*, T*>::value>::type>
  CADeleter(const CADeleter<U>&) {}

  void operator()(T* ptr) { FallbackableCustomDeallocate(ptr, sizeof(T)); }
};

template <class T>
using CAUniquePtr = std::unique_ptr<T, CADeleter<T>>;

template <typename T, typename... Args>
CAUniquePtr<T> CAMakeUnique(Args&&... args) {
  auto* p = FallbackableCustomAllocate(sizeof(T));
  return CAUniquePtr<T>(new (p) T(std::forward<Args>(args)...));
}

template <class Key, class T, class Compare = std::less<Key>>
using CAMap = std::map<Key, T, Compare, StlAllocator<std::pair<const Key, T>>>;

template <class Key, class T, class Hash = std::hash<Key>,
          class KeyEqual = std::equal_to<Key>>
using CAUnorderedMap =
    std::unordered_map<Key, T, Hash, KeyEqual,
                       StlAllocator<std::pair<const Key, T>>>;

template <class Key, class Hash = std::hash<Key>,
          class KeyEqual = std::equal_to<Key>>
using CAUnorderedSet =
    std::unordered_set<Key, Hash, KeyEqual, StlAllocator<Key>>;

template <class Key, class Compare = std::less<Key>>
using CASet = std::set<Key, Compare, StlAllocator<Key>>;

// STL uses the equality operator to determine if memory allocated by one
// allocator can be deallocated with another.
template <class T, class U>
bool operator==(const StlAllocator<T>&, const StlAllocator<U>&) noexcept {
  return true;  // For safety
}

template <class T, class U>
bool operator!=(const StlAllocator<T>& x, const StlAllocator<U>& y) noexcept {
  return !(x == y);
}

}  // namespace spvtools

#endif  // SOURCE_ALLOCATOR_H_
