#ifndef _VKGLSLPROGRAM_HPP
#define _VKGLSLPROGRAM_HPP
/*-------------------------------------------------------------------------
 * Vulkan CTS Framework
 * --------------------
 *
 * Copyright (c) 2017 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *//*!
 * \file
 * \brief GLSL source program.
 *//*--------------------------------------------------------------------*/

#include "vkDefs.hpp"
#include "gluShaderProgram.hpp"

#include <string>

namespace tcu
{
class TestLog;
} // tcu

namespace vk
{

struct GlslBuildOptions
{
	SpirvVersion	targetVersion;
	deUint32		flags;

	GlslBuildOptions (SpirvVersion targetVersion_, deUint32 flags_)
		: targetVersion	(targetVersion_)
		, flags			(flags_)
	{}

	GlslBuildOptions (void)
		: targetVersion	(SPIRV_VERSION_1_0)
		, flags			(0u)
	{}
};

struct GlslSource
{
	std::vector<std::string>	sources[glu::SHADERTYPE_LAST];
	GlslBuildOptions			buildOptions;

								GlslSource (void) {}

	GlslSource&					operator<<			(const glu::ShaderSource& shaderSource)	{ sources[shaderSource.shaderType].push_back(shaderSource.source);	return *this;	}
	GlslSource&					operator<<			(const GlslBuildOptions& buildOptions_)	{ buildOptions = buildOptions_;										return *this;	}
};

tcu::TestLog&	operator<<		(tcu::TestLog& log, const GlslSource& glslSource);

} // vk

#endif // _VKGLSLPROGRAM_HPP
