/*
 * Copyright 2020 The TensorFlow Runtime Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//===- dataset.h ------------------------------------------------*- C++ -*-===//
//
// This file declares abstract classes needed by the data pipeline library.
//
//===----------------------------------------------------------------------===//

#ifndef TFRT_LIB_DATA_DATASET_H_
#define TFRT_LIB_DATA_DATASET_H_

#include <memory>

#include "tfrt/host_context/execution_context.h"
#include "tfrt/host_context/host_allocator.h"
#include "tfrt/host_context/host_context.h"
#include "tfrt/support/forward_decls.h"
#include "tfrt/support/rc_array.h"
#include "tfrt/support/ref_count.h"

namespace tfrt {
namespace data {

struct IterationResult {
  // Construct IterationResult with valid values and eof = false.
  static IterationResult Values(SmallVector<RCReference<AsyncValue>, 4> values,
                                HostContext* host) {
    return IterationResult(std::move(values),
                           host->MakeAvailableAsyncValueRef<bool>(false));
  }

  // Construct IterationResult with eof = true. `values` will have error.
  static IterationResult Eof(HostContext* host, size_t num_values) {
    SmallVector<RCReference<AsyncValue>, 4> values;
    values.resize(num_values);
    auto error = host->MakeErrorAsyncValueRef("iterator reached end");
    for (size_t i = 0; i < num_values; ++i) {
      values[i] = error.CopyRef();
    }
    return IterationResult(std::move(values),
                           host->MakeAvailableAsyncValueRef<bool>(true));
  }

  // Construct IterationResult with error in both `values` and `eof`.
  static IterationResult Error(RCReference<AsyncValue> error,
                               size_t num_values) {
    SmallVector<RCReference<AsyncValue>, 4> values;
    values.resize(num_values);
    for (size_t i = 0; i < num_values; ++i) {
      values[i] = error.CopyRef();
    }
    return IterationResult(std::move(values),
                           AsyncValueRef<bool>(error.CopyRef()));
  }

  static IterationResult Pending(SmallVector<RCReference<AsyncValue>, 4> values,
                                 AsyncValueRef<bool> eof) {
    return IterationResult(std::move(values), std::move(eof));
  }

  IterationResult(llvm::SmallVector<RCReference<AsyncValue>, 4> v,
                  AsyncValueRef<bool> e)
      : values(std::move(v)), eof(std::move(e)) {}

  IterationResult(IterationResult&) = delete;
  IterationResult& operator=(IterationResult&) = delete;
  IterationResult(IterationResult&&) = default;
  IterationResult& operator=(IterationResult&&) = default;

  IterationResult CopyRef() const {
    auto copy = llvm::map_range(values, [](auto& v) { return v.CopyRef(); });
    return {{copy.begin(), copy.end()}, eof.CopyRef()};
  }

  llvm::SmallVector<AsyncValue*, 8> AsyncValues() const {
    auto av = llvm::map_range(values, [](auto& v) { return v.get(); });
    llvm::SmallVector<AsyncValue*, 8> result(av.begin(), av.end());
    result.push_back(eof.GetAsyncValue());
    return result;
  }

  // When these AsyncValues resolve, there are three possible states:
  //  (A) End of iteration: `eof` will be true, `values` will contain error
  //      values.
  //  (B) Error: both `eof` and `values` will contain error values.
  //  (C) Success: `eof` should be false; `values` will contain the values from
  //      iteration.
  llvm::SmallVector<RCReference<AsyncValue>, 4> values;
  AsyncValueRef<bool> eof;
};

namespace internal {

template <typename SubClass>
void DestroyImpl(SubClass* ptr, HostAllocator* allocator) {
  ptr->~SubClass();
  allocator->DeallocateBytes(ptr, sizeof(SubClass));
}

bool IsConcreteAndEmpty(const IterationResult& result);

template <typename... T, size_t... I>
static void AllocateTupleResult(
    MutableArrayRef<RCReference<AsyncValue>> results, HostContext* host,
    std::index_sequence<I...>) {
  std::ignore = std::initializer_list<int>{
      (results[I] = host->MakeUnconstructedAsyncValueRef<T>().ReleaseRCRef(),
       0)...};
}

}  // namespace internal

class Iterator : public ReferenceCounted<Iterator> {
 public:
  explicit Iterator() {}

  virtual ~Iterator() {}

  virtual IterationResult GetNext(const ExecutionContext& exec_ctx) = 0;

 protected:
  // For access to Destroy().
  friend class ReferenceCounted<Iterator>;
  virtual void Destroy() = 0;
};

// TODO(rachelim): Define `DatasetContext` and `IteratorContext` as a container
// for common arguments to dataset and iterator constructors respectively.
class Dataset : public ReferenceCounted<Dataset> {
 public:
  virtual ~Dataset() {}

  // Creates an iterator that points to the first element of the dataset.
  // The iterator should keep +1 reference to the parent_dataset.
  virtual RCReference<Iterator> MakeIterator() = 0;

 private:
  // For access to Destroy().
  friend class ReferenceCounted<Dataset>;

  virtual void Destroy() = 0;
};

}  // namespace data
}  // namespace tfrt

#endif  // TFRT_LIB_DATA_DATASET_H_
