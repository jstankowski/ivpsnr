/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "../src/xCommonDefPMBB-BASE.h"
#include "../src/xCfgINI.h"
#include "../src/xString.h"

using namespace PMBB_BASE;


//===============================================================================================================================================================================================================
// xString
//===============================================================================================================================================================================================================
TEST_CASE("xString")
{ 
  SUBCASE("replace")
  {
    const std::string Token     = "[token]";
    const std::string Replace   = "PMBB";
    const std::string StringSrc = "[token] Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nam porta [token] blandit tempor. Cras mi massa, sodales rutrum nulla vitae, consectetur aliquet sapien. \n Duis sit amet orci sed libero [token] lobortis pharetra ut id ante. Sed luctus dolor non [token] nibh luctus tempus. Maecenas id lorem nisi. Etiam a sodales lectus. Morbi sit amet sodales ligula, id iaculis sapien. ";
    
    const std::string StringRef_replaceFirst = "PMBB Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nam porta [token] blandit tempor. Cras mi massa, sodales rutrum nulla vitae, consectetur aliquet sapien. \n Duis sit amet orci sed libero [token] lobortis pharetra ut id ante. Sed luctus dolor non [token] nibh luctus tempus. Maecenas id lorem nisi. Etiam a sodales lectus. Morbi sit amet sodales ligula, id iaculis sapien. ";
    const std::string StringRef_replaceLast  = "[token] Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nam porta [token] blandit tempor. Cras mi massa, sodales rutrum nulla vitae, consectetur aliquet sapien. \n Duis sit amet orci sed libero [token] lobortis pharetra ut id ante. Sed luctus dolor non PMBB nibh luctus tempus. Maecenas id lorem nisi. Etiam a sodales lectus. Morbi sit amet sodales ligula, id iaculis sapien. ";
    const std::string StringRef_replaceAll   = "PMBB Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nam porta PMBB blandit tempor. Cras mi massa, sodales rutrum nulla vitae, consectetur aliquet sapien. \n Duis sit amet orci sed libero PMBB lobortis pharetra ut id ante. Sed luctus dolor non PMBB nibh luctus tempus. Maecenas id lorem nisi. Etiam a sodales lectus. Morbi sit amet sodales ligula, id iaculis sapien. ";

    CHECK(xString::replaceFirst(StringSrc, Token, Replace) == StringRef_replaceFirst);
    CHECK(xString::replaceLast (StringSrc, Token, Replace) == StringRef_replaceLast );
    CHECK(xString::replaceAll  (StringSrc, Token, Replace) == StringRef_replaceAll  );
  }

  SUBCASE("strip")
  {
    const std::string_view StringSrc        = "   Lorem ipsum     ";
    const std::string_view StringRef_stripL = "Lorem ipsum     "   ;
    const std::string_view StringRef_stripR = "   Lorem ipsum"     ;
    
    CHECK(xString::stripL(std::string_view(StringSrc)) == StringRef_stripL);
    CHECK(xString::stripR(std::string_view(StringSrc)) == StringRef_stripR);    
  }

  SUBCASE("trim")
  {
    const std::string StringSrc       = "   Lorem ipsum     ";
    const std::string StringRef_trimL = "Lorem ipsum     "   ;
    const std::string StringRef_trimR = "   Lorem ipsum"     ;

    std::string String_trimL = StringSrc;
    xString::trimL(String_trimL);
    CHECK(String_trimL == StringRef_trimL);
    std::string String_trimR = StringSrc;
    xString::trimR(String_trimR);
    CHECK(String_trimR == StringRef_trimR);
  }  
}

//===============================================================================================================================================================================================================
// xCfgINI
//===============================================================================================================================================================================================================
TEST_CASE("xCfgINI")
{
  SUBCASE("simple")
  {
    const std::string ConfigSimple = R"RAWSTRING(
Text      = "A.yuv"
Integer   = 2048
Float     = 3.14
Negative  = -1

)RAWSTRING";

    xCfgINI::xParser CfgParser;
    REQUIRE(CfgParser.loadFromString(ConfigSimple));

    REQUIRE(CfgParser.getRootSection().findParam("Text"    ));
    REQUIRE(CfgParser.getRootSection().findParam("Integer" ));
    REQUIRE(CfgParser.getRootSection().findParam("Float"   ));
    REQUIRE(CfgParser.getRootSection().findParam("Negative"));

    REQUIRE(CfgParser.findParam("Text"    ));
    REQUIRE(CfgParser.findParam("Integer" ));
    REQUIRE(CfgParser.findParam("Float"   ));
    REQUIRE(CfgParser.findParam("Negative"));

    CHECK(CfgParser.getParam("Text"    ).getArg(0, std::string("")) == "A.yuv");
    CHECK(CfgParser.getParam("Integer" ).getArg(0, std::string("")) == "2048" );
    CHECK(CfgParser.getParam("Float"   ).getArg(0, std::string("")) == "3.14" );
    CHECK(CfgParser.getParam("Negative").getArg(0, std::string("")) == "-1"   );

    CHECK(CfgParser.getParam("Text"    ).getArg(0, std::string("")) == "A.yuv");
    CHECK(CfgParser.getParam("Integer" ).getArg(0, 0              ) == 2048   );
    CHECK(CfgParser.getParam("Float"   ).getArg(0, 0.0f           ) == 3.14f  );
    CHECK(CfgParser.getParam("Negative").getArg(0, 0              ) == -1     );

    CHECK(CfgParser.getParam1stArg("Text"    , std::string("")) == "A.yuv");
    CHECK(CfgParser.getParam1stArg("Integer" , 0              ) == 2048   );
    CHECK(CfgParser.getParam1stArg("Float"   , 0.0f           ) == 3.14f  );
    CHECK(CfgParser.getParam1stArg("Negative", 0              ) == -1     );
  }

  SUBCASE("sections")
  {
    const std::string ConfigSections = R"RAWSTRING(
Text = "A.yuv"
[sectiomA]
Integer   = 2048
Float     = 3.14
[sectiomB]
Negative  = -1

)RAWSTRING";

    xCfgINI::xParser CfgParser;
    REQUIRE(CfgParser.loadFromString(ConfigSections));
    
    //root section    
    REQUIRE(CfgParser.findParam("Text"));
    REQUIRE(CfgParser.getRootSection().findParam("Text"));
    CHECK(CfgParser.getRootSection().getParam("Text").getArg(0, std::string("")) == "A.yuv");
    CHECK(CfgParser.getRootSection().getParam("Text").getArg(0, std::string("")) == "A.yuv");
    CHECK(CfgParser.getRootSection().getParam1stArg("Text", std::string("")) == "A.yuv");

    //sectionA
    REQUIRE(CfgParser.findSection("sectiomA"));

    REQUIRE(CfgParser.getSection("sectiomA").findParam("Integer"));
    REQUIRE(CfgParser.getSection("sectiomA").findParam("Float"  ));

    CHECK(CfgParser.getSection("sectiomA").getParam("Integer").getArg(0, std::string("")) == "2048");
    CHECK(CfgParser.getSection("sectiomA").getParam("Float"  ).getArg(0, std::string("")) == "3.14");

    CHECK(CfgParser.getSection("sectiomA").getParam("Integer").getArg(0, 0   ) == 2048 );
    CHECK(CfgParser.getSection("sectiomA").getParam("Float"  ).getArg(0, 0.0f) == 3.14f);

    CHECK(CfgParser.getSection("sectiomA").getParam1stArg("Integer", 0 ) == 2048 );
    CHECK(CfgParser.getSection("sectiomA").getParam1stArg("Float", 0.0f) == 3.14f);

    CHECK(CfgParser.getParam1stArg("sectiomA", "Integer", 0   ) == 2048 );
    CHECK(CfgParser.getParam1stArg("sectiomA", "Float"  , 0.0f) == 3.14f);

    //sectionB
    REQUIRE(CfgParser.findSection("sectiomB"));

    REQUIRE(CfgParser.getSection("sectiomB").findParam("Negative"));

    CHECK(CfgParser.getSection("sectiomB").getParam("Negative").getArg(0, std::string("")) == "-1");

    CHECK(CfgParser.getSection("sectiomB").getParam("Negative").getArg(0, 0) == -1);

    CHECK(CfgParser.getSection("sectiomB").getParam1stArg("Negative", 0) == -1);

    CHECK(CfgParser.getParam1stArg("sectiomB", "Negative", 0) == -1);
  }

  SUBCASE("multi-args")
  {
    const std::string ConfigVector = R"RAWSTRING(
VectorIntSingleline = 0, 1, 2, 4, 5, 4, 3, 2, 1, 0
VectorIntMultiline  = 0, 1, 2,
                      4, 5, 4,
                      3, 2, 1,
                      0

)RAWSTRING";

    const std::vector<std::string> RefVectorStr{ "0", "1", "2", "4", "5", "4", "3", "2", "1", "0" };
    const std::vector<int32      > RefVectorInt{0, 1, 2, 4, 5, 4, 3, 2, 1, 0};

    xCfgINI::xParser CfgParser;
    REQUIRE(CfgParser.loadFromString(ConfigVector));

    REQUIRE(CfgParser.findParam("VectorIntSingleline"));
    CHECK(CfgParser.getParam    ("VectorIntSingleline").getArgs() == RefVectorStr);
    CHECK(CfgParser.getParamArgs("VectorIntSingleline")           == RefVectorStr);
    CHECK(CfgParser.getParam    ("VectorIntSingleline").getArgs(0) == RefVectorInt);
    //CHECK(CfgParser.getParamArgs("VectorIntSingleline", 0) == std::vector<int32>{0, 1, 2, 4, 5, 4, 3, 2, 1, 0});

    REQUIRE(CfgParser.findParam("VectorIntMultiline"));
    CHECK(CfgParser.getParam    ("VectorIntMultiline").getArgs() == RefVectorStr);
    CHECK(CfgParser.getParamArgs("VectorIntMultiline")           == RefVectorStr);
    CHECK(CfgParser.getParam    ("VectorIntMultiline").getArgs(0) == RefVectorInt);
  }

  SUBCASE("comments")
  {
    const std::string ConfigWithComments = R"RAWSTRING(
#something
Text      = "A.yuv" #some useless note
[sectiomA]          #section note
Integer   = 2048    #2nd useless note
Float     = 3.14    #another useless note
Negative  = -1

)RAWSTRING";

    xCfgINI::xParser CfgParser;
    REQUIRE(CfgParser.loadFromString(ConfigWithComments));

    CHECK(CfgParser.getRootSection().getComment() == "");
    REQUIRE(CfgParser.findParam("Text"));
    CHECK(CfgParser.getParam("Text").getComment() == "some useless note");

    REQUIRE(CfgParser.findSection("sectiomA"));
    CHECK  (CfgParser.getSection("sectiomA").getComment() == "section note");
    REQUIRE(CfgParser.getSection("sectiomA").findParam("Integer"));
    CHECK  (CfgParser.getSection("sectiomA").getParam("Integer").getComment() == "2nd useless note");

    REQUIRE(CfgParser.getSection("sectiomA").findParam("Float"));
    CHECK(CfgParser.getSection("sectiomA").getParam("Float").getComment() == "another useless note");

    REQUIRE(CfgParser.getSection("sectiomA").findParam("Negative"));
    CHECK(CfgParser.getSection("sectiomA").getParam("Negative").getComment() == "");
  }
}

//===============================================================================================================================================================================================================
