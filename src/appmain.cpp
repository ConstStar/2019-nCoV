#include "Fun.hpp"

#include <CQEVE.h>
#include <CQLogger.h>
#include <CQAPI.h>
#include <CQAPI_EX.h>
#include <CQEVE_ALL.h>

#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/widgets/checkbox.hpp>
#include <nana/gui/msgbox.hpp>

#include "APPINFO.h"

#include <string>

using namespace std;
using namespace CQ;

//请加上static,表示这个logger只有本cpp有效
static Logger logger("疫情查询");


//初始化
void init()
try
{
	GroupList.clear();
	QQList.clear();

	Json::Value root;
	Json::Reader reader;
	ifstream file(appPath + "conf.json");

	reader.parse(file, root);
	isQQ = root["QQ"].asBool();
	isGroup = root["Group"].asBool();

	for (auto temp : root["QQList"])
	{
		QQList.push_back(std::stoll(temp.asString()));
	}
	for (auto temp : root["GroupList"])
	{
		GroupList.push_back(std::stoll(temp.asString()));
	}
	file.close();
}
catch (exception & e)
{
	logger.Error((string("设置读取失败 原因:") + e.what()).c_str());
}
catch (...)
{
	logger.Error("设置读取失败 未知原因");
}



EVE_Enable(Enable)
{
	appPath = CQ::getAppDirectory();
	init();

	logger.Info("应用被启用");

	return 0;
}

EVE_Disable(Disable)
{
	logger.Info("应用被停用");
	return 0;
}
MUST_AppInfo_RETURN(CQAPPID)



EVE_PrivateMsg(PrivateMsg)
{
	if (isQQ)
		if (find(QQList.begin(), QQList.end(), fromQQ) == QQList.end())
		{
			return 0;
		}


	std::function<void(string)> send = [=](string msg)
	{
		CQ::sendPrivateMsg(fromQQ, msg);
	};


	MsgFun(msg, send);

	return 0;
}

EVE_GroupMsg(GroupMsg)
{
	if (isGroup)
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




char* Ansi2Unicode(const char* str)
{
	int dwUnicodeLen = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
	if (!dwUnicodeLen)
	{
		return _strdup(str);
	}
	size_t num = dwUnicodeLen * sizeof(wchar_t);
	wchar_t* pwText = (wchar_t*)malloc(num);
	memset(pwText, 0, num);
	MultiByteToWideChar(CP_ACP, 0, str, -1, pwText, dwUnicodeLen);
	return (char*)pwText;
}

char* Unicode2Utf8(const char* unicode)
{
	int len;
	len = WideCharToMultiByte(CP_UTF8, 0, (const wchar_t*)unicode, -1, NULL, 0, NULL, NULL);
	char* szUtf8 = (char*)malloc(len + 1);
	memset(szUtf8, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, (const wchar_t*)unicode, -1, szUtf8, len, NULL, NULL);
	return szUtf8;
}


char* tr(const char* str)
{
	char* unicode = Ansi2Unicode(str);
	char* utf8 = Unicode2Utf8(unicode);
	free(unicode);
	return utf8;
}



//分行
vector<string> text2vector(string text)
{
	vector<string> ret;
	string buf;

	for (auto temp : text)
	{
		if (temp == '\r')
			continue;

		if (temp == '\n')
		{
			if (!buf.empty())
				ret.push_back(buf);
			buf.clear();
			continue;
		}

		buf += temp;
	}

	if (!buf.empty())
		ret.push_back(buf);

	return ret;
}


EVE_Menu(__menu)
{

	using namespace nana;
	form fm(API::make_center(350, 250));
	fm.caption(tr("设置（每行一个号码）"));       // (with this title)
	place  fm_place{ fm };                    // have automatic layout
	button btn{ fm,tr("保存") };                // and a button
	textbox textQQ{ fm,true };
	textbox textGroup{ fm,true };
	checkbox checkQQ{ fm,tr("私聊只开启设置的QQ") };
	checkbox checkGroup{ fm,tr("群消息只开启设置的群号") };


	//初始化
	auto initGui = [&]()
	{
		checkQQ.check(isQQ);
		checkGroup.check(isGroup);

		string QQStr;
		string GroupStr;
		for (auto temp : QQList)
		{
			QQStr += to_string(temp) + "\r\n";
		}

		for (auto temp : GroupList)
		{
			GroupStr += to_string(temp) + "\r\n";
		}

		textQQ.reset(QQStr);
		textGroup.reset(GroupStr);

		textQQ.editable(checkQQ.checked());
		textGroup.editable(checkGroup.checked());
	};
	initGui();

	btn.events().click([&]()               // now the button know how to respond
		{
			string json;
			Json::Value root;
			Json::FastWriter wrtier;
			root["QQ"] = checkQQ.checked();
			root["Group"] = checkGroup.checked();

			vector<string> QQList = text2vector(textQQ.text());
			vector<string> GroupList = text2vector(textGroup.text());

			for (auto temp : QQList)
			{
				root["QQList"].append(temp);
			}

			for (auto temp : GroupList)
			{
				root["GroupList"].append(temp);
			}

			json = wrtier.write(root);

			ofstream file(appPath + "conf.json");
			file << json;
			file.close();

			init();
			msgbox m(fm, tr("成功"), msgbox::ok);
			m.icon(m.icon_none);
			m << tr("保存成功!");
			auto response = m();

			initGui();
		});

	checkQQ.events().click([&] {
		textQQ.editable(checkQQ.checked());
		});


	checkGroup.events().click([&] {
		textGroup.editable(checkGroup.checked());
		});


	fm_place.div("<vertical <label weight=10%> <text weight=70%> <button>>");
	fm_place["label"] << checkQQ << checkGroup;        // and place the controls there
	fm_place["text"] << textQQ << textGroup;        // and place the controls there
	fm_place["button"] << btn;        // and place the controls there
	fm_place.collocate();                      // and collocate all in place
	fm.show();
	exec();

	return 0;
}