#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <sstream>
#include <string.h>
#include <string>
#include <vector>

#include <curl/curl.h>

#include <fmt/chrono.h>
#include <fmt/format.h>

#undef GetObject
#include <rapidjson/document.h>

#include "console_check.h"
#include "curl_util.h"
#include "version_util.h"

/**
 * \brief Barf a usage message and exit
 */
void usage(const std::string &argv0);

/**
 * \brief Exit and pause for input
 */
void quit(int code);

extern "C" int main(int argc, char *argv[])
{
	std::string root_dir;
	std::string manifest_raw;
	rapidjson::Document manifest;
	std::string version_name;
	std::string version_raw;
	rapidjson::Document version;

	/* Deal with arguments */
	if (argc > 1) {
		/* See if we have any arguments */
		if (strcmp(argv[1], "-d") == 0) {
			/* Make sure we have another argument */
			if (argc < 3)
				usage(argv[0]);

			/* Set the root directory */
			root_dir = fs::absolute(argv[2]).generic_string();
		} else if (strcmp(argv[1], "-help") == 0 ||
			   strcmp(argv[1], "--help") == 0)
			usage(argv[0]);
	}

	/* Make sure the root directory is set */
	if (!root_dir.size()) {
		root_dir = fs::absolute(".").generic_string();
		if (!fs::is_empty(root_dir)) {
			BREAD_LOG(
				"Directory {} is not empty, using subfolder .breadlauncher\n\n",
				root_dir);
			root_dir += "/.breadlauncher";
		}
	}

	/* Create the directories */
	try {
		fs::create_directory(root_dir);
		fs::create_directory(root_dir + "/accounts");
		fs::create_directory(root_dir + "/assets");
		fs::create_directory(root_dir + "/libs");
		fs::create_directory(root_dir + "/libs/native");
		fs::create_directory(root_dir + "/versions");
	} catch (std::exception e) {
		BREAD_LOG(
			"Failed to create one or more directories for Minecraft: {}\n",
			e.what());
		quit(1);
	}

	/* Initialize libcurl */
	curl_global_init(CURL_GLOBAL_ALL);

	/* Get the version manifest */
	manifest_raw = get_file(
		"https://launchermeta.mojang.com/mc/game/version_manifest.json",
		root_dir + "/versions/version_manifest.json");

	/* Parse the manifest */
	manifest.Parse(manifest_raw.c_str());
	manifest_raw.clear();
	if (manifest.HasParseError()) {
		BREAD_LOG("Failed to parse version manifest\n");
	}

	/* Check for the list of versions */
	if (!manifest.HasMember("versions") ||
	    !manifest["versions"].IsArray()) {
		BREAD_LOG(
			"Version manifest does not have a valid list of versions\n");
		quit(1);
	}

	/* Get the version (plain cin works because versions don't have spaces) */
	std::cout << "\nEnter the version to run: ";
	std::cin >> version_name;
	if (tolower(version_name[0]) == 'q' ||
	    tolower(version_name[0]) == 'e') {
		BREAD_LOG("Quitting.\n");
		quit(0);
	} else if (version_name == "latest" ||
		   version_name == "latest-snapshot") {
		version_name = manifest["latest"]["snapshot"].GetString();
		BREAD_LOG(
			"Using latest version (may be a release or a snapshot), which is {}\n",
			version_name);
	} else if (version_name == "latest-release") {
		version_name = manifest["latest"]["release"].GetString();
		BREAD_LOG("Using latest release, which is {}\n", version_name);
	}

	/* Get the version */
	version = get_version_definition(version_name, root_dir, manifest);
	if (version.IsNull())
		quit(1);

	/* Get the assets for this version */
	BREAD_LOG()

	/* Clean up and return */
	curl_global_cleanup();
	quit(0);
}

void usage(const std::string &argv0)
{
	fmt::format(
		"{0} usage: {0} [options]\n"
		"\t-d <directory>\tsets the directory to store data in\n"
		"\t-help/--help\tshow this help\n"
		"\nNote that any additional arguments will be passed to the JVM\n",
		argv0);
	quit(1);
}

void quit(int code)
{
	/* Pause until a key is pressed on Windows */
	if (check_parent_is_console()) {
		std::cout << "\nPress any key to exit...\n";
		(void)getch_();
	}
	exit(code);
}
