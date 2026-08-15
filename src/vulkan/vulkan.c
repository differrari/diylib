#include <vulkan/vulkan_core.h>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <vulkan/vulkan.h>
#endif
#include "graph_backend.h"
#include "syscalls/syscalls.h"

#define vkcheck(cond,name) do { VkResult res = (cond); if (res != VK_SUCCESS){ print("Failed " name " with error %i",res); return; } } while (0);

extern const char** window_make_vk_extensions(uint32_t *amount);

VkInstance instance = {};

bool gvk_debug = true;

VKAPI_ATTR VkBool32 gvk_debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    print("[VKValidation %x]: %s",messageSeverity,pCallbackData->pMessage);
    return VK_FALSE;
}

VkResult gvk_make_instance(const char **extensions, u32 count){
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Vulc",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "None",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0,
    };

    const char *hardcoded_vlay = "VK_LAYER_KHRONOS_validation";

    VkDebugUtilsMessengerCreateInfoEXT debugInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = gvk_debugCallback,
    };

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = count,
        .ppEnabledExtensionNames = extensions,
        .enabledLayerCount = 1,
        .ppEnabledLayerNames = &hardcoded_vlay,
        .pNext = &debugInfo
    };
    return vkCreateInstance(&createInfo, 0, &instance);
}

VkResult gvk_setup_extensions(){//TODO: the layer part here needs to happen before, also the extension part is a stub
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(0, &extensionCount, 0);
    
    print("Vulkcan instance succeded with %i extensions\n",extensionCount);

    VkExtensionProperties extensionProps[extensionCount] = {};

    vkEnumerateInstanceExtensionProperties(0, &extensionCount, extensionProps);

    for (u32 i = 0; i < extensionCount; i++)
        print("%s\n",extensionProps[i].extensionName);

    u32 layer_count = 0;
    vkEnumerateInstanceLayerProperties(&layer_count, 0);

    VkLayerProperties pProperties[layer_count] = {};

    vkEnumerateInstanceLayerProperties(&layer_count, pProperties);

    for (u32 i = 0; i < layer_count; i++)
        print("%s\n",pProperties[i].layerName);

    return VK_SUCCESS;
}

VkPhysicalDevice pDevice = {};

VkResult gvk_select_pdevice(){
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, 0);

    VkPhysicalDevice pDevices[deviceCount] = {};
    vkEnumeratePhysicalDevices(instance, &deviceCount, pDevices);

    for (u32 i = 0; i < deviceCount; i++){
        VkPhysicalDeviceProperties props = {};
        vkGetPhysicalDeviceProperties(pDevices[i], &props);
        VkPhysicalDeviceFeatures feats = {};
        vkGetPhysicalDeviceFeatures(pDevices[i], &feats);
        if (feats.geometryShader){//TODO: we can choose discreet vs integrated device if we prompt the dev for a choice
            print("Selected Device: %s\n",props.deviceName);
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
    
    // for (u32 i = 0; i < dExtensionCount; i++){
    //     print("Device EXT: %s\n",dExtensionProps[i].extensionName);
    // }
    return VK_SUCCESS;
}

int graphQueueIndex = -1;
int presentQueueIndex = -1;
VkResult gvk_find_queues(){
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &queueFamilyCount, 0);
    
    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &queueFamilyCount, queueFamilies);

    for (int i = 0; i < queueFamilyCount; i++){
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
            graphQueueIndex = i;
        }
        if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT){
            presentQueueIndex = i;
        }
    }

    if (graphQueueIndex < 0 || presentQueueIndex < 0){
        print("Did not select queues, should've made better checks, mb\n");
        return VK_ERROR_UNKNOWN;
    }

    return VK_SUCCESS;
}

VkDevice lDevice = {};
VkQueue graphQueue = {}, presentQueue = {};

VkResult gvk_make_ldevice(){
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo[2] = {
        {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = graphQueueIndex,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority,
        },
        {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = presentQueueIndex,//NOTE: assuming this is the same queue as the graph, it's the presentation queue
            .queueCount = 1,
            .pQueuePriorities = &queuePriority,
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
        .ppEnabledExtensionNames = &hardcoded_ext,
    };

    VkResult res = vkCreateDevice(pDevice, &deviceCreateInfo, 0, &lDevice);
    if (res != VK_SUCCESS) return res;

    vkGetDeviceQueue(lDevice, graphQueueIndex, 0, &graphQueue);
    vkGetDeviceQueue(lDevice, presentQueueIndex, 0, &presentQueue);

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
        print("Vulkan init done");
    } else 
        print("Vulkan failed to get window extensions");
}

VkSurfaceKHR surface;

extern VkResult window_make_vk_surface(VkInstance instance, VkSurfaceKHR *surface);

VkResult gvk_can_present(){
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(pDevice, graphQueueIndex, surface, &presentSupport);

    return presentSupport ? VK_SUCCESS : VK_ERROR_FEATURE_NOT_PRESENT;
}

VkExtent2D surfaceExtent = {};
VkSwapchainKHR swapChain = {};

VkResult gvk_create_swapchain(){
    VkSwapchainCreateInfoKHR scCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = 3,//TODO: this is based on capabilities
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .imageFormat = VK_FORMAT_B8G8R8A8_SRGB,
        .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        .imageExtent = surfaceExtent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .queueFamilyIndexCount = 2,
        .pQueueFamilyIndices = (const u32[]){graphQueueIndex,presentQueueIndex}
    };
    
    return vkCreateSwapchainKHR(lDevice, &scCreateInfo, 0, &swapChain);
}

VkImageView imgViews[8] = {};//TODO: This should be allocated, but the allocator is causing some weird issues right now so we're hardcoding
u32 imageCount = 0;

VkResult gvk_make_swapchain_imageviews(){
    vkGetSwapchainImagesKHR(lDevice, swapChain, &imageCount, 0);
    VkImage swapChainImages[imageCount] = {};
    vkGetSwapchainImagesKHR(lDevice, swapChain, &imageCount, swapChainImages);

    for (u32 i = 0; i < imageCount; i++){
        VkImageViewCreateInfo icreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = swapChainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = VK_FORMAT_B8G8R8A8_SRGB,
            .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .subresourceRange.baseMipLevel = 0,
            .subresourceRange.levelCount = 1,
            .subresourceRange.baseArrayLayer = 0,
            .subresourceRange.layerCount = 1,
        };

        if (vkCreateImageView(lDevice, &icreateInfo, 0, &imgViews[i]) != VK_SUCCESS){
            return VK_ERROR_UNKNOWN;
        }
    }
    return VK_SUCCESS;
}

VkCommandPool commandPool = {};
VkCommandBuffer commandBuffers[8] = {};//TODO: alloc

VkResult gvk_make_command_pool_buffer(){
    VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = graphQueueIndex,
    };
    VkResult res = vkCreateCommandPool(lDevice, &poolInfo, 0, &commandPool);
    if (res != VK_SUCCESS) return res;
    
    for (u32 i = 0; i < imageCount; i++){
        VkCommandBufferAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        
        VkResult res = vkAllocateCommandBuffers(lDevice, &allocInfo, &commandBuffers[i]);
        if (res != VK_SUCCESS) return res;

    }

    return VK_SUCCESS;
}

VkPipelineLayout pipelineLayout = {};
VkPipeline pipeline = {};
VkRenderPass renderPass = {};
VkFramebuffer framebuffers[8];//TODO: allocate

VkResult gvk_make_framebuffers(){
    for (uint32_t i = 0; i < imageCount; i++) {
        VkFramebufferCreateInfo fbci = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            //.renderPass = renderPass,
            .attachmentCount = 1,
            .pAttachments = &imgViews[i],
            .width = surfaceExtent.width,
            .height = surfaceExtent.height,
            .layers = 1,
            .renderPass = renderPass,
        };
    
        VkResult res = vkCreateFramebuffer(lDevice, &fbci, 0, &framebuffers[i]);
        if (res != VK_SUCCESS) return res;
    }

    return VK_SUCCESS;
}

void graph_make_viewport(u32 w, u32 h){
    vkcheck(window_make_vk_surface(instance, &surface), "creating surface");
    surfaceExtent = (VkExtent2D){ w, h};
    vkcheck(gvk_can_present(), "no presentation support");
    vkcheck(gvk_create_swapchain(), "creating swapchain");
    vkcheck(gvk_make_swapchain_imageviews(), "making swapchain images");
    vkcheck(gvk_make_command_pool_buffer(), "command pool");
    graph_make_pipeline();//TODO: the functions called inside here should be a default CPU-render fallback, the true function should be configurable through the t2d pipeline
    vkcheck(gvk_make_framebuffers(), "making framebuffers");
    print("Surface created with framebuffers and command queue");
}

#include "embedded_shaders.h"

VkResult gvk_make_renderpass(){
    VkShaderModule vertShaderModule = {}, fragShaderModule = {};
    VkShaderModuleCreateInfo vsh = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = _build_shaders_em_vert_spv_len,
        .pCode = (u32*)_build_shaders_em_vert_spv,
    };

    VkResult res = vkCreateShaderModule(lDevice, &vsh, 0, &vertShaderModule);
    if (res != VK_SUCCESS) return res;

    VkShaderModuleCreateInfo fsh = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = _build_shaders_em_frag_spv_len,
        .pCode = (u32*)_build_shaders_em_frag_spv,
    };

    res = vkCreateShaderModule(lDevice, &fsh, 0, &fragShaderModule);
    if (res != VK_SUCCESS){
        vkDestroyShaderModule(lDevice, vertShaderModule, 0);
        return res;
    }

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertShaderModule,
        .pName = "main",
    };

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragShaderModule,
        .pName = "main",
    };

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = 0,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = 0,
    };//TODO: proper vertex input

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float) surfaceExtent.width,
        .height = (float) surfaceExtent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = surfaceExtent,
    };

    VkPipelineViewportStateCreateInfo viewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };

    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1
    };

    VkPipelineLayoutCreateInfo plci = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pSetLayouts = 0,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = 0,
    };
    
    res = vkCreatePipelineLayout(lDevice, &plci, 0, &pipelineLayout);
    if (res != VK_SUCCESS) return res;

    VkAttachmentDescription colorAttachment = {
        .format = VK_FORMAT_B8G8R8A8_SRGB,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentReference colorAttachmentRef = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
    };

    VkRenderPassCreateInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
    };

    res = vkCreateRenderPass(lDevice, &renderPassInfo, 0, &renderPass);
    if (res != VK_SUCCESS) return res;

    VkPipelineMultisampleStateCreateInfo ms = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    };

    VkPipelineColorBlendAttachmentState cbAttach = {
        .colorWriteMask = 
                                    VK_COLOR_COMPONENT_R_BIT | 
                                    VK_COLOR_COMPONENT_G_BIT |
                                    VK_COLOR_COMPONENT_B_BIT | 
                                    VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE
    };
    
    VkPipelineColorBlendStateCreateInfo cb = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments = &cbAttach,
    };

    VkGraphicsPipelineCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &ms,
        .pColorBlendState = &cb,
        .layout = pipelineLayout,
        .renderPass = renderPass,
        // .subpass = 0,
        // .basePipelineHandle = 0,
        // .basePipelineIndex = -1,
    };

    res = vkCreateGraphicsPipelines(lDevice, 0, 1, &createInfo, 0, &pipeline);

    vkDestroyShaderModule(lDevice, fragShaderModule, 0);
    vkDestroyShaderModule(lDevice, vertShaderModule, 0);

    return res;
}

VkSemaphore imageAvailableSemaphore;
VkSemaphore renderFinishedSemaphore;
VkFence inFlightFence;

VkResult gvk_create_sync_objects(){
    VkSemaphoreCreateInfo semaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    VkResult res = vkCreateSemaphore(lDevice, &semaphoreInfo, 0, &imageAvailableSemaphore);
    if (res != VK_SUCCESS) return res;

    res = vkCreateSemaphore(lDevice, &semaphoreInfo, 0, &renderFinishedSemaphore);
    if (res != VK_SUCCESS) return res;

    VkFenceCreateInfo fenceInfo = { 
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    return vkCreateFence(lDevice, &fenceInfo, 0, &inFlightFence);
    //TODO: most of this code needs cleanup
}

//TODO: expose this 
void graph_make_pipeline(){
    vkcheck(gvk_make_renderpass(), "making render pass");
    vkcheck(gvk_create_sync_objects(), "making sync objects");
    print("Pipeline setup done");
}

void graph_resize_viewport(draw_ctx *ctx, u32 w, u32 h){
    print("TODO: resize surface here");
}

VkResult gvk_record_command_buffer(VkCommandBuffer buffer, u32 imageIndex){
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0,
        .pInheritanceInfo = 0 
    };

    return vkBeginCommandBuffer(buffer, &beginInfo);
}

VkResult gvk_begin_render_pass(VkCommandBuffer buffer, u32 imageIndex){
    VkResult res = vkResetCommandBuffer(buffer, 0);
    if (res != VK_SUCCESS) return res;

    res = gvk_record_command_buffer(buffer,imageIndex);
    if (res != VK_SUCCESS) return res;

    VkClearValue clearColor = {{{0.5f, 0.0f, 0.0f, 1.0f}}};
    VkRenderPassBeginInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass,
        .framebuffer = framebuffers[imageIndex],
        .renderArea.offset = {0, 0},
        .renderArea.extent = surfaceExtent,
        .clearValueCount = 1,
        .pClearValues = &clearColor,
    };
    vkCmdBeginRenderPass(buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    vkCmdDraw(buffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(buffer);

    return vkEndCommandBuffer(buffer);
}

VkResult gvk_submit_command(VkCommandBuffer buffer, u32 imageIndex){
    
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &imageAvailableSemaphore,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &buffer,
        .pSignalSemaphores = &renderFinishedSemaphore,
        .signalSemaphoreCount = 1,
    };

    return vkQueueSubmit(graphQueue, 1, &submitInfo, inFlightFence);
}

VkResult gvk_present(u32 imageIndex){
    VkPresentInfoKHR present = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &renderFinishedSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &swapChain,
        .pImageIndices = &imageIndex,
    };
    return vkQueuePresentKHR(presentQueue, &present); 
}

void graph_render(draw_ctx *ctx){
    vkWaitForFences(lDevice, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(lDevice, 1, &inFlightFence);
    u32 imageIndex;
    vkAcquireNextImageKHR(lDevice, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
    vkcheck(gvk_begin_render_pass(commandBuffers[imageIndex], imageIndex), "executing command buffer");
    vkcheck(gvk_submit_command(commandBuffers[imageIndex], imageIndex), "submitting command");
    vkcheck(gvk_present(imageIndex),"presenting");
}