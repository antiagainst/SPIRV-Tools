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

#include "source/opt/allocator.h"

#if defined(_MSC_VER) && _MSC_VER <= 1800
#define TLS __declspec(thread)
#else
#define TLS thread_local
#endif

static TLS spvtools::Allocator* tlCustomAllocator = nullptr;

namespace spvtools {

void SetCustomAllocator(Allocator* allocator) { tlCustomAllocator = allocator; }

void* CustomAllocate(std::size_t size) {
  assert(tlCustomAllocator);
  return tlCustomAllocator->Allocate(size, /*align=*/8);
}

void CustomDeallocate(void* ptr, std::size_t size) {
  assert(tlCustomAllocator);
  tlCustomAllocator->Deallocate(ptr, size);
}

}  // namespace spvtools
