#include "httplib.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <codecvt>


#include <json/json.h>

using namespace std;

vector<long long> GroupList;
vector<long long> QQList;
bool isQQ = true;
bool isGroup = true;

string appPath;

struct AREA
{
	AREA(string name, int confirmedCount, int deadCount, int curedCount)
		:name(name)
		, confirmedCount(confirmedCount)
		, deadCount(deadCount)
		, curedCount(curedCount)
	{}

	AREA()
		:name("��ѯʧ��,���޴�����")
		, confirmedCount(0)
		, deadCount(0)
		, curedCount(0)
	{}

	string name;
	int confirmedCount;//ȷ��
	int deadCount;//����
	int curedCount;//����
};

bool AREAComp(const AREA& a, const AREA& b)
{
	return a.confirmedCount > b.confirmedCount;
}

class Plague
{

private:

	std::wstring UTF8ToUnicode(const std::string& str)
	{
		std::wstring ret;
		try {
			std::wstring_convert< std::codecvt_utf8<wchar_t> > wcv;
			ret = wcv.from_bytes(str);
		}
		catch (const std::exception & e) {
			//std::cerr << e.what() << std::endl;
		}
		return ret;
	}

	std::string UnicodeToANSI(const std::wstring& wstr)
	{
		string result;
		//��ȡ��������С��������ռ䣬��������С�°��ֽڼ����  
		int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
		char* buffer = new char[len + 1];
		//���ֽڱ���ת���ɶ��ֽڱ���  
		WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len, NULL, NULL);
		buffer[len] = '\0';
		//ɾ��������������ֵ  
		result.append(buffer);
		delete[] buffer;
		return result;
	}

	std::string UTF8ToANSI(const std::string str)
	{
		return UnicodeToANSI(UTF8ToUnicode(str));
	}

	std::string substring(std::string buf, const std::string start, const std::string end)
	{
		int subStart = buf.find(start);
		if (subStart != buf.npos)
		{
			subStart += start.size();
		}
		else
		{
			throw exception("�ָ��ַ���ͷ����");
		}
		buf = buf.substr(subStart);


		int subEnd = buf.find(end);//- subStart;
		if (subEnd == buf.npos)
		{
			throw exception("�ָ��ַ���β����");
		}
		buf = buf.substr(0, subEnd);

		return buf;
	}

	//����ͼƬ ������ͼƬ����
	string downImg(string host, string path)
	{

		httplib::Client client(host, 80);

		auto ret = client.Get(path.c_str());
		if (!ret || ret->status != 200)
		{
			throw exception("ͼƬ����ʧ�� �����쳣");
		}

		string downPath("2019-nCoV_" + path.substr(path.rfind("/") + 1));

		ofstream file("data\\image\\" + downPath, ios::out | ios::binary);
		file << ret->body;
		file.close();
		deleteFile.push_back("data\\image\\" + downPath);

		return downPath;
	}

	//��ȡ����
	bool getData(string& error)
	{
		httplib::Client cli("ncov.dxy.cn", 80);

		auto res = cli.Get("/ncovh5/view/pneumonia");

		if (!res || res->status != 200)
		{
			error = "��ȡ����ʧ�� �����쳣";
		}
		else
		{

			html = UTF8ToANSI(res->body);

			//��������������Ϣ
			string json = substring(html, "<script id=\"getAreaStat\">try { window.getAreaStat = ", "}catch(e){}");
			Json::Reader reader;
			reader.parse(json, root);

			//����������Ϣ
			Json::Reader newReader;
			string newJson = substring(html, "<script id=\"getTimelineService\">try { window.getTimelineService = ", "}catch(e){}");
			newReader.parse(newJson, newsRoot);

			//��������������Ϣ
			Json::Reader abroadReader;
			string abroadJson = substring(html, "<script id=\"getListByCountryTypeService2\">try { window.getListByCountryTypeService2 = ", "}catch(e){}");
			abroadReader.parse(abroadJson, abroadRoot);

			//������Ҫ��Ϣ
			Json::Reader mainReader;
			string mainJson = substring(html, "<script id=\"getStatisticsService\">try { window.getStatisticsService = ", "}catch(e){}");
			mainReader.parse(mainJson, mainRoot);

			return true;
		}

		return false;
	}




public:

	Plague()
	{
		string error;
		for (int i = 0; i < 5; i++)
		{
			if (getData(error))
				break;

			if (i == 4)
			{
				//����
				throw exception(error.c_str());
			}
			Sleep(200);
		}
	}

	~Plague()
	{
		for (auto temp : deleteFile)
		{
			remove(temp.c_str());
		}


	}

	//��ȡ�����ͼ
	string getMap()
	{

		/*
		//http://www.xiaoxiaoge.cn/PlagueMap.json
		httplib::Client cli("www.xiaoxiaoge.cn", 80);
		stringstream buf;

		auto res = cli.Get("/PlagueMap.json");

		if (!res || res->status != 200)
		{
			throw exception("��ȡ��ͼ����ʧ�� �����쳣");
		}
		else
		{
			string json = UTF8ToANSI(res->body);

			Json::Value root;
			Json::Reader reader;
			reader.parse(json, root);

			string host = root["host"].asString();
			string path = root["path"].asString();

			buf << "[CQ:image,file=";
			buf << downImg(host, path);
			buf << "]" << endl;
			buf << "��������: " << root["from"].asString() << endl;
			buf << "����ʱ��: " << root["updataTime"].asString();

			return buf.str();
		}

		return "";

		*/

		stringstream buf;
		string url = mainRoot["imgUrl"].asString();

		string host = substring(html, "https://", "/");
		string path(url.substr(url.find(".com") + strlen(".com")));

		buf << "[CQ:image,file=";
		buf << downImg(host, path);
		buf << "]";
		return buf.str();


	}

	//��ȡ��������ͼ
	string getTrendMap()
	{
		stringstream buf;
		string url = mainRoot["dailyPic"].asString();

		string host = substring(html, "https://", "/");
		string path(url.substr(url.find(".com") + strlen(".com")));

		buf << "[CQ:image,file=";
		buf << downImg(host, path);
		buf << "]";
		return buf.str();
	}

	//��ȡ����
	string getNews()
	{
		string str;
		int id = 0;

		for (auto temp : newsRoot)
		{
			if (id < temp["id"].asInt())
			{
				id = temp["id"].asInt();
				str = temp["title"].asString() + "(" + temp["pubDateStr"].asString() + ")\n";
				str += temp["summary"].asString() + "\n" + temp["provinceName"].asString() + "��" + temp["infoSource"].asString() + ")\n";

				str += "����:" + temp["sourceUrl"].asString();
			}
		}

		return str;
	}

	//��ȡʡ������
	string getNewsProvince(string name)
	{
		string str;
		int id = 0;

		for (auto temp : newsRoot)
		{
			if (temp["provinceName"].asString().find(name) == string::npos && temp["summary"].asString().find(name) == string::npos)
				continue;

			if (id < temp["id"].asInt())
			{
				id = temp["id"].asInt();
				str = temp["title"].asString() + "(" + temp["pubDateStr"].asString() + ")\n";
				str += temp["summary"].asString() + "\n" + temp["provinceName"].asString() + "��" + temp["infoSource"].asString() + ")\n";

				str += "����:" + temp["sourceUrl"].asString();
			}

		}

		if (str.empty())
			str = "δ�ҵ��й���������";

		return str;
	}

	//��ȡĳ��������
	string getArea(string name)
	{
		stringstream buf;

		for (auto province : root)
		{
			string provinceName = province["provinceName"].asString();

			if (provinceName.find(name) != string::npos)
			{
				int confirmedCount = province["confirmedCount"].asInt();
				int deadCount = province["deadCount"].asInt();
				int curedCount = province["curedCount"].asInt();
				buf << provinceName;
				buf << " ȷ��" << confirmedCount;
				buf << " ����" << deadCount;
				buf << " ����" << curedCount;
				buf << endl << endl;

				for (auto city : province["cities"])
				{
					buf << city["cityName"].asString();
					buf << " ȷ��" << city["confirmedCount"].asInt();
					buf << " ����" << city["deadCount"].asInt();
					buf << " ����" << city["curedCount"].asInt();
					buf << endl;

				}

				if (!province["comment"].asString().empty())
				{
					buf << endl;
					buf << province["comment"].asString() << endl;
				}

				return buf.str();
			}
		}


		for (auto province : root)
		{
			for (auto city : province["cities"])
			{
				if (city["cityName"].asString() == name)
				{
					buf << province["provinceName"].asString() << "-" << city["cityName"].asString();
					buf << " ȷ��" << city["confirmedCount"].asInt();
					buf << " ����" << city["deadCount"].asInt();
					buf << " ����" << city["curedCount"].asInt();

					return buf.str();
				}
			}
		}

		buf << "δ�ҵ�������ݣ����ܴ˵��������飬��ȷ��ʡ��/���������Ƿ���ȷ";
		return buf.str();
	}

	//��ȡȫ��ʡ������
	string getProvince()
	{
		vector<AREA> all;

		for (auto province : root)
		{
			string provinceName = province["provinceName"].asString();
			int confirmedCount = province["confirmedCount"].asInt();
			int deadCount = province["deadCount"].asInt();
			int curedCount = province["curedCount"].asInt();

			all.push_back(AREA(provinceName, confirmedCount, deadCount, curedCount));
		}

		sort(all.begin(), all.end(), AREAComp);

		stringstream buf;

		buf << getMap() << endl;

		//buf << "ȫ������:" << endl;
		buf << "ȷ�ﲡ��: " << mainRoot["confirmedCount"].asInt() << endl;
		buf << "���Ʋ���: " << mainRoot["suspectedCount"].asInt() << endl;
		buf << "��������: " << mainRoot["deadCount"].asInt() << endl;
		buf << "��������: " << mainRoot["curedCount"].asInt() << endl << endl;


		for (auto temp : all)
		{
			buf << temp.name;
			buf << " ȷ��" << temp.confirmedCount;
			buf << " ����" << temp.deadCount;
			buf << " ����" << temp.curedCount;

			buf << endl;
		}

		return buf.str();
	}

	//��ȡ��������
	string getAbroad()
	{
		vector<AREA> all;

		for (auto province : abroadRoot)
		{
			string provinceName = province["provinceName"].asString();
			int confirmedCount = province["confirmedCount"].asInt();
			int deadCount = province["deadCount"].asInt();
			int curedCount = province["curedCount"].asInt();

			all.push_back(AREA(provinceName, confirmedCount, deadCount, curedCount));
		}

		sort(all.begin(), all.end(), AREAComp);


		stringstream buf;

		for (auto temp : all)
		{
			buf << temp.name;
			buf << " ȷ��" << temp.confirmedCount;
			buf << " ����" << temp.deadCount;
			buf << " ����" << temp.curedCount;

			buf << endl;
		}

		return buf.str();
	}

	////��ȡ������Ҫ����
	//string getCountRemark()
	//{
	//	string json = substring(html, "<script id=\"getStatisticsService\">try { window.getStatisticsService = ", "}catch(e){}");

	//	Json::Value root;
	//	Json::Reader reader;
	//	reader.parse(json, root);

	//	return root["countRemark"].asString();
	//}


private:
	string html;
	Json::Value root;
	Json::Value newsRoot;
	Json::Value abroadRoot;
	Json::Value mainRoot;

	vector<string> deleteFile;
};



#define VERSION 3

//������
string getUpdate()
{
	httplib::Client cli("www.xiaoxiaoge.cn", 80);

	auto res = cli.Get("/2019-nCoV_Update.json");

	if (!res || res->status != 200)
	{
		return "��ȡ��������ʧ�� �����쳣";
	}
	else
	{
		string json = res->body;

		Json::Value root;
		Json::Reader reader;
		reader.parse(json, root);

		int version = root["version"].asInt();
		if (version > VERSION)
		{
			string host = root["host"].asString();
			string path = root["path"].asString();

			
			httplib::Client client(host, 80);

			auto ret = client.Get(path.c_str());
			if (!ret || ret->status != 200)
			{
				throw exception("�������ʧ�� �����쳣");
			}

			ofstream file("app\\cn.xiaoxiaoge.2019-nCoV.cpk", ios::out | ios::binary);
			file << ret->body;
			file.close();

			return "�������,�����������°汾";
		}

		return "���ް汾����";
	}

	return "";


}


//ͳһ��Ϣ������
void MsgFun(string msg, std::function<void(string)> send)
{
	try
	{
		if (msg == "�˵�" || msg == "ָ��" || msg == "����" || msg == "����")
		{
			stringstream buf;

			buf << "ָ������:" << endl;
			buf << "�����ѯ/ȫ������" << endl;
			buf << "��������" << endl;
			buf << "�����ͼ" << endl;
			buf << "��������ͼ" << endl;
			buf << "��������" << endl;
			buf << "ʡ��(��ɽ��)+����" << endl;
			buf << "ʡ��(��ɽ��)+����" << endl;
			buf << "����(��Ϋ��)+����" << endl;
			buf << "������" << endl;

			buf << endl << "(�������� ����԰������ҽ��)";
			send(buf.str());
		}
		else if (msg == "�����ѯ" || msg == "ȫ������")
		{
			Plague a;
			string Data = a.getProvince();

			send(Data);
		}
		else if (msg == "��������")
		{
			Plague a;
			string Data = a.getAbroad();

			send(Data);
		}
		else if (msg == "�����ͼ")
		{
			Plague a;
			string Data = a.getMap();

			send(Data);
		}
		else if (msg == "��������ͼ")
		{
			Plague a;
			string Data = a.getTrendMap();

			send(Data);
		}
		else if (msg == "��������")
		{
			Plague a;
			string Data = a.getNews();

			send(Data);

		}
		else if (msg == "������")
		{
			string Data = getUpdate();

			send(Data);
		}
		else if (msg.length() != strlen("����") && msg.find("����") != msg.npos && msg.find("����") + strlen("����") == msg.size())
		{
			Plague a;
			string provinceName(msg.substr(0, msg.find("����")));
			string Data = a.getNewsProvince(provinceName);

			send(Data);

		}
		else if (msg.length() != strlen("����") && msg.find("����") != msg.npos && msg.find("����") + strlen("����") == msg.size())
		{
			Plague a;
			string cityName(msg.substr(0, msg.find("����")));
			string Data = a.getArea(cityName);

			send(Data);
		}

	}
	catch (exception & e)
	{
		stringstream buf;
		buf << "�쳣 " << e.what();

		send(buf.str());
	}
	catch (...)
	{
		stringstream buf;
		cout << "�����쳣";

		send(buf.str());
	}
}
