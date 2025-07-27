#pragma once

#include <unordered_map>
#include <vector>

#include <Windows.h>

#include "User.h"

#include "../Library/ErrorCode.h"

namespace sniperholdem::server
{
	class UserManager
	{
	public:

		static void Init(const UINT32 maxUserCount);
		static void AddUser(char* userID, int clientIndex);

		static UINT32 GetCurrentUserCnt() { return CurrentUserCnt; }
		static UINT32 GetMaxUserCnt() { return MaxUserCnt; }
		static void IncreaseUserCnt() { CurrentUserCnt++; }
		static void DecreaseUserCnt() { CurrentUserCnt = max(--CurrentUserCnt, 0); }

		static UINT32 FindUserIndexByID(char* userID);
		static void DeleteUserInfo(User* user);
		static User* GetUserByIndex(UINT32 clientIndex);


	private:
		static UINT32 MaxUserCnt;
		static UINT32 CurrentUserCnt;

		static std::vector<User*> UserObjPool;
		static std::unordered_map<std::string, int> UserIDDictionary;

	};
}
