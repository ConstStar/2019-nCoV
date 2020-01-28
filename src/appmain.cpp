#include "Fun.hpp"

#include <CQEVE.h>
#include <CQLogger.h>
#include <CQAPI.h>
#include <CQAPI_EX.h>
#include <CQEVE_ALL.h>

#include "APPINFO.h"

#include <string>

using namespace std;
using namespace CQ;

//�����static,��ʾ���loggerֻ�б�cpp��Ч
static Logger logger("�����ѯ");


vector<long long> GroupList;
vector<long long> QQList;


EVE_Enable(Enable)
{
	logger.Info("Ӧ�ñ�����");

	QQList.push_back(1164442003);

	GroupList.push_back(686920365);
	GroupList.push_back(537086369);
	GroupList.push_back(674465120);


	return 0;
}

EVE_Disable(Disable)
{
	logger.Info("Ӧ�ñ�ͣ��");
	return 0;
}
MUST_AppInfo_RETURN(CQAPPID)



EVE_PrivateMsg(PrivateMsg)
{

	std::function<void(string)> send = [=](string msg)
	{
		CQ::sendPrivateMsg(fromQQ, msg);
	};


	MsgFun(msg, send);

	return 0;
}

EVE_GroupMsg(GroupMsg)
{
	if (find(GroupList.begin(), GroupList.end(), fromGroup) == GroupList.end())
	{
		return 0;
	}


	std::function<void(string)> send = [=](string msg)
	{
		CQ::sendGroupMsg(fromGroup, msg);
	};

	MsgFun(msg, send);


	return 0;
}

