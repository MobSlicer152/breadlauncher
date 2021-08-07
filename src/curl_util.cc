#include "curl_util.h"

static size_t write_resp_to_mem(void *contents, size_t size, size_t nmemb, void *user)
{
	std::vector<char> *out;
	size_t i;

	/* Resize the vector and copy the data in */
	out = (std::vector<char> *)user;
	for (i = 0; i < size * nmemb; i++) {
		out->push_back(((char *)contents)[i]);
	}

	return size * nmemb;
}

std::vector<char> get_file_in_memory(const std::string &url)
{
	CURL *curl;
	CURLcode res;
	std::vector<char> buf;

	/* Initialize the handle */
	curl = curl_easy_init();
	if (!curl) {
		BREAD_LOG("Failed to initialize curl operation\n");
		return buf;
	}

	/* Set up options */
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_resp_to_mem);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

	/* Do the request */
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	if (res != CURLE_OK) {
		BREAD_LOG("Failed to get file {}: {}\n", url, curl_easy_strerror(res));
		buf.clear();
		return buf;
	}

	/* Return the buffer */
	return buf;
}

fs::file_time_type get_remote_mtime(const std::string &url)
{
	CURL *curl;
	CURLcode res;
	std::vector<char> header_raw;
	std::string header;
	std::string mtime_str;
	chrono::time_point<chrono::system_clock> mtime;
	fs::file_time_type time;

	/* Initialize the handle */
	curl = curl_easy_init();
	if (!curl) {
		BREAD_LOG("Failed to initialize curl operation\n");
		return fs::file_time_type(-1s);
	}

	/* Set up the request */
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_NOBODY, 1); /* We only want the header */
	curl_easy_setopt(curl, CURLOPT_HEADER, 1);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_resp_to_mem);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &header_raw);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

	/* Do the request */
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	if (res != CURLE_OK) {
		BREAD_LOG("Failed to get header: {}\n", curl_easy_strerror(res));
		return fs::file_time_type(-1s);
	}

	/* Convert the header to a string */
	header = std::string(header_raw.data(), header_raw.size());

	/* Now that we have the header, we can parse it */
	mtime_str = header.substr(header.find("Last-Modified") + 15, header.size() - 1);
	mtime_str = mtime_str.substr(0, mtime_str.find_first_of("\r\n"));
	mtime = parse_net_time(mtime_str);
	time = CONVERT_TIME(mtime, chrono::system_clock, chrono::file_clock);
	return time;
}

std::string get_file(const std::string &url, const std::string &target, bool keep_contents, bool verbose)
{
	std::string raw;
	fs::file_time_type mtime = get_remote_mtime(url);

	/* Check on the file */
	if (!fs::exists(target) || mtime > fs::last_write_time(target)) {
		if (fs::exists(target)) {
			BREAD_LOG2(verbose, "File {} has modification time {:} behind server", target,
				   chrono::time_point<chrono::system_clock>(mtime - fs::last_write_time(target)));
		} else {
			BREAD_LOG2(verbose, "File {} is not present", target);
		}
		BREAD_LOG2(verbose, ", downloading a new copy from {}\n", url);

		/* Download the file into memory */
		std::vector<char> vec = get_file_in_memory(url);
		if (!vec.size()) {
			raw.clear();
			return raw;
		}

		/* Write out the file */
		std::ofstream out;
		out.open(target, std::ios::binary);
		if (out.fail()) {
			BREAD_LOG("Failed to open the file {}\n", target);
			return raw;
		}
		out.write(vec.data(), vec.size());
		if (out.fail()) {
			BREAD_LOG("Failed to write {} bytes to {}\n", vec.size(), target);
			return raw;
		}
		out.close();

		/* Get a string */
		if (keep_contents)
			raw = std::string(vec.data(), vec.size());
		
		/* Clear the vector */
		vec.clear();

		/* Say we have the file */
		BREAD_LOG2(verbose, "Finished downloading {}\n", target);
	} else {
		if (!keep_contents)
			return raw;

		/* Open up the existing file */
		BREAD_LOG2(verbose, "Using existing file {}\n", target);
		std::ifstream in;
		size_t len;
		in.open(target, std::ios::binary);
		in >> std::noskipws;
		if (in.fail()) {
			BREAD_LOG("Failed to open {}\n", target);
			raw.clear();
			in.close();
			return raw;
		}

		/* Get the file's size */
		in.seekg(0, std::ios::end);
		len = in.tellg();
		in.seekg(0, std::ios::beg);
		raw.reserve(len);

		/* Read the file */
		raw.insert(raw.begin(), std::istream_iterator<char>(in), std::istream_iterator<char>());
		if (raw.size() < len) {
			BREAD_LOG("Failed to read {} bytes from {}\n", len, target);
			raw.clear();
			in.close();
			return raw;
		}
		in.close();
	}

	/* Return the file's contents */
	return raw;
}
