/**
 * \file breadlauncher.h
 * \author MobSlicer152 (brambleclaw1414@gmail.com)
 * \brief General definitions
 * \version 0.1
 * \date 02-08-2021
 * 
 * \copyright Copyright 2021 MobSlicer152
 */

#pragma once

#ifndef BREADLAUNCHER_H
#define BREADLAUNCHER_H 1

/* Given that the names inside these namespaces are also long this gets to be a bit much */
#include <chrono>
#include <filesystem>
namespace chrono = std::chrono;
using namespace std::literals;
namespace fs = std::filesystem;

/* Debug console log */
#ifndef BREAD_CONFIG_DIST
#include <fmt/color.h>
#include <fmt/chrono.h>
#include <fmt/format.h>
#define BREAD_LOG(...) fmt::print(__VA_ARGS__)
#define BREAD_LOG2(verbose, ...)                 \
	{                                        \
		if (verbose)                     \
			fmt::print(__VA_ARGS__); \
	}
#else
#define BREAD_LOG(...)
#define BREAD_LOG2(...)
#endif

/* OS name for checking the libraries in the version definition */
#if BREAD_WIN32
#define BREAD_OS_NAME "windows"
#elif BREAD_MACOS
#define BREAD_OS_NAME "osx"
#elif BREAD_LINUX
#define BREAD_OS_NAME "linux"
#endif

#endif /* !BREADLAUNCHER_H */
