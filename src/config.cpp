#include <cjson/cJSON.h>
#include <string>
#include <cstdio>
#include <stdexcept>
#include <iostream>

#include <cstring>
#include <map>
#include <set>
#include <memory>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "config.h"
#include "tool.h"
#include "exception.h"
#include "option.h"

namespace YQ {


using std::to_string;
using std::exception;
using std::set;
using std::map;
using std::unique_ptr;
using std::make_pair;
using std::ostream;

struct IOException: public Exception {
	using Exception::Exception;
};
struct ConfigException : public Exception {
	using Exception::Exception;
#define throw_if_NULL(call) if((call)==NULL) throw ConfigException()
};


static string
read_whole_file (const char *file) noexcept (false)
{
	FILE *pfile = fopen (file, "r");
	if (pfile == NULL)
		throw IOException ();
		
	if (fseek (pfile, 0, SEEK_END) == -1) {
		if (fclose (pfile) == EOF)
			throw IOException ();
		throw IOException ();
	}
	
	const auto file_size = ftell (pfile);
	
	if (file_size == -1) {
		if (fclose (pfile) == EOF)
			throw IOException ();
		throw IOException ();
	}
	
	rewind (pfile);
	
	char buf[file_size];
	
	size_t nread = fread (buf, sizeof (char), sizeof (buf), pfile);
	if (nread != file_size) {
		if (fclose (pfile) == EOF)
			throw IOException ();
		throw IOException ();
	}
	if (fclose (pfile) == EOF)
		throw IOException ();
	return string (buf);
}

void load_exe_array (set<string> &dest, cJSON *J_array)
{
	const int size = cJSON_GetArraySize (J_array);
	for (int i = 0; i < size; ++i) {
		cJSON *J_string = cJSON_GetArrayItem (J_array, i);
		throw_if_NULL (J_string);
		
		const char *str = cJSON_GetStringValue (J_string);
		throw_if_NULL (str);
		
		dest.insert (get_real_path (str));
	}
}

void load_perm_list (perm_list_t &perm_list, cJSON *J_perm_list, string key, string item)
{
	const int size = cJSON_GetArraySize (J_perm_list);
	for (int i = 0; i < size ; ++i) {
		cJSON *J_piece = cJSON_GetArrayItem (J_perm_list, i);
		throw_if_NULL (J_piece);
		
		cJSON *J_key = cJSON_GetObjectItem (J_piece, key.c_str());
		throw_if_NULL (J_key);
		
		string key;
		{
			const char *str = cJSON_GetStringValue (J_key);
			throw_if_NULL (str);
			
			key = str;
		}
		
		cJSON *J_exe_list = cJSON_GetObjectItem (J_piece, item.c_str());
		throw_if_NULL (J_exe_list);
		
		set<string> exes;
		load_exe_array (exes, J_exe_list);
		perm_list.insert (make_pair (key, exes));
	}
}

enum class PERM_LIST {
	FILE_BLACK_LIST,
	FILE_WHITE_LIST,
	IP_BLACK_LIST,
	IP_WHITE_LIST
};

static
const perm_list_t &perm_list (PERM_LIST id)
{
	static map<PERM_LIST, perm_list_t> perm_list_table;
	if (perm_list_table.size() == 0) {
	
		perm_list_t file_black_list;
		perm_list_t file_white_list;
		perm_list_t ip_black_list;
		perm_list_t ip_white_list;
		
		
		const string config = read_whole_file (CONFIG_FILE_PATH);
		
		cJSON *J_config = cJSON_Parse (config.c_str());
		throw_if_NULL (J_config);
		
		cJSON *J_file_black_list = cJSON_GetObjectItem (J_config, "file_black_list");
		cJSON *J_file_white_list = cJSON_GetObjectItem (J_config, "file_white_list");
		cJSON *J_ip_black_list = cJSON_GetObjectItem (J_config, "ip_black_list");
		cJSON *J_ip_white_list = cJSON_GetObjectItem (J_config, "ip_white_list");
		
		
		throw_if_NULL (J_file_black_list);
		throw_if_NULL (J_file_white_list);
		throw_if_NULL (J_ip_black_list);
		throw_if_NULL (J_ip_white_list);
		
		load_perm_list (file_black_list, J_file_black_list, "file", "exe");
		load_perm_list (file_white_list, J_file_white_list, "file", "exe");
		load_perm_list (ip_black_list, J_ip_black_list, "ip", "exe");
		load_perm_list (ip_white_list, J_ip_white_list, "ip", "exe");
		
		perm_list_table.insert (make_pair (PERM_LIST::FILE_BLACK_LIST, file_black_list));
		perm_list_table.insert (make_pair (PERM_LIST::FILE_WHITE_LIST, file_white_list));
		perm_list_table.insert (make_pair (PERM_LIST::IP_BLACK_LIST, ip_black_list));
		perm_list_table.insert (make_pair (PERM_LIST::IP_WHITE_LIST, ip_white_list));
	}
	return perm_list_table.at (id);
}

const perm_list_t &file_black_list()
{
	return perm_list (PERM_LIST::FILE_BLACK_LIST);
}

const perm_list_t &file_white_list()
{
	return perm_list (PERM_LIST::FILE_WHITE_LIST);
}

const perm_list_t &ip_black_list()
{
	return perm_list (PERM_LIST::IP_BLACK_LIST);
}

const perm_list_t &ip_white_list()
{
	return perm_list (PERM_LIST::IP_WHITE_LIST);
}

void print_perm_list (FILE *os, const perm_list_t &perm_list)
{
	for (auto &it : perm_list) {
		fprintf (os, "%s : [", it.first.c_str());
		
		for (auto p_exe = it.second.begin(); p_exe != it.second.end();) {
			fprintf (os, "\n    %s", p_exe->c_str());
			if (++p_exe == it.second.end())
				fprintf (os, "\n");
			else
				fprintf (os, ",");
		}
		fprintf (os, "]");
	}
}


[[deprecated]]//未知bug：系统调用中使用C++ ostream会导致 段错误。
void __print_perm_list (ostream &os, const perm_list_t &perm_list)
{
	for (auto &it : perm_list) {
		os << it.first << " : [";
		
		for (auto p_exe = it.second.begin(); p_exe != it.second.end();) {
			os << "\n    " << *p_exe << "";
			if (++p_exe == it.second.end())
				os << "\n";
			else
				os << ",";
		}
		
		os << "]";
	}
}

}