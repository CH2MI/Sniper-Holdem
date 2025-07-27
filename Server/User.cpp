#include "User.h"

namespace sniperholdem::server
{
	User::User() :
		mIndex(-1),
		mUserID{},
		mBuffer{}
	{
	}

	User::~User()
	{
	}

	void User::Clear()
	{
		mUserID = "";
		mBuffer.Clear();
	}
}