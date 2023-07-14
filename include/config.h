#pragma once

#include <map>
#include <ostream>
#include <string>
#include <set>
namespace YQ {

using std::map;
using std::ostream;
using std::set;
using std::string;

using perm_list_t = map<string, set<string>>;

const perm_list_t &file_black_list() noexcept (false);
const perm_list_t &file_white_list() noexcept (false);
const perm_list_t &ip_black_list() noexcept (false);
const perm_list_t &ip_white_list() noexcept (false);

void print_perm_list (FILE *os, const perm_list_t &perm_list);

class IOException;
class ConfigException;

}
