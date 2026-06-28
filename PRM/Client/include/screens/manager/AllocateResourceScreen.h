#ifndef ALLOCATE_RESOURCE_SCREEN_H
#define ALLOCATE_RESOURCE_SCREEN_H

#include "screens/Screen.h"
#include "manager/managerConstants.h"
#include <optional>
#include <vector>
#include <string>
#include "dto/AiResponseDTO.h"
#include "dto/AllocationDTO.h"

class AiClientService;
class AllocationClientService;
class ProjectClientService;
class EmployeeClientService;

class AllocateResourceScreen : public Screen
{
   public:
    AllocateResourceScreen(AiClientService& aiService, AllocationClientService& allocService, ProjectClientService& projService, EmployeeClientService& empService, int currentUserId);
    void show() override;
    void displayMenu() override;
    void handleInput() override;

   private:
    void displayCandidates(const std::vector<AiCandidateDTO>& candidates);
    void printCandidateRow(const AiCandidateDTO& item, int idx);

    void displayActiveAllocations(const std::vector<std::pair<AllocationDTO, std::string>>& allocationsWithName);
    AiClientService& aiService_;
    AllocationClientService& allocService_;
    ProjectClientService& projService_;
    EmployeeClientService& empService_;
    void findResourceAI();
    void allocateDirectly();
    void endAllocation();
    
    std::optional<std::string> promptForAIRequirement();
    std::optional<int> promptForProject(const std::string& promptText);
    
    std::optional<AiCandidateDTO> fetchSkillMatchCandidates(const std::string& reqText);
    std::optional<AiCandidateDTO> displayAndSelectAICandidate(const std::vector<AiCandidateDTO>& candidates);
    void promptForAllocationDetailsAndSave(int empId, int projectId);
    std::optional<int> promptForEmployeeId();
    
    std::optional<AllocationDTO> selectActiveAllocation(int projectId);
    std::vector<std::pair<AllocationDTO, std::string>> fetchAllocationsWithNames(const std::vector<AllocationDTO>& activeAllocs);
    std::optional<AllocationDTO> promptAndReturnAllocation(const std::vector<AllocationDTO>& activeAllocs);

    void confirmAndEndAllocation(int allocId);

    int currentUserId_;
    bool keepRunning_ = true;

   protected:
    ScreenDecorator decorator() const override;
};

#endif
