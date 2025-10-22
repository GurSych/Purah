#pragma once

#include "ParserNodes.hpp"

namespace purah { namespace opr {

    using operator_handler = nds::ASTPtr(*)(nds::ASTPtr, nds::ASTPtr);

} }