#include "httplib.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <codecvt>

#include <json/json.h>

using namespace std;

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

		string downPath("Plague" + path.substr(path.rfind("/") + 1));


		ofstream file("data\\image\\" + downPath, ios::out | ios::binary);
		file << ret->body;
		file.close();

		return downPath;
	}

	//��ȡ����
	bool getData(string& error)
	{
		httplib::Client cli("3g.dxy.cn", 80);

		auto res = cli.Get("/newh5/view/pneumonia");

		if (!res || res->status != 200)
		{
			error = "��ȡ����ʧ�� �����쳣";
		}
		else
		{

			html = UTF8ToANSI(res->body);
			//string json = substring(buf, "window.getTimelineService =", "}catch(e){}");
			string json = substring(html, "<script id=\"getAreaStat\">try { window.getAreaStat = ", "}catch(e){}");

			Json::Reader reader;
			reader.parse(json, root);

			Json::Reader newReader;
			string newJson = substring(html, "<script id=\"getTimelineService\">try { window.getTimelineService = ", "}catch(e){}");
			newReader.parse(newJson, newsRoot);

			Json::Reader abroadReader;
			string abroadJson = substring(html, "<script id=\"getListByCountryTypeService2\">try { window.getListByCountryTypeService2 = ", "}catch(e){}");
			abroadReader.parse(abroadJson, abroadRoot);

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

	//��ȡ�����ͼ
	string getMap()
	{
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
	}

	//��ȡ��������ͼ
	string getTrendMap()
	{
		string json = substring(html, "<script id=\"getStatisticsService\">try { window.getStatisticsService = ", "}catch(e){}");
		stringstream buf;

		Json::Value root;
		Json::Reader reader;
		reader.parse(json, root);

		string url = root["dailyPic"].asString();

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

	//��ȡ��������
	string getCity(string name)
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
					//AREA cityData(city["cityName"].asString(), city["confirmedCount"].asInt(), city["deadCount"].asInt(), city["curedCount"].asInt());
					buf << city["cityName"].asString();
					buf << " ȷ��" << city["confirmedCount"].asInt();
					buf << " ����" << city["deadCount"].asInt();
					buf << " ����" << city["curedCount"].asInt();
					buf << endl;

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
					AREA cityData(name, city["confirmedCount"].asInt(), city["deadCount"].asInt(), city["curedCount"].asInt());
					buf << cityData.name;
					buf << " ȷ��" << cityData.confirmedCount;
					buf << " ����" << cityData.deadCount;
					buf << " ����" << cityData.curedCount;

					return buf.str();
				}
			}
		}

		buf << "δ�ҵ�������ݣ����ܴ˵��������飬��ȷ��ʡ��/���������Ƿ���ȷ";
		return buf.str();
	}

	//��ȡȫ��ʡ������
	vector<AREA> getProvince()
	{
		vector<AREA> temp;

		for (auto province : root)
		{
			string provinceName = province["provinceName"].asString();
			int confirmedCount = province["confirmedCount"].asInt();
			int deadCount = province["deadCount"].asInt();
			int curedCount = province["curedCount"].asInt();

			temp.push_back(AREA(provinceName, confirmedCount, deadCount, curedCount));
		}

		sort(temp.begin(), temp.end(), AREAComp);

		return temp;
	}

	//��ȡȫ��ʡ������
	vector<AREA> getAbroad()
	{
		vector<AREA> temp;

		for (auto province : abroadRoot)
		{
			string provinceName = province["provinceName"].asString();
			int confirmedCount = province["confirmedCount"].asInt();
			int deadCount = province["deadCount"].asInt();
			int curedCount = province["curedCount"].asInt();

			temp.push_back(AREA(provinceName, confirmedCount, deadCount, curedCount));
		}

		sort(temp.begin(), temp.end(), AREAComp);

		return temp;
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
};


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

			buf << endl << "(�������� ����԰������ҽ��)";
			send(buf.str());
		}
		else if (msg == "�����ѯ" || msg == "ȫ������")
		{
			Plague a;
			stringstream buf;
			vector<AREA> all = a.getProvince();
			//buf << a.getCountRemark() << endl << endl;

			for (auto temp : all)
			{
				buf << temp.name;
				buf << " ȷ��" << temp.confirmedCount;
				buf << " ����" << temp.deadCount;
				buf << " ����" << temp.curedCount;

				buf << endl;
			}

			send(buf.str());
		}
		else if (msg == "��������")
		{
			Plague a;
			stringstream buf;
			vector<AREA> all = a.getAbroad();
			//buf << a.getCountRemark() << endl << endl;

			for (auto temp : all)
			{
				buf << temp.name;
				buf << " ȷ��" << temp.confirmedCount;
				buf << " ����" << temp.deadCount;
				buf << " ����" << temp.curedCount;

				buf << endl;
			}

			send(buf.str());
		}
		else if (msg == "�����ͼ")
		{
			Plague a;

			send(a.getMap());
		}
		else if (msg == "��������ͼ")
		{
			Plague a;
	
			send(a.getTrendMap());
		}
		else if (msg == "��������")
		{
			Plague a;
			stringstream buf;
			buf << a.getNews();

			send(buf.str());

		}
		else if (msg.length() != strlen("����") && msg.find("����") != msg.npos && msg.find("����") + strlen("����") == msg.size())
		{
			Plague a;
			stringstream buf;
			string provinceName(msg.substr(0, msg.find("����")));

			buf << a.getNewsProvince(provinceName);

			send(buf.str());

		}
		else if (msg.length() != strlen("����") && msg.find("����") != msg.npos && msg.find("����") + strlen("����") == msg.size())
		{
			Plague a;
			string cityName(msg.substr(0, msg.find("����")));
			auto Data = a.getCity(cityName);

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