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

#include "spirv-tools/optimizer.hpp"

static thread_local spvtools::Allocator* tlCustomAllocator = nullptr;

namespace spvtools {
void* FallbackableCustomAllocate(std::size_t size) {
  if (tlCustomAllocator) {
    return tlCustomAllocator->allocate(tlCustomAllocator, size, /*align=*/8);
  }
  return ::operator new(size);
}

void FallbackableCustomDeallocate(void* ptr, std::size_t size) {
  if (tlCustomAllocator) {
    tlCustomAllocator->deallocate(tlCustomAllocator, ptr, size);
  } else {
    ::operator delete(ptr);
  }
}

void Optimizer::SetAllocator(Allocator* allocator) {
  tlCustomAllocator = allocator;
}

}  // namespace spvtools