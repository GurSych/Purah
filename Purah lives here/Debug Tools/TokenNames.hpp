// Copyright 2026 GurSych (Daniil Gurchin)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef PURAH_DEBUG_TOOLS_TOKEN_NAMES_HPP
#define PURAH_DEBUG_TOOLS_TOKEN_NAMES_HPP

#pragma once

#include <string>
#include <map>

#include "../Lexer/Tokens.hpp"

using namespace purah;

namespace purah::debug {

    inline std::map<tkn::TokenType, std::string> tokenNames = {
        { tkn::TokenType::EOF,              "EOF"              },
        { tkn::TokenType::EMPTY,            "EMPTY"            },
        { tkn::TokenType::IDENTIFIER,       "IDENTIFIER"       },
        { tkn::TokenType::INTEGER,          "INTEGER"          },
        { tkn::TokenType::STRING,           "STRING"           },
        { tkn::TokenType::FLOAT,            "FLOAT"            },
        { tkn::TokenType::BOOL,             "BOOL"             },
        { tkn::TokenType::SLASH,            "SLASH"            },
        { tkn::TokenType::DBL_SLASH,        "DBL_SLASH"        },
        { tkn::TokenType::BACKSLASH,        "BACKSLASH"        },
        { tkn::TokenType::DBL_BACKSLASH,    "DBL_BACKSLASH"    },
        { tkn::TokenType::PLUS,             "PLUS"             },
        { tkn::TokenType::MINUS,            "MINUS"            },
        { tkn::TokenType::EQUALS,           "EQUALS"           },
        { tkn::TokenType::L_BRACKET,        "L_BRACKET"        },
        { tkn::TokenType::R_BRACKET,        "R_BRACKET"        },
        { tkn::TokenType::INCREMENT,        "INCREMENT"        },
        { tkn::TokenType::DECREMENT,        "DECREMENT"        },
        { tkn::TokenType::STAR,             "STAR"             },
        { tkn::TokenType::PERCENT,          "PERCENT"          },
        { tkn::TokenType::IF,               "IF"               },
        { tkn::TokenType::ELSE,             "ELSE"             },
        { tkn::TokenType::FUNCTION,         "FUNCTION"         },
        { tkn::TokenType::RETURN,           "RETURN"           },
        { tkn::TokenType::L_CRL_BRACKET,    "L_CRL_BRACKET"    },
        { tkn::TokenType::R_CRL_BRACKET,    "R_CRL_BRACKET"    },
        { tkn::TokenType::ARROW,            "ARROW"            },
        { tkn::TokenType::L_SQR_BRACKET,    "L_SQR_BRACKET"    },
        { tkn::TokenType::R_SQR_BRACKET,    "R_SQR_BRACKET"    },
        { tkn::TokenType::NOT,              "NOT"              },
        { tkn::TokenType::AND,              "AND"              },
        { tkn::TokenType::OR,               "OR"               },
        { tkn::TokenType::TRUE,             "TRUE"             },
        { tkn::TokenType::FALSE,            "FALSE"            },
        { tkn::TokenType::EQUALITY,         "EQUALITY"         },
        { tkn::TokenType::NO_EQUALITY,      "NO_EQUALITY"      },
        { tkn::TokenType::MORE,             "MORE"             },
        { tkn::TokenType::MORE_OR_EQUALITY, "MORE_OR_EQUALITY" },
        { tkn::TokenType::LESS,             "LESS"             },
        { tkn::TokenType::LESS_OR_EQUALITY, "LESS_OR_EQUALITY" },
        { tkn::TokenType::DOT,              "DOT"              },
        { tkn::TokenType::COMMA,            "COMMA"            },
        { tkn::TokenType::COLON,            "COLON"            },
        { tkn::TokenType::SEMICOLON,        "SEMICOLON"        },
        { tkn::TokenType::VAR,              "VAR"              },
        { tkn::TokenType::CONST,            "CONST"            },
        { tkn::TokenType::WHILE,            "WHILE"            },
        { tkn::TokenType::FOR,              "FOR"              },
        { tkn::TokenType::BREAK,            "BREAK"            },
        { tkn::TokenType::CONTINUE,         "CONTINUE"         },
        { tkn::TokenType::COUT,             "COUT"             },
        { tkn::TokenType::CLASS,            "CLASS"            }
    };

    inline std::string getTokenName(tkn::TokenType type) {
        if (tokenNames.contains(type)) {
            return tokenNames[type];
        }
        return "UNNAMED";
    }

}

#endif // PURAH_DEBUG_TOOLS_TOKEN_NAMES_HPP
