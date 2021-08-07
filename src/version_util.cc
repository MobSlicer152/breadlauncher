#include "version_util.h"

rapidjson::Document get_version_definition(const std::string &version, const std::string &root_dir,
					   rapidjson::Document &version_manifest)
{
	std::string version_raw;
	rapidjson::Document version_json;

	/* Search for the version in the manifest to get its details */
	auto versions = version_manifest["versions"].GetArray();
	rapidjson::Value version_data;
	for (rapidjson::Value &ver : versions) {
		if (version == ver["id"].GetString()) {
			version_data = ver;
			break;
		}
	}
	if (version_data.IsNull()) {
		BREAD_LOG("The specified version ({}) is not present in the version manifest\n", version);
		return rapidjson::Document(); /* Return a NULL document */
	}

	/* Get the version's JSON */
	BREAD_LOG("Version {} is of type {} and is defined at"
		  " {}.\n",
		  version, version_data["type"].GetString(), version_data["url"].GetString());
	try {
		fs::create_directory(root_dir + "/versions/" + version);
	} catch (std::exception e) {
		BREAD_LOG("Failed to create directory {}/versions/{}: {}", root_dir, version, e.what());
		return rapidjson::Document();
	}

	/* Get the definition of this version */
	version_raw = get_file(version_data["url"].GetString(),
			       fmt::format("{}/versions/{}/version.json", root_dir, version), true);
	version_json.Parse(version_raw.c_str());
	version_raw.clear();
	if (version_json.HasParseError()) {
		BREAD_LOG("Failed to parse version definition for {}\n", version);
		return rapidjson::Document();
	}

	/* Return the document */
	return version_json;
}
