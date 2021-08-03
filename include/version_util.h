#pragma once

#ifndef VERSION_UTIL_H
#define VERSION_UTIL_H

#include <iostream>
#include <ios>
#include <string>

#include <rapidjson/document.h>

#include "curl_util.h"

/**
 * \brief Gets the definition for a version
 * 
 * \param version is the name of the version (corresponds to its id key in the version manifest)
 * \param root_dir is the root directory to store the definition in
 * \param version_manifest is the parsed version manifest to search
 * \return Returns the parsed version definition
 */
rapidjson::Document get_version_definition(const std::string &version,
					   const std::string &root_dir,
					   rapidjson::Document &version_manifest);

#endif /* !VERSION_UTIL_H */
