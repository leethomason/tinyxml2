/*
Original code by Lee Thomason (www.grinninglizard.com)

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

module;

#include <tinyxml2.h>

export module tinyxml2;

export using ::TIXML2_MAJOR_VERSION;
export using ::TIXML2_MINOR_VERSION;
export using ::TIXML2_PATCH_VERSION;
export using ::TINYXML2_MAX_ELEMENT_DEPTH;

#pragma push_macro("TINYXML2_MAJOR_VERSION")
#undef TINYXML2_MAJOR_VERSION
export inline constexpr auto TINYXML2_MAJOR_VERSION =
#pragma pop_macro("TINYXML2_MAJOR_VERSION")
    TINYXML2_MAJOR_VERSION;

#pragma push_macro("TINYXML2_MINOR_VERSION")
#undef TINYXML2_MINOR_VERSION
export inline constexpr auto TINYXML2_MINOR_VERSION =
#pragma pop_macro("TINYXML2_MINOR_VERSION")
    TINYXML2_MINOR_VERSION;

#pragma push_macro("TINYXML2_PATCH_VERSION")
#undef TINYXML2_PATCH_VERSION
export inline constexpr auto TINYXML2_PATCH_VERSION =
#pragma pop_macro("TINYXML2_PATCH_VERSION")
    TINYXML2_PATCH_VERSION;

export namespace tinyxml2 {
using tinyxml2::Whitespace;
using tinyxml2::XMLAttribute;
using tinyxml2::XMLComment;
using tinyxml2::XMLConstHandle;
using tinyxml2::XMLDeclaration;
using tinyxml2::XMLDocument;
using tinyxml2::XMLElement;
using tinyxml2::XMLError;
using tinyxml2::XMLHandle;
using tinyxml2::XMLNode;
using tinyxml2::XMLPrinter;
using tinyxml2::XMLText;
using tinyxml2::XMLUnknown;
using tinyxml2::XMLUtil;
using tinyxml2::XMLVisitor;
using tinyxml2::DynArray;
} // namespace tinyxml2
