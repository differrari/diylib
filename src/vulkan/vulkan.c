#include <vulkan/vulkan_core.h>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <vulkan/vulkan.h>
#endif
#include "graph_backend.h"
// #include "syscalls/syscalls.h"
#include <stdio.h>

void graph_render(draw_ctx *ctx){
    // print("TODO: render loop here");
}

#define vkcheck(cond,name) if ((cond) != VK_SUCCESS){ puts("Failed " name); return; }

extern const char** window_make_vk_extensions(uint32_t *amount);

VkInstance instance = {};

VkResult gvk_make_instance(const char **extensions, u32 count){
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Vulc",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "None",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0,
    };

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = count,
        .ppEnabledExtensionNames = extensions,
    };
    return vkCreateInstance(&createInfo, 0, &instance);
}

VkResult gvk_setup_extensions(){//TODO: stub
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(0, &extensionCount, 0);
    
    printf("Vulkcan instance succeded with %i extensions\n",extensionCount);

    VkExtensionProperties extensionProps[extensionCount] = {};

    vkEnumerateInstanceExtensionProperties(0, &extensionCount, extensionProps);

    for (int i = 0; i < extensionCount; i++)
        printf("%s\n",extensionProps[i].extensionName);

    return VK_SUCCESS;
}

VkPhysicalDevice pDevice = {};

VkResult gvk_select_pdevice(){
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, 0);

    VkPhysicalDevice pDevices[deviceCount] = {};
    vkEnumeratePhysicalDevices(instance, &deviceCount, pDevices);

    for (int i = 0; i < deviceCount; i++){
        VkPhysicalDeviceProperties props = {};
        vkGetPhysicalDeviceProperties(pDevices[i], &props);
        VkPhysicalDeviceFeatures feats = {};
        vkGetPhysicalDeviceFeatures(pDevices[i], &feats);
        if (feats.geometryShader){//TODO: we can choose discreet vs integrated device if we prompt the dev for a choice
            printf("Selected Device: %s\n",props.deviceName);
            pDevice = pDevices[i];
            return VK_SUCCESS;
        }
    }
    return VK_ERROR_UNKNOWN;
}

VkResult gvk_filter_extensions(){//TODO: filter all extensions with the ones existing in the device
    uint32_t dExtensionCount;
    vkEnumerateDeviceExtensionProperties(pDevice, 0, &dExtensionCount, 0);

    VkExtensionProperties dExtensionProps[dExtensionCount] = {};
    vkEnumerateDeviceExtensionProperties(pDevice, 0, &dExtensionCount, dExtensionProps);
    
    for (int i = 0; i < dExtensionCount; i++){
        printf("Device EXT: %s\n",dExtensionProps[i].extensionName);
    }
    return VK_SUCCESS;
}

int graphQueue = -1;
VkResult gvk_find_queues(){
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &queueFamilyCount, 0);
    
    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &queueFamilyCount, queueFamilies);

    for (int i = 0; i < queueFamilyCount; i++){
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
            graphQueue = i;
        }
    }

    if (graphQueue < 0){
        printf("Did not select a valid gpu, should've made better checks, mb\n");
        return VK_ERROR_UNKNOWN;
    }

    return VK_SUCCESS;
}

VkDevice lDevice = {};
VkQueue graphicsQueue = {};

VkResult gvk_make_ldevice(){
    VkDeviceQueueCreateInfo queueCreateInfo[2] = {
        {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = graphQueue,
            .queueCount = 1,
        },
        {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = graphQueue,//NOTE: assuming this is the same queue as the graph, it's the presentation queue
            .queueCount = 1,
        }
    };

    VkPhysicalDeviceFeatures enabledVk10Features = {
        .samplerAnisotropy = VK_TRUE
    };

    const char *hardcoded_ext = "VK_KHR_swapchain";//TODO: This is hardcoded

    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = queueCreateInfo,
        .queueCreateInfoCount = 2,
        .pEnabledFeatures = &enabledVk10Features,
        .enabledExtensionCount = 1,
        .enabledLayerCount = 0,
        .ppEnabledExtensionNames = &hardcoded_ext,
    };

    VkResult res = vkCreateDevice(pDevice, &deviceCreateInfo, 0, &lDevice);
    if (res != VK_SUCCESS) return res;

    vkGetDeviceQueue(lDevice, graphQueue, 0, &graphicsQueue);

    return res;
}

void graph_init(){
    uint32_t amount = 0;
    const char **exts = window_make_vk_extensions(&amount);
    if (amount){
        vkcheck(gvk_make_instance(exts, amount),"instance creation");
        vkcheck(gvk_setup_extensions(),"extension enumeration");
        vkcheck(gvk_select_pdevice(), "physical device selection");
        vkcheck(gvk_filter_extensions(), "filter extensions");
        vkcheck(gvk_find_queues(), "finding queues");
        vkcheck(gvk_make_ldevice(), "making logical device");
        puts("Vulkan init done");
    }
}

void graph_make_viewport(u32 w, u32 h){
    puts("TODO: surface creation here");
}

void graph_resize_viewport(draw_ctx *ctx, u32 w, u32 h){
    puts("TODO: resize surface here");
}