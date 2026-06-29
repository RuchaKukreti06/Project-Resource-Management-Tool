#pragma once

#include <unordered_map>
#include <vector>

#include "IUserRepository.h"
#include "User.h"

class MockUserRepository : public IUserRepository
{
   public:
    MockUserRepository() = default;

    bool createUser(const User& user) override
    {
        if (usernameIndex_.find(user.username) != usernameIndex_.end())
        {
            return false;
        }

        User stored = user;
        stored.id = nextId_++;

        usersById_[stored.id] = stored;
        usernameIndex_[stored.username] = stored.id;

        return true;
    }

    User getUserById(int id) override
    {
        auto it = usersById_.find(id);
        return it == usersById_.end() ? User{} : it->second;
    }

    User getUserByUsername(const std::string& username) override
    {
        auto it = usernameIndex_.find(username);

        if (it == usernameIndex_.end())
        {
            return User{};
        }

        return usersById_[it->second];
    }

    User getUserByEmail(const std::string& email) override
    {
        for (const auto& pair : usersById_)
        {
            if (pair.second.email == email)
            {
                return pair.second;
            }
        }
        return User{};
    }

    std::vector<User> getAllUsers() override
    {
        std::vector<User> users;

        users.reserve(usersById_.size());

        for (const auto& pair : usersById_)
        {
            users.push_back(pair.second);
        }

        return users;
    }

    bool updateUser(const User& user) override
    {
        auto it = usersById_.find(user.id);

        if (it == usersById_.end())
        {
            return false;
        }

        usersById_[user.id] = user;
        usernameIndex_[user.username] = user.id;

        return true;
    }

    bool deleteUser(int id) override
    {
        auto it = usersById_.find(id);

        if (it == usersById_.end())
        {
            return false;
        }

        usernameIndex_.erase(it->second.username);
        usersById_.erase(it);

        return true;
    }

    bool updatePassword(int id, const std::string& passwordHash) override
    {
        auto it = usersById_.find(id);

        if (it == usersById_.end())
        {
            return false;
        }

        it->second.passwordHash = passwordHash;

        return true;
    }

    bool setUserStatus(int id, const std::string& status) override
    {
        auto it = usersById_.find(id);

        if (it == usersById_.end())
        {
            return false;
        }

        it->second.status = status;

        return true;
    }

    bool setForcePasswordChange(int id, bool forcePasswordChange) override
    {
        auto it = usersById_.find(id);

        if (it == usersById_.end())
        {
            return false;
        }

        it->second.forcePasswordChange = forcePasswordChange;

        return true;
    }

    bool assignManager(int userId, int managerId) override
    {
        auto it = usersById_.find(userId);

        if (it == usersById_.end())
        {
            return false;
        }

        it->second.managerId = managerId;

        return true;
    }

   private:
    std::unordered_map<int, User> usersById_;
    std::unordered_map<std::string, int> usernameIndex_;
    int nextId_ = 1;
};
