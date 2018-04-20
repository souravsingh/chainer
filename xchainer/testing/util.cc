#include "xchainer/testing/util.h"

#include <atomic>
#include <cassert>
#include <cstdlib>
#include <string>

#include <gtest/gtest.h>

#include "xchainer/backend.h"
#include "xchainer/context.h"
#include "xchainer/error.h"

namespace xchainer {
namespace testing {

namespace {

std::atomic<int> g_skipped_native_test_count{0};
std::atomic<int> g_skipped_cuda_test_count{0};

int GetNativeDeviceLimit(Backend& backend) {
    assert(backend.GetName() == "native");
    static int limit = -1;
    if (limit >= 0) {
        return limit;
    }
    const char* env = std::getenv("XCHAINER_TEST_NATIVE_DEVICE_LIMIT");
    if (env == nullptr) {
        limit = backend.GetDeviceCount();
    } else {
        limit = std::stoi(env);
        if (limit < 0) {
            throw XchainerError{"XCHAINER_TEST_NATIVE_DEVICE_LIMIT must be non-negative integer: ", env};
        }
    }
    return limit;
}

int GetCudaDeviceLimit(Backend& backend) {
    assert(backend.GetName() == "cuda");
    static int limit = -1;
    if (limit >= 0) {
        return limit;
    }
    const char* env = std::getenv("XCHAINER_TEST_CUDA_DEVICE_LIMIT");
    if (env == nullptr) {
        limit = backend.GetDeviceCount();
    } else {
        limit = std::stoi(env);
        if (limit < 0) {
            throw XchainerError{"XCHAINER_TEST_CUDA_DEVICE_LIMIT must be non-negative integer: ", env};
        }
    }
    return limit;
}

}  // namespace

namespace internal {

int GetSkippedNativeTestCount() { return g_skipped_native_test_count; }

int GetSkippedCudaTestCount() { return g_skipped_cuda_test_count; }

int GetDeviceLimit(Backend& backend) {
    if (backend.GetName() == "native") {
        return GetNativeDeviceLimit(backend);
    }
    if (backend.GetName() == "cuda") {
        return GetCudaDeviceLimit(backend);
    }
    throw BackendError{"invalid backend: ", backend.GetName()};
}

bool SkipIfDeviceUnavailable(Backend& backend, int required_num) {
    if (GetDeviceLimit(backend) >= required_num) {
        return false;
    }
    const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
    std::cout << "[     SKIP ] " << test_info->test_case_name() << "." << test_info->name() << std::endl;

    if (backend.GetName() == "native") {
        ++g_skipped_native_test_count;
    } else if (backend.GetName() == "cuda") {
        ++g_skipped_cuda_test_count;
    } else {
        throw BackendError{"invalid backend: ", backend.GetName()};
    }
    return true;
}

bool SkipIfDeviceUnavailable(const std::string& backend_name, int required_num) {
    return SkipIfDeviceUnavailable(GetDefaultContext().GetBackend(backend_name), required_num);
}

}  // namespace internal
}  // namespace testing
}  // namespace xchainer
