#include "UserManager.h"

namespace sniperholdem::server
{
	UINT32 UserManager::MaxUserCnt = 0;
	UINT32 UserManager::CurrentUserCnt = 0;

	std::vector<User*> UserManager::UserObjPool{};
	std::unordered_map<std::string, int> UserManager::UserIDDictionary{};

	void UserManager::Init(const UINT32 maxUserCount)
	{
		MaxUserCnt = maxUserCount;
		UserObjPool = std::vector<User*>(MaxUserCnt);

		for (size_t i = 0; i < MaxUserCnt; i++)
		{
			UserObjPool[i] = new User();
			UserObjPool[i]->SetIndex(i);
		}
	}

	void UserManager::AddUser(char* userID, int clientIndex)
	{
		UserObjPool[clientIndex]->SetUserID(userID);
		UserIDDictionary.insert({ userID, clientIndex });
	}

	UINT32 UserManager::FindUserIndexByID(char* userID)
	{
		auto itr = UserIDDictionary.find(userID);
		if (itr == UserIDDictionary.end())
			return UINT32_MAX;

		return itr->second;
	}

	void UserManager::DeleteUserInfo(User* user)
	{
		UserIDDictionary.erase(user->GetUserID());
		user->Clear();
	}

	User* UserManager::GetUserByIndex(UINT32 clientIndex)
	{
		return UserObjPool[clientIndex];
	}
}