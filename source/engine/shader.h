#include <DEngine.h>

namespace DEUtil {

// reads a binary file and returns a (const char *) with file contents.
std::vector<char> ReadBinFile(const char *filepath)
{
    std::ifstream file(filepath, std::ios::ate | std::ios::binary);

    if(!file.is_open())
    {
        LERROR("couldn't open file with path: " << filepath << "\n");
    }

    u64 fileSize = static_cast<u64>(file.tellg());
    file.seekg(0);

    std::vector<char> buff(fileSize);
    file.read(buff.data(), fileSize);

    file.close();
    return buff;
}

vk::ShaderModule CreateShaderModule(const char *filepath, vk::Device device)
{
    std::vector<char> shaderSrc = ReadBinFile(filepath);

    vk::ShaderModuleCreateInfo moduleInfo{};

    moduleInfo.flags = vk::ShaderModuleCreateFlags();
    moduleInfo.codeSize = shaderSrc.size();
    moduleInfo.pCode = reinterpret_cast<const u32 *>(shaderSrc.data());

    try
    {
        return device.createShaderModule(moduleInfo);
    }
    catch(vk::SystemError err)
    {
        LERROR("VULKAN ERROR: couldn't create shader module with path: " << filepath << "\n\t" << err.what() << "\n");
        return nullptr;
    }
}

} // namespace DEUtil