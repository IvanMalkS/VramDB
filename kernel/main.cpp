#include <vulkan/vulkan.h>
#include <iostream>
#include <cstring>
#include <map>
#include <stdexcept>

struct KeyValue {
    char key[32];   // Maximum string length to save a key
    char value[1024]; // Maximum string length to save a value
};

class GPUKeyValueDB {
public:
    GPUKeyValueDB() {
        // initializing Vulkan
        instance = createInstance();
        physicalDevice = pickPhysicalDevice(instance);
        device = createLogicalDevice(physicalDevice);
    }

    ~GPUKeyValueDB() {
        cleanup();
    }

    void create(const std::string& key, const std::string& value) {
        if (key.size() >= 32 || value.size() >= 64) {
            std::cerr << "Key or value too large" << std::endl;
            return;
        }

        KeyValue keyValue;
        strcpy(keyValue.key, key.c_str());
        strcpy(keyValue.value, value.c_str());

        VkDeviceSize bufferSize = sizeof(KeyValue);
        VkBuffer buffer = createBuffer(bufferSize);
        VkDeviceMemory bufferMemory = allocateBufferMemory(buffer);

        writeToBuffer(bufferMemory, keyValue, bufferSize);

        data[key] = {buffer, bufferMemory};
        std::cout << "Entry created: " << key << " = " << value << std::endl;
    }

    void update(const std::string& key, const std::string& newValue) {
        if (data.find(key) == data.end()) {
            std::cerr << "Key not found" << std::endl;
            return;
        }

        KeyValue keyValue;
        strcpy(keyValue.key, key.c_str());
        strcpy(keyValue.value, newValue.c_str());

        VkDeviceMemory bufferMemory = data[key].second;
        writeToBuffer(bufferMemory, keyValue, sizeof(KeyValue));

        std::cout << "Entry updated: " << key << " = " << newValue << std::endl;
    }

    void remove(const std::string& key) {
        if (data.find(key) == data.end()) {
            std::cerr << "Key not found" << std::endl;
            return;
        }

        vkDestroyBuffer(device, data[key].first, nullptr);
        vkFreeMemory(device, data[key].second, nullptr);
        data.erase(key);

        std::cout << "Entry deleted: " << key << std::endl;
    }

    void show() {
        for (const auto& entry : data) {
            printBuffer(entry.second.second, sizeof(KeyValue));
        }
    }

private:
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    std::map<std::string, std::pair<VkBuffer, VkDeviceMemory>> data;

    // Vulcan initialization
    VkInstance createInstance() {
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "GPU DB";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        VkInstance instance;
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan instance");
        }
        return instance;
    }

    VkPhysicalDevice pickPhysicalDevice(VkInstance instance) {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            throw std::runtime_error("Failed to find GPUs with Vulkan support");
        }

        VkPhysicalDevice devices[deviceCount];
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices);
        return devices[0];
    }

    VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice) {
        VkDeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        VkDevice device;
        if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create logical device");
        }
        return device;
    }

    VkBuffer createBuffer(VkDeviceSize size) {
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkBuffer buffer;
        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create buffer");
        }
        return buffer;
    }

    VkDeviceMemory allocateBufferMemory(VkBuffer buffer) {
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits,
                                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VkDeviceMemory bufferMemory;
        if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate buffer memory");
        }

        vkBindBufferMemory(device, buffer, bufferMemory, 0);
        return bufferMemory;
    }

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("Failed to find suitable memory type");
    }

    void writeToBuffer(VkDeviceMemory bufferMemory, const KeyValue& keyValue, VkDeviceSize size) {
        void* mappedData;
        vkMapMemory(device, bufferMemory, 0, size, 0, &mappedData);
        memcpy(mappedData, &keyValue, size);
        vkUnmapMemory(device, bufferMemory);
    }

    void printBuffer(VkDeviceMemory bufferMemory, VkDeviceSize bufferSize) {
        void* data;
        vkMapMemory(device, bufferMemory, 0, bufferSize, 0, &data);

        KeyValue keyValue;
        memcpy(&keyValue, data, sizeof(KeyValue));

        std::cout << "Key: " << keyValue.key << ", Value: " << keyValue.value << std::endl;

        vkUnmapMemory(device, bufferMemory);
    }

    void cleanup() {
        for (auto& entry : data) {
            vkDestroyBuffer(device, entry.second.first, nullptr);
            vkFreeMemory(device, entry.second.second, nullptr);
        }
        vkDestroyDevice(device, nullptr);
        vkDestroyInstance(instance, nullptr);
    }
};

// Command gui to work with gpu
void runCommandLoop(GPUKeyValueDB& db) {
    std::string command;
    while (true) {
        std::cout << "Enter command (create/update/delete/show/exit): ";
        std::cin >> command;

        if (command == "create") {
            std::string key, value;
            std::cout << "Enter key: ";
            std::cin >> key;
            std::cout << "Enter value: ";
            std::cin >> value;
            db.create(key, value);
        } else if (command == "update") {
            std::string key, value;
            std::cout << "Enter key: ";
            std::cin >> key;
            std::cout << "Enter new value: ";
            std::cin >> value;
            db.update(key, value);
        } else if (command == "delete") {
            std::string key;
            std::cout << "Enter key: ";
            std::cin >> key;
            db.remove(key);
        } else if (command == "show") {
            db.show();
        } else if (command == "exit") {
            break;
        } else {
            std::cout << "Unknown command" << std::endl;
        }
    }
}

int main() {
    GPUKeyValueDB db;
    runCommandLoop(db);
    return 0;
}
