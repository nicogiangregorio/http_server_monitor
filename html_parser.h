#ifndef HTML_PARSER_H
#define HTML_PARSER_H
#include <iostream>
#include <vector>

using namespace std;

const string SERVER_UPTIME_STRING = "Server uptime: ";
const string RESTART_TIME_STRING = "Restart Time: ";
const string CPU_USAGE_STRING = "CPU Usage: ";
const string ACTIVE_THREADS = "requests currently being processed, ";

struct server_status {
	int total_threads;
	int idle_threads;
	int status;
	float cpu_usage;
	string last_start;
	string uptime;
	string server_name;
	string path;
};

template <class T>
bool from_string(T& t, const std::string& s, std::ios_base& (*f)(std::ios_base&))
{
  std::istringstream iss(s);
  return !(iss >> f >> t).fail();
}


class html_parser
{

public: 
	html_parser(stringbuf& strbuf)
	{
		parse_html_dt(strbuf);
		//get_server_infos(v_dt);
	}

        void get_server_infos(server_status *status)
	{
		int work_threads;
		float cpu_usage_f;

		for(vector<string>::iterator itr = v_dt.begin(); itr < v_dt.end(); ++itr)
		{
			size_t pos;

			if((pos = ((string)*itr).find(SERVER_UPTIME_STRING)) != string::npos)
			{
                                status->uptime = ((string)*itr).substr(SERVER_UPTIME_STRING.size(), ((string)*itr).size() - pos);
			}
			else if ((pos = ((string)*itr).find(RESTART_TIME_STRING)) != string::npos)
			{
                                status->last_start = ((string)*itr).substr(RESTART_TIME_STRING.size(), ((string)*itr).size() - pos);
			}
			else if ((pos = ((string)*itr).find(CPU_USAGE_STRING)) != string::npos)
			{
				size_t start_pos = ((string)*itr).find("- ") + 2;
				size_t length_to_cut = ((string)*itr).find("%") - start_pos;
				
				string cpu_usage("0");
				cpu_usage.append(((string)*itr).substr(start_pos, length_to_cut));
				
				from_string<float>(cpu_usage_f, cpu_usage, std::dec);
                                status->cpu_usage = cpu_usage_f;
			}
			else if ((pos = ((string)*itr).find(ACTIVE_THREADS)) != string::npos)
			{
				size_t length_to_cut = ((string)*itr).find("idle") - pos - ACTIVE_THREADS.size();
				
				from_string<int>(work_threads, ((string)*itr).substr(pos + ACTIVE_THREADS.size(), length_to_cut), std::dec);
                                status->idle_threads = work_threads;
			}
		}
	}

private:
	void parse_html_dt(stringbuf& strbuf)
	{
		string extr_str(strbuf.str());

		size_t start_pos = 0;
		size_t end_pos = 0;

		/* I am interested in all the information until the first occurence of <pre> tag */
		size_t max_limit = extr_str.find("<pre>", 0);

		while (end_pos < max_limit)
		{
			start_pos = extr_str.find("<dt>", start_pos);
			end_pos = extr_str.find("</dt>", start_pos);
			
			if(start_pos!= string::npos)
			{
				v_dt.push_back(extr_str.substr(start_pos + 4, end_pos - start_pos - 4));
				start_pos = start_pos + 4;
			}
		}
	}

	
	vector<string> v_dt;
};


#endif
