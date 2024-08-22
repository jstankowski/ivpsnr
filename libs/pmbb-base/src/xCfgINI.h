/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefPMBB-BASE.h"
#include "xString.h"
#include <vector>
#include <map>
#include <sstream>
#include <functional>

namespace PMBB_BASE {

//===============================================================================================================================================================================================================

class xCfgINI
{
  enum class eResult
  {
    INVALID = NOT_VALID,
    UNKNOWN = 0,
    Success,
    Error,
    Failure
  };

public:
  using stringVx = std::vector<std::string>;
  using flt32Vx  = std::vector<flt32      >;
  using flt64Vx  = std::vector<flt64      >;
  using int64Vx  = std::vector<int64      >;
  using int32Vx  = std::vector<int32      >;
  using boolVx   = std::vector<bool       >;

  using tSectionNames = std::vector<std::string>;

  using tCSR = const std::string&;
  using tCSV = const std::string_view;

public:
  class xCmdParam
  {

  public:
    enum class eType
    {
      None,
      Rglr, //regular
      Flag,
      List,
      Fake,
    };

  protected:
    eType       m_Type;
    std::string m_CmdPattern;
    std::string m_SectionName;
    std::string m_ParamName;
    std::string m_FlagValue;
    char        m_Separator;
    std::string m_Comment;

  public:
    xCmdParam() { m_Type = eType::None; };
    [[deprecated]] xCmdParam(tCSR CmdPattern, tCSR SectionName, tCSR ParamName                ) { m_Type = eType::Rglr; m_CmdPattern = CmdPattern; m_SectionName = SectionName; m_ParamName = ParamName; m_FlagValue = ""       ; m_Comment = ""; }
    [[deprecated]] xCmdParam(tCSR CmdPattern, tCSR SectionName, tCSR ParamName, tCSR FlagValue) { m_Type = eType::Flag; m_CmdPattern = CmdPattern; m_SectionName = SectionName; m_ParamName = ParamName; m_FlagValue = FlagValue; m_Comment = ""; }

    xCmdParam(eType Type, tCSR CmdPattern, tCSR SectionName, tCSR ParamName, tCSR FlagValue, char Separator)
    {
      m_Type = Type;  m_CmdPattern = CmdPattern; m_SectionName = SectionName; m_ParamName = ParamName; m_FlagValue = FlagValue; m_Separator = Separator, m_Comment = "";
    }

  public:
    inline eType getType        (                ) const { return m_Type;              }
    inline void  setCmdName     (tCSR CmdPattern )       { m_CmdPattern = CmdPattern;  }
    inline tCSR  getCmdName     (                ) const { return m_CmdPattern;        }
    inline void  setSectionName (tCSR SectionName)       { m_SectionName = SectionName;}
    inline tCSR  getSectionName (                ) const { return m_SectionName;       }
    inline void  setParamName   (tCSR ParamName  )       { m_ParamName = ParamName;    }
    inline tCSR  getParamName   (                ) const { return m_ParamName;         }
    inline bool  getIsFlag      (                ) const { return m_Type == eType::Flag; }
    inline void  setFlagValue   (tCSR FlagValue  )       { m_FlagValue = FlagValue;    }
    inline tCSR  getFlagValue   (                ) const { return m_FlagValue;         }
    inline void  setSeparator   (char Separator  )       { m_Separator = Separator;    }
    inline char  getSeparator   (                ) const { return m_Separator;         }

  };

  class xParam
  {
  protected:
    std::string m_Name;
    std::string m_Comment;
    stringVx    m_Args;

  public:
    xParam(tCSR Name) { m_Name = Name; }
    xParam(tCSV Name) { m_Name = Name; }

    tCSR getName   (            ) const { return m_Name; }
    void setComment(tCSR Comment)       { m_Comment = Comment; }
    void setComment(tCSV Comment)       { m_Comment = Comment; }
    tCSR getComment(            ) const { return m_Comment; }

    void     clearArgs ()       { m_Args.clear(); }
    uintSize getNumArgs() const { return m_Args.size(); }
    void     setArg    (const stringVx& Args) { m_Args = Args; }
    void     addArg    (tCSR             Arg) { m_Args.push_back(Arg); }
    void     addArg    (std::string&&    Arg) { m_Args.push_back(std::move(Arg)); }
    void     addArg    (std::string_view Arg) { m_Args.emplace_back(Arg); }

    tCSR  getArg(uint32 ArgIdx, tCSR  Default) const { return m_Args.size()>ArgIdx ? m_Args[ArgIdx] : Default; }
    flt64 getArg(uint32 ArgIdx, flt64 Default) const { return m_Args.size()>ArgIdx ? xString::StrToXXX(m_Args[ArgIdx], Default) : Default; }
    flt32 getArg(uint32 ArgIdx, flt32 Default) const { return m_Args.size()>ArgIdx ? xString::StrToXXX(m_Args[ArgIdx], Default) : Default; }
    int64 getArg(uint32 ArgIdx, int64 Default) const { return m_Args.size()>ArgIdx ? xString::StrToXXX(m_Args[ArgIdx], Default) : Default; }
    int32 getArg(uint32 ArgIdx, int32 Default) const { return m_Args.size()>ArgIdx ? xString::StrToXXX(m_Args[ArgIdx], Default) : Default; }
    bool  getArg(uint32 ArgIdx, bool  Default) const { return m_Args.size()>ArgIdx ? xString::StrToXXX(m_Args[ArgIdx], Default) : Default; }

    const stringVx&    getArgs(             ) const { return m_Args; }
    std::vector<flt32> getArgs(flt32 Default) const { return xString::VecOfStringToVecOfXXX(m_Args, Default); }
    std::vector<flt64> getArgs(flt64 Default) const { return xString::VecOfStringToVecOfXXX(m_Args, Default); }
    std::vector<int64> getArgs(int64 Default) const { return xString::VecOfStringToVecOfXXX(m_Args, Default); }
    std::vector<int32> getArgs(int32 Default) const { return xString::VecOfStringToVecOfXXX(m_Args, Default); }
    std::vector<bool > getArgs(bool  Default) const { return xString::VecOfStringToVecOfXXX(m_Args, Default); }
  };

  class xSection
  {
  public:
    using tParams = std::map<std::string, xParam>;

  protected:
    std::string m_Name;
    std::string m_Comment;
    tParams     m_Params;

  public:
    xSection(tCSR     Name) { m_Name = Name; }
    xSection(tCSV Name) { m_Name = Name; }
    void clear() { m_Comment.clear(); m_Params.clear(); }

    tCSR getName   (            ) const { return m_Name; }
    void setComment(tCSR Comment)       { m_Comment = Comment; }
    void setComment(tCSV Comment)       { m_Comment = Comment; }
    tCSR getComment(            ) const { return m_Comment; }

    //params
    void             clearParams (                   )       { m_Params.clear(); }
    uintSize         getNumParams(                   ) const { return m_Params.size(); }
    void             assignParam (const xParam& Param)       { m_Params.insert_or_assign(Param.getName(), Param); }
    void             assignParam (xParam&&      Param)       { m_Params.insert_or_assign(Param.getName(), std::move(Param)); }
    tParams&         getParams   (                   )       { return m_Params; }
    const tParams&   getParams   (                   ) const { return m_Params; }
    bool             findParam   (tCSR ParamName) const { return (m_Params.find(ParamName) != m_Params.end()); }
    xParam&          getParam    (tCSR ParamName)       { return m_Params.at(ParamName); }
    const xParam&    getParam    (tCSR ParamName) const { return m_Params.at(ParamName); }
    const stringVx   getParamList() const { stringVx PL; std::transform(m_Params.begin(), m_Params.end(), std::back_inserter(PL), [](auto const& Pair) { return Pair.first; }); return PL; }

    //params - direct access
    tCSR  getParam1stArg(tCSR ParamName, tCSR  Default) const { return findParam(ParamName) ? getParam(ParamName).getArg(0, Default) : Default; }
    flt64 getParam1stArg(tCSR ParamName, flt64 Default) const { return findParam(ParamName) ? getParam(ParamName).getArg(0, Default) : Default; }
    flt32 getParam1stArg(tCSR ParamName, flt32 Default) const { return findParam(ParamName) ? getParam(ParamName).getArg(0, Default) : Default; }
    int64 getParam1stArg(tCSR ParamName, int64 Default) const { return findParam(ParamName) ? getParam(ParamName).getArg(0, Default) : Default; }
    int32 getParam1stArg(tCSR ParamName, int32 Default) const { return findParam(ParamName) ? getParam(ParamName).getArg(0, Default) : Default; }
    bool  getParam1stArg(tCSR ParamName, bool  Default) const { return findParam(ParamName) ? getParam(ParamName).getArg(0, Default) : Default; }
    template<typename XXX> XXX cvtParam1stArg(tCSR ParamName, XXX Default, XXX(*Interpreter)(tCSR)) const
    {
      if(!findParam(ParamName) || getParam(ParamName).getNumArgs() < 1) { return Default; }
      return Interpreter(getParam(ParamName).getArgs()[0]);
    }
    //template<typename XXX> XXX cvtParam1stArg(tCSV ParamName, XXX Default, XXX(*Interpreter)(tCSR)) const
    //{
    //  if(!findParam(ParamName) || getParam(ParamName).getNumArgs() < 1) { return Default; }
    //  return Interpreter(getParam(ParamName).getArgs()[0]);
    //}

    stringVx getParamArgs(tCSR ParamName               ) const { return findParam(ParamName) ? getParam(ParamName).getArgs(       ) : stringVx(); }
    flt32Vx  getParamArgs(tCSR ParamName, flt32 Default) const { return findParam(ParamName) ? getParam(ParamName).getArgs(Default) : flt32Vx (); }
    flt64Vx  getParamArgs(tCSR ParamName, flt64 Default) const { return findParam(ParamName) ? getParam(ParamName).getArgs(Default) : flt64Vx (); }
    int64Vx  getParamArgs(tCSR ParamName, int64 Default) const { return findParam(ParamName) ? getParam(ParamName).getArgs(Default) : int64Vx (); }
    int32Vx  getParamArgs(tCSR ParamName, int32 Default) const { return findParam(ParamName) ? getParam(ParamName).getArgs(Default) : int32Vx (); }
    boolVx   getParamArgs(tCSR ParamName, bool  Default) const { return findParam(ParamName) ? getParam(ParamName).getArgs(Default) : boolVx  (); }
    
    template <typename XXX> std::vector<XXX> getParamArgs(tCSR ParamName, XXX Default) const { return findParam(ParamName) ? getParam(ParamName).getArgs(Default) : std::vector<XXX>()    ; }
  };

  class xRootSection final : public xSection
  {
  public:
    using tSections = std::map<std::string, xSection>;    

  protected:
    tSections m_SubSections;

  public:
    xRootSection() : xSection(std::string_view("root")) {}
    void clear() { m_Comment.clear(); m_Params.clear(); m_SubSections.clear();  }

    //subsections
    void             clearSections (                )       { m_SubSections.clear(); }
    uintSize         getNumSections(                ) const { return m_SubSections.size(); }
    void             addSection    (tCSR SectionName)       { if(!findSection(SectionName)) { m_SubSections.emplace(std::make_pair(SectionName, SectionName)); } }
    tSections&       getSections   (                )       { return m_SubSections; }
    const tSections& getSections   (                ) const { return m_SubSections; }
    bool             findSection   (tCSR SectionName) const { return (m_SubSections.find(SectionName) != m_SubSections.end()); }
    xSection&        getSection    (tCSR SectionName)       { return m_SubSections.at(SectionName); }
    const xSection&  getSection    (tCSR SectionName) const { return m_SubSections.at(SectionName); }
    xSection*        getSectionPtr (tCSR SectionName)       { return &m_SubSections.at(SectionName); }
    const xSection*  getSectionPtr (tCSR SectionName) const { return &m_SubSections.at(SectionName); }
    
    tSectionNames    getSectionsNames() const
    {
      std::vector<std::string> SectionsNames;
      std::transform(m_SubSections.begin(), m_SubSections.end(), std::inserter(SectionsNames, SectionsNames.end()), [](auto pair) { return pair.first; });
      return SectionsNames;
    }
  };

  class xParser
  {
  protected:
    static constexpr bool     c_AllowMultiline        = true;
    static constexpr bool     c_AllowEmptyArgs        = true;
    static constexpr bool     c_AllowUTF8_BOM         = true;
    static constexpr uintSize c_ExpectedMaxLineLength = 1024;
    static constexpr uintSize c_ExpectedMaxNameLength = 256;

  protected:
    static constexpr char c_CmdToken    = '-';
    static constexpr char c_SectionBeg  = '[';
    static constexpr char c_SectionEnd  = ']';
    static constexpr char c_QuoteMarkA  = '"';
    static constexpr char c_QuoteMarkB  = '\'';
    static constexpr char c_AssignmentA = '=';
    static constexpr char c_AssignmentB = ':';
    static constexpr char c_Separator   = ',';
    static constexpr char c_CommentA    = '#';
    static constexpr char c_CommentB    = ';';

    static constexpr std::string_view c_CmdPrefixShrt = "-" ;
    static constexpr std::string_view c_CmdPrefixLong = "--";

    static inline bool xIsSectionBeg(int32 c) { return (c==c_SectionBeg); }
    static inline bool xIsSectionEnd(int32 c) { return (c==c_SectionEnd); }
    static inline bool xIsSection   (int32 c) { return (xIsSectionBeg(c)  || xIsSectionEnd(c)); }

    static inline bool xIsQuoteMark (int32 c) { return (c==c_QuoteMarkA   || c==c_QuoteMarkB ); }
    static inline bool xIsAssignment(int32 c) { return (c==c_AssignmentA  || c==c_AssignmentB); }
    static inline bool xIsSeparator (int32 c) { return (c==c_Separator); }

    static inline bool xIsComment   (int32 c) { return (c==c_CommentA || c==c_CommentB); }

    static inline bool xIsNameBeg   (int32 c) { return (!xString::IsBlank(c) && !xString::IsEndl(c) && !xIsAssignment(c) && !xIsSection(c) && !xIsSeparator(c) && !xIsComment(c)); }
    static inline bool xIsName      (int32 c) { return (                        !xString::IsEndl(c) && !xIsAssignment(c) && !xIsSection(c) && !xIsSeparator(c) && !xIsComment(c)); }

    static inline uintSize xCountBlank(std::string_view S) { for(uintSize C = 0; C < S.length(); C++) { if(!xString::IsBlank(S[C])) { return C; } } return std::string_view::npos; }

  protected:
    bool               m_AllowUnknownCmdParams = false;
    bool               m_AllowEmptyCmdParams   = false;
    xRootSection       m_RootSection;
    bool               m_ConfigFileFoundInCmd;
    std::stringstream  m_ConfigStream;
    std::string        m_CurrLine;
    uint32             m_CurrLineNum;
    std::string        m_LastSectionName;
    std::string        m_LastParamName;
    int32              m_LastParamIndent;
    std::string        m_ParserLog;

    std::map<std::string, xCmdParam> m_CmdParams;

  public:
    xParser();
    void        setUnknownCmdParams(bool AllowUnknownCmdParams) { m_AllowUnknownCmdParams = AllowUnknownCmdParams; }
    void        setEmptyCmdParams  (bool AllowEmptyCmdParams  ) { m_AllowEmptyCmdParams   = AllowEmptyCmdParams  ; }
    void        addCmdParm(tCSR CmdShrt, tCSR CmdLong, tCSR SectionName, tCSR ParamName                );
    void        addCmdFlag(tCSR CmdShrt, tCSR CmdLong, tCSR SectionName, tCSR ParamName, tCSR FlagValue);
    void        addCmdList(tCSR CmdShrt, tCSR CmdLong, tCSR SectionName, tCSR ParamName, char Separator);
    void        addCmdFake(tCSR CmdShrt, tCSR CmdLong                                                  );
    bool        loadFromCmdln (int argc, const char* argv[], tCSR CfgTokenShort = std::string(c_CmdPrefixShrt) + "c", tCSR = std::string(c_CmdPrefixLong) + "config");
    bool        loadFromFile  (tCSR FileName);
    bool        loadFromString(tCSR Buffer  );
    std::string printConfig         (             );
    void        clear               (             ) { m_RootSection.clear(); m_CmdParams.clear(); }
    bool        getCfgFileFoundInCmd(             ) { return m_ConfigFileFoundInCmd; }
    std::string getParsingLog       (             ) { return std::string("PARSING LOG\n") + m_ParserLog; }

  protected:
    void        xAddCmdParam     (xCmdParam CmdParam) { m_CmdParams.insert_or_assign(CmdParam.getCmdName(), CmdParam); }
    eResult     xParseSection    (                               std::string_view  LineView);
    eResult     xParseSectionName(std::string_view& SectionName, std::string_view& LineView);
    eResult     xParseParam      (                               std::string_view  LineView);
    eResult     xParseParamCont  (                               std::string_view  LineView);
    eResult     xParseParamName  (std::string_view& ParamName,   std::string_view& LineView);
    eResult     xParseParamArg   (std::string_view& ParamArg ,   std::string_view& LineView);

    std::string xStoreSection    (const xSection& Section);
    std::string xStoreParam      (const xParam&   Param  );

  public:
    xRootSection&       getRootSection()       { return m_RootSection; }
    const xRootSection& getRootSection() const { return m_RootSection; }
 
    bool            findSection   (tCSR SectionName) const { return m_RootSection.findSection(SectionName); }
    xSection&       getSection    (tCSR SectionName)       { return m_RootSection.getSection (SectionName); }
    const xSection& getSection    (tCSR SectionName) const { return m_RootSection.getSection (SectionName); }
    tSectionNames   getSectionsNames() { return m_RootSection.getSectionsNames(); }

    bool        findParam     (tCSR ParamName) { return m_RootSection.findParam(ParamName); } //operates on root section
    xParam      getParam      (tCSR ParamName) { return m_RootSection.getParam (ParamName); } //operates on root section

    tCSR  getParam1stArg(tCSR ParamName, tCSR  Default) const { return m_RootSection.getParam1stArg(ParamName, Default); } //operates on root section
    flt64 getParam1stArg(tCSR ParamName, flt64 Default) const { return m_RootSection.getParam1stArg(ParamName, Default); } //operates on root section
    flt32 getParam1stArg(tCSR ParamName, flt32 Default) const { return m_RootSection.getParam1stArg(ParamName, Default); } //operates on root section
    int64 getParam1stArg(tCSR ParamName, int64 Default) const { return m_RootSection.getParam1stArg(ParamName, Default); } //operates on root section
    int32 getParam1stArg(tCSR ParamName, int32 Default) const { return m_RootSection.getParam1stArg(ParamName, Default); } //operates on root section
  //bool  getParam1stArg(tCSR ParamName, bool  Default) const { return m_RootSection.getParam1stArg(ParamName, Default); } //operates on root section
    //template<typename XXX> XXX cvtParam1stArg(const char ParamName[], XXX Default, std::function<XXX(tCSR)> Interpreter) const { return m_RootSection.cvtParam1stArg(ParamName, Default, Interpreter); } //operates on root section
    template<typename XXX> XXX cvtParam1stArg(tCSR ParamName, XXX Default, XXX(*Interpreter)(tCSR)) const { return m_RootSection.cvtParam1stArg(ParamName, Default, Interpreter); } //operates on root section
    //template<typename XXX> XXX cvtParam1stArg(tCSV ParamName, XXX Default, std::function<XXX(tCSV)> Interpreter) const { return m_RootSection.cvtParam1stArg(ParamName, Default, Interpreter); } //operates on root section
  
    tCSR  getParam1stArg(tCSR SectionName, tCSR ParamName, tCSR  Default) const { return findSection(SectionName) ? getSection(SectionName).getParam1stArg(ParamName, Default) : Default; } //operates on root section
    flt64 getParam1stArg(tCSR SectionName, tCSR ParamName, flt64 Default) const { return findSection(SectionName) ? getSection(SectionName).getParam1stArg(ParamName, Default) : Default; } //operates on root section
    flt32 getParam1stArg(tCSR SectionName, tCSR ParamName, flt32 Default) const { return findSection(SectionName) ? getSection(SectionName).getParam1stArg(ParamName, Default) : Default; } //operates on root section
    int64 getParam1stArg(tCSR SectionName, tCSR ParamName, int64 Default) const { return findSection(SectionName) ? getSection(SectionName).getParam1stArg(ParamName, Default) : Default; } //operates on root section
    int32 getParam1stArg(tCSR SectionName, tCSR ParamName, int32 Default) const { return findSection(SectionName) ? getSection(SectionName).getParam1stArg(ParamName, Default) : Default; } //operates on root section
  //bool  getParam1stArg(tCSR SectionName, tCSR ParamName, bool  Default) const { return findSection(SectionName) ? getSection(SectionName).getParam1stArg(ParamName, Default) : Default; } //operates on root section

    stringVx getParamArgs(tCSR ParamName) const { return m_RootSection.getParamArgs(ParamName); } //operates on root section
  };

public:
  static void printCommandlineArgs(int argc, const char* argv[]);
  static void printParsingError(tCSR ErrorMessage, tCSR HelpString);
  static void printError(tCSR ErrorMessage, tCSR  HelpString = std::string());
  static void printError(tCSR ErrorMessage, tCSV& HelpString);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB

