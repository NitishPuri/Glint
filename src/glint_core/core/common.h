#pragma once

#define VK_CHECK_RESULT(f)                                                                                         \
  {                                                                                                                \
    VkResult res = (f);                                                                                            \
    if (res != VK_SUCCESS) {                                                                                       \
        LOG("Fatal : VkResult is \"" ,vks::tools::errorString(res) << "\" in " << __FILE__ << " at line " \
                << __LINE__ << "\n";                                                                               \
      assert(res == VK_SUCCESS);                                                                                   \
    }                                                                                                              \
  }

namespace glint {}