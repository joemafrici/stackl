#pragma once
//*******************************************************
// Purpose: Class for a prints statement
//
// Author: Philip Howard
// Email:  phil.howard@oit.edu
//
// Date: 2/20/2015
//
//*******************************************************

#include <string>

#include "cStmtNode.h"
#include "cExprNode.h"
#include "codegen.h"

class cPrintsNode : public cStmtNode
{
  public:
    cPrintsNode(char *str) : cStmtNode() , mStr(str)
    {
    }

    virtual std::string toString()
    {
        std::string result("PRINTS: <<NOT IMPLEMENTED>>");
        return result;
    }

    virtual int ComputeOffsets(int base)
    {
        return base;
    }

    virtual void GenerateCode()
    {
        //EmitInt(PRINT_OP);
    }
  protected:
    std::string mStr;       // string to be printed
};

