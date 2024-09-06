#include <DEngine.h>

namespace DE::VK {

vk::Instance MakeInstance(std::string name);

vk::DebugUtilsMessengerEXT MakeDebugMessenger(const vk::Instance &vkInstance, vk::DispatchLoaderDynamic dispatch);

} // namespace DE::VK