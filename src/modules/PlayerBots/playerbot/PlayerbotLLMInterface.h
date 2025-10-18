#include <atomic>
#include <string>
#include <vector>

class PlayerbotLLMInterface
{
public:
    PlayerbotLLMInterface() {}
    static std::string SanitizeForJson(const std::string& input);

    static std::string Generate(const std::string& prompt, int timeOutSeconds, int maxGenerations, std::vector<std::string>& debugLines);

    static std::vector<std::string> ParseResponse(const std::string& response, const std::string& startPattern, const std::string& endPattern, const std::string& deletePattern, const std::string& splitPattern, std::vector<std::string>& debugLines);

    static void LimitContext(std::string& context, int currentLength);
private:
    std::atomic<int> generationCount = 0;
};

#define sPlayerbotLLMInterface MaNGOS::Singleton<PlayerbotLLMInterface>::Instance()

