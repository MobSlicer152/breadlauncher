#pragma once

#ifndef CURL_UTIL_H
#define CURL_UTIL_H 1

#include <fstream>
#include <ios>
#include <iterator>
#include <string>
#include <vector>

extern "C" {
#include <curl/curl.h>
}

#include "breadlauncher.h"

/**
 * \brief Translates network time into system time using curl_getdate
 * 
 * \param time is the time string to parse
 * \return Returns the time relative to the system clock
 */
inline chrono::time_point<chrono::system_clock>
parse_net_time(const std::string &time)
{
	return chrono::system_clock::from_time_t(
		curl_getdate(time.c_str(), nullptr));
}

/**
 * \brief Get the contents of a file in memory
 * 
 * \param url is the URL to download
 * \return Returns contents of the file or an empty buffer
 */
std::vector<char> get_file_in_memory(const std::string &url);

/**
 * \brief Get the modification time of a remote file
 * 
 * \param url is the URL to probe
 * \return Returns the time the file was modified or 1 second before the epoch
 */
fs::file_time_type get_remote_mtime(const std::string &url);

/**
 * \brief Download a file or open an existing copy of it if it's up to date
 * 
 * \param url is the URL to download
 * \param target is the location to download the file to
 * \return Returns the contents of the file
 */
std::string get_file(const std::string &url, const std::string &target,
		     bool keep_contents = true, bool verbose = true);

#endif /* !CURL_UTIL_H */
