#pragma once
#include"config.h"


namespace vkLogging {

    class Logger {
    public:
        static Logger* logger;
        static Logger* get_logger();
        void set_debug_mode(bool mode);
        bool get_debug_mode();
        //void log_device_properties(vk::PhysicalDevice physical_device);
        void print(std::string message);
        void print_list(std::vector<std::string> items);
        //void log_device_properties(vk::PhysicalDevice device);
        //void log_surface_capabilities(vk::SurfaceCapabilitiesKHR surfaceCapabilities);
        //void log_surface_format(vk::SurfaceFormatKHR surfaceFormat);
    private:
        //std::array<int, 4> extract_version_number;
        //std::string extract_driver_version_nvidia;
        //std::string extract_driver_version_intel;
        //std::string extract_driver_version_standard;
        //void log_physical_device_limits(vk::PhysicalDeviceLimits limits);
        //std::vector<std::string> log_transform_bits(vk::SurfaceTransformFlagsKHR bits);
        //std::vector<std::string> log_alpha_composite_bits(vk::CompositeAlphaFlagsKHR bits);
        //std::vector<std::string> log_image_usage_bits(vk::ImageUsageFlags bits);
        //std::string log_present_mode(vk::PresentModeKHR presentMode);
        bool debugMode;
    };
};

namespace vkInit {


    
/*
* Debug call back:
* 
* 
*   typedef enum VkDebugUtilsMessageSeverityFlagBitsEXT {
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT = 0x00000001,
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT = 0x00000010,
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT = 0x00000100,
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT = 0x00001000,
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT = 0x7FFFFFFF
    } VkDebugUtilsMessageSeverityFlagBitsEXT;

   *
	typedef struct VkDebugUtilsMessengerCallbackDataEXT {
    VkStructureType                              sType;
    const void*                                  pNext;
    VkDebugUtilsMessengerCallbackDataFlagsEXT    flags;
    const char*                                  pMessageIdName;
    int32_t                                      messageIdNumber;
    const char*                                  pMessage;
    uint32_t                                     queueLabelCount;
    const VkDebugUtilsLabelEXT*                  pQueueLabels;
    uint32_t                                     cmdBufLabelCount;
    const VkDebugUtilsLabelEXT*                  pCmdBufLabels;
    uint32_t                                     objectCount;
    const VkDebugUtilsObjectNameInfoEXT*         pObjects;
 VkDebugUtilsMessengerCallbackDataEXT;
*/

    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    );

    vk::DebugUtilsMessengerEXT make_debug_messenger(vk::Instance& instance, vk::DispatchLoaderDynamic& dldi);
    std::vector<std::string> log_transform_bits(vk::SurfaceTransformFlagsKHR bits);
    
    std::vector<std::string> log_alpha_composite_bits(vk::CompositeAlphaFlagsKHR bits);
   

    std::vector<std::string> log_image_usage_bits(vk::ImageUsageFlags bits);
      

    std::string log_present_mode(vk::PresentModeKHR presentMode);
    void log_device_properties(const vk::PhysicalDevice& device);

}